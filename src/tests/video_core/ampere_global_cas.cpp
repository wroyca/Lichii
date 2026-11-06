// SPDX-License-Identifier: GPL-3.0-or-later

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <span>
#include <catch2/catch_test_macros.hpp>

#include "shader_recompiler/backend/spirv/emit_spirv.h"
#include "shader_recompiler/frontend/ampere/lower_to_maxwell.h"
#include "shader_recompiler/frontend/maxwell/decode.h"
#include "shader_recompiler/frontend/maxwell/translate/impl/impl.h"
#include "shader_recompiler/ir_opt/passes.h"

namespace {
namespace IR = Shader::IR;
using Shader::Ampere::Instruction;
using Shader::Ampere::LowerToMaxwell;

// seedds/nxnative#1462: FoliageShader.sharcb, 13002657609092b4 + 0x8c00.
constexpr Instruction Cas{{0x000073a9, 0x00000008, 0x001ee109, 0x000ea400}};
constexpr Instruction Exit{{0x0000794d, 0, 0x03800000, 0}};

void Set(Instruction& instruction, u32 bit, u32 width, u32 value) {
    const u32 mask = ((1U << width) - 1) << (bit % 32);
    auto& word = instruction.words[bit / 32];
    word = (word & ~mask) | ((value << (bit % 32)) & mask);
}

class Environment final : public Shader::Environment {
public:
    u64 ReadInstruction(u32) override { FAIL("Unexpected instruction fetch"); return 0; }
    u32 ReadCbufValue(u32, u32) override { FAIL("Unexpected constant fetch"); return 0; }
    Shader::TextureType ReadTextureType(u32) override { return Shader::TextureType::Color2D; }
    Shader::TexturePixelFormat ReadTexturePixelFormat(u32) override {
        return Shader::TexturePixelFormat::A8B8G8R8_UNORM;
    }
    bool IsTexturePixelFormatInteger(u32) override { return false; }
    u32 ReadViewportTransformState() override { return 1; }
    u32 TextureBoundBuffer() const override { return 0; }
    u32 LocalMemorySize() const override { return 0; }
    u32 SharedMemorySize() const override { return 0; }
    std::array<u32, 3> WorkgroupSize() const override { return {1, 1, 1}; }
    bool HasHLEMacroState() const override { return false; }
    std::optional<Shader::ReplaceConstant> GetReplaceConstBuffer(u32, u32) override {
        return std::nullopt;
    }
    void Dump(u64, u64) override {}
};

struct State {
    std::array<u32, 256> registers{};
    std::array<bool, 8> predicates{};
    u32 memory{};
    u32 exchanges{};
};

// Interpret the real Maxwell frontend's IR, rather than copying the Ampere lowering's encoding.
void Execute(State& state, std::span<const u64> code, s32 offset) {
    Environment env;
    for (const u64 word : code) {
        const auto opcode = Shader::Maxwell::Decode(word);
        if (opcode == Shader::Maxwell::Opcode::EXIT)
            continue;
        const u32 guard = (word >> 16) & 15;
        const bool enabled = (guard & 7) == 7 || state.predicates[guard & 7];
        if (enabled == bool(guard & 8))
            continue;
        Shader::ObjectPool<IR::Inst> pool;
        IR::Block block{pool};
        Shader::Maxwell::TranslatorVisitor visitor{env, block};
        switch (opcode) {
        case Shader::Maxwell::Opcode::MOV_reg: visitor.MOV_reg(word); break;
        case Shader::Maxwell::Opcode::ATOM_cas: visitor.ATOM_cas(word); break;
        case Shader::Maxwell::Opcode::ISETP_reg: visitor.ISETP_reg(word); break;
        default: FAIL("Unexpected lowered instruction");
        }
        std::map<const IR::Inst*, u64> values;
        const auto read = [&](IR::Value value) -> u64 {
            if (!value.IsImmediate())
                return values.at(value.Inst());
            if (value.Type() == IR::Type::U1)
                return value.U1();
            if (value.Type() == IR::Type::U64)
                return value.U64();
            return value.U32();
        };
        for (const auto& inst : block) {
            const auto arg = [&](size_t index) { return read(inst.Arg(index)); };
            u64 result{};
            switch (inst.GetOpcode()) {
            case IR::Opcode::GetRegister:
                result = state.registers[static_cast<u32>(inst.Arg(0).Reg())]; break;
            case IR::Opcode::SetRegister:
                state.registers[static_cast<u32>(inst.Arg(0).Reg())] = arg(1); break;
            case IR::Opcode::CompositeConstructU32x2:
                result = arg(0) | (arg(1) << 32); break;
            case IR::Opcode::PackUint2x32: result = arg(0); break;
            case IR::Opcode::IAdd64: result = arg(0) + arg(1); break;
            case IR::Opcode::GlobalAtomicCompareExchange32:
                REQUIRE(arg(0) == u64{0x1234567800001000} + offset);
                result = state.memory;
                if (state.memory == arg(1))
                    state.memory = arg(2);
                ++state.exchanges;
                break;
            case IR::Opcode::IEqual: result = arg(0) == arg(1); break;
            case IR::Opcode::LogicalAnd: result = arg(0) && arg(1); break;
            case IR::Opcode::LogicalNot: result = !arg(0); break;
            case IR::Opcode::SetPred:
                state.predicates[static_cast<u32>(inst.Arg(0).Pred())] = arg(1); break;
            default: FAIL("Unexpected IR: " << IR::NameOf(inst.GetOpcode()));
            }
            values.emplace(&inst, result);
        }
    }
}
} // namespace

