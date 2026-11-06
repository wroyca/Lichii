#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Execute the bundled AArch64 Dynamic FPS routine with synthetic counter values.

Requires unicorn. Run: uv run --no-project --with unicorn python tools/nx2/test_dynamic_fps_routine.py
"""

from pathlib import Path
import re
import struct
import unittest

from unicorn import Uc, UC_ARCH_ARM64, UC_MODE_ARM, UC_HOOK_CODE
from unicorn.arm64_const import (
    UC_ARM64_REG_CPACR_EL1, UC_ARM64_REG_PC, UC_ARM64_REG_S0, UC_ARM64_REG_S1,
    UC_ARM64_REG_S8, UC_ARM64_REG_X11, UC_ARM64_REG_X13, UC_ARM64_REG_X30,
)

SOURCE = Path(__file__).resolve().parents[2] / "src/core/file_sys/patch_manager.cpp"
ROUTINE = 0x3C46200
STATE = 0x4E9FE00
FREQUENCY = 720_000_000


def routine():
    source = SOURCE.read_text().split("DYNAMIC_FPS_ROUTINE{", 1)[1].split("};", 1)[0]
    code = bytes(int(value, 16) for value in re.findall(r"0x([0-9a-fA-F]{2})\b", source))
    assert len(code) == 120
    return code


def execute(code, fps, cold=False):
    cpu = Uc(UC_ARCH_ARM64, UC_MODE_ARM)
    cpu.mem_map(ROUTINE & ~0xFFF, 0x1000)
    cpu.mem_map(STATE & ~0xFFF, 0x1000)
    cpu.mem_write(ROUTINE, bytes(code))
    previous = FREQUENCY if not cold else 0
    cpu.mem_write(STATE, struct.pack("<QQ", previous, 0))
    cpu.reg_write(UC_ARM64_REG_CPACR_EL1, 3 << 20)
    sixty = struct.unpack("<I", struct.pack("<f", 60.0))[0]
    cpu.reg_write(UC_ARM64_REG_S0, sixty)
    cpu.reg_write(UC_ARM64_REG_S1, sixty)
    cpu.reg_write(UC_ARM64_REG_X30, 0x1000)

    def counters(machine, address, size, user):
        if address == ROUTINE + 12:
            machine.reg_write(UC_ARM64_REG_X11, FREQUENCY + FREQUENCY // fps)
            machine.reg_write(UC_ARM64_REG_PC, address + size)
        elif address == ROUTINE + 32:
            machine.reg_write(UC_ARM64_REG_X13, FREQUENCY)
            machine.reg_write(UC_ARM64_REG_PC, address + size)

    cpu.hook_add(UC_HOOK_CODE, counters)
    cpu.emu_start(ROUTINE, 0x1000, count=100)
    assert cpu.reg_read(UC_ARM64_REG_PC) == 0x1000
    return struct.unpack("<f", struct.pack("<I", cpu.reg_read(UC_ARM64_REG_S8)))[0]


class DynamicFpsRoutineTests(unittest.TestCase):
    def test_real_time_scales_between_ten_and_sixty_fps(self):
        for fps in (60, 40, 30, 20, 16, 15, 12, 10):
            with self.subTest(fps=fps):
                self.assertAlmostEqual(execute(routine(), fps), 60 / fps, places=5)

    def test_long_stalls_stop_at_one_hundred_milliseconds(self):
        for fps in (9, 5, 1):
            with self.subTest(fps=fps):
                self.assertAlmostEqual(execute(routine(), fps), 6.0, places=5)

    def test_old_and_new_routines_match_above_twenty_fps(self):
        old = bytearray(routine())
        old[36:40] = bytes.fromhex("8e0280d2")
        for fps in (240, 120, 60, 40, 30, 20):
            with self.subTest(fps=fps):
                self.assertEqual(execute(old, fps), execute(routine(), fps))
        self.assertAlmostEqual(execute(old, 10), 3.0, places=5)
        self.assertAlmostEqual(execute(routine(), 10), 6.0, places=5)

    def test_first_frame_retains_the_original_game_timestep(self):
        self.assertEqual(execute(routine(), 10, cold=True), 1.0)


if __name__ == "__main__":
    unittest.main()