TEST_CASE("Ampere foliage CAS returns the old word and handles aliased outputs and guards",
          "[video_core][ampere][global_cas]") {
    for (const u32 destination : {0U, 8U, 9U, 20U, 255U}) {
        for (const u32 guard : {0U, 7U, 8U}) {
            for (const bool initial_predicate : {false, true}) {
                for (const u32 old : {0U, 7U}) {
                    for (const s32 offset : {-0x80000, -4, 0, 4, 0x7ffff}) {
                        CAPTURE(destination, guard, initial_predicate, old, offset);
                        auto instruction = Cas;
                        Set(instruction, 16, 8, destination);
                        Set(instruction, 12, 4, guard);
                        Set(instruction, 40, 24, static_cast<u32>(offset));
                        Set(instruction, 81, 3, 0); // success writes P0, which may be the guard
                        const auto lowered = LowerToMaxwell(std::array{instruction, Exit});
                        INFO(lowered.error);
                        REQUIRE(lowered);
                        State state;
                        state.registers[0] = 0x1000;
                        state.registers[1] = 0x12345678;
                        state.registers[8] = 0;
                        state.registers[9] = 42;
                        state.predicates[0] = initial_predicate;
                        state.memory = old;
                        const auto original = state.registers;
                        Execute(state, lowered.instructions, offset);
                        const bool enabled = guard == 7 || (guard == 0 ? initial_predicate
                                                                                     : !initial_predicate);
                        CHECK(state.exchanges == u32{enabled});
                        CHECK(state.memory == (enabled && old == 0 ? 42 : old));
                        CHECK(state.predicates[0] == (enabled ? old == 0 : initial_predicate));
                        for (u32 reg = 0; reg < 246; ++reg)
                            CHECK(state.registers[reg] ==
                                  (enabled && reg == destination ? old : original[reg]));
                    }
                }
            }
        }
    }
}

TEST_CASE("Ampere foliage CAS rejects unsupported sizes and addressing modes",
          "[video_core][ampere][global_cas]") {
    for (const auto fields : {std::array{73U, 3U, 2U}, std::array{73U, 3U, 3U},
                              std::array{72U, 1U, 0U}, std::array{91U, 1U, 1U},
                              std::array{24U, 8U, 1U}, std::array{40U, 24U, 0x80000U}}) {
        auto instruction = Cas;
        Set(instruction, fields[0], fields[1], fields[2]);
        CHECK_FALSE(LowerToMaxwell(std::array{instruction, Exit}));
    }
    auto signed_cas = Cas;
    Set(signed_cas, 73, 3, 1);
    CHECK(LowerToMaxwell(std::array{signed_cas, Exit}));
}

TEST_CASE("Ampere foliage CAS shadows all high guest register operands",
          "[video_core][ampere][global_cas]") {
    auto instruction = Cas;
    Set(instruction, 16, 8, 240);
    Set(instruction, 24, 8, 242);
    Set(instruction, 32, 8, 244);
    Set(instruction, 64, 8, 245);
    const auto pressure = Shader::Ampere::PrepareRegisterPressure(instruction, 216);
    REQUIRE(pressure.valid);
    for (const u32 bit : {16U, 24U, 32U, 64U})
        CHECK(pressure.instruction.Bits(bit, 8) < 216);
    const Instruction uniform_marker{{0x0000fc02, 0, 0, 0}};
    CHECK(LowerToMaxwell(std::array{uniform_marker, instruction, Exit}));
}

TEST_CASE("An unused CAS result retains the atomic through optimization and SPIRV",
          "[video_core][ampere][global_cas]") {
    Environment env;
    Shader::ObjectPool<IR::Inst> pool;
    IR::Block block{pool};
    Shader::Maxwell::TranslatorVisitor visitor{env, block};
    const auto lowered = LowerToMaxwell(std::array{Cas, Exit});
    REQUIRE(lowered);
    for (const auto word : lowered.instructions) {
        if (Shader::Maxwell::Decode(word) == Shader::Maxwell::Opcode::ATOM_cas)
            visitor.ATOM_cas(word);
    }
    IR::Program program;
    program.stage = Shader::Stage::Compute;
    program.workgroup_size = {1, 1, 1};
    program.blocks = {&block};
    program.post_order_blocks = {&block};
    program.syntax_list = {{{.block = &block}, IR::AbstractSyntaxNode::Type::Block},
                           {{}, IR::AbstractSyntaxNode::Type::Return}};
    for (auto& inst : block) {
        if (inst.GetOpcode() == IR::Opcode::SetRegister)
            inst.Invalidate(); // discard Rd, as when the guest never reads the old word
        if (inst.GetOpcode() == IR::Opcode::GlobalAtomicCompareExchange32) {
            inst.ReplaceOpcode(IR::Opcode::StorageAtomicCompareExchange32);
            inst.SetArg(0, IR::Value{0U});
            inst.SetArg(1, IR::Value{0U});
            inst.SetArg(2, IR::Value{0U});
            inst.SetArg(3, IR::Value{42U});
        }
    }
    Shader::Optimization::DeadCodeEliminationPass(program);
    program.info.storage_buffers_descriptors = {{0, 0, 1, true}};
    program.info.used_storage_buffer_types = IR::Type::U32;
    Shader::Profile profile;
    profile.supported_spirv = 0x00010300;
    const auto words = Shader::Backend::SPIRV::EmitSPIRV(profile, program);
    u32 exchanges{};
    for (size_t i = 5; i < words.size(); i += words[i] >> 16) {
        REQUIRE((words[i] >> 16) != 0);
        exchanges += (words[i] & 0xffffU) == 230; // OpAtomicCompareExchange
    }
    CHECK(exchanges == 1);
    if (const char* directory = std::getenv("EDEN_CAS_TEST_OUTPUT")) {
        std::filesystem::create_directories(directory);
        std::ofstream file{std::filesystem::path{directory} / "foliage-cas.spv", std::ios::binary};
        file.write(reinterpret_cast<const char*>(words.data()), words.size() * sizeof(u32));
        REQUIRE(file.good());
    }
}
