// SPDX-License-Identifier: GPL-3.0-or-later

#include <array>
#include <chrono>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "common/scope_exit.h"
#include "core/hle/service/nx2_gsv.h"
#include "tests/audio_core/sysmix_test_context.h"

namespace Service {

struct Nx2VideoSessionTestAccess {
    using Clock = std::chrono::steady_clock;

    static void Start(INx2VideoSession& session, Clock::time_point now) {
        session.m_clock_running = true;
        session.m_clock_start = now - std::chrono::milliseconds{500};
        session.m_clock_last_take = now;
        session.m_clock_start_pts = 0;
        session.m_free.push_back(7);
    }

    static void Queue(INx2VideoSession& session, std::span<Tegra::Host1x::Nx2Picture> pictures,
                      bool end = false) {
        session.QueueDecodedPictures(pictures, 99, end);
    }

    static auto& Decoded(INx2VideoSession& session) { return session.m_decoded; }
    static auto& Free(INx2VideoSession& session) { return session.m_free; }
    static u64 Dropped(INx2VideoSession& session) { return session.m_late_dropped; }
    static bool Late(INx2VideoSession& session, u64 pts) { return session.IsLate(pts); }
    static void Stop(INx2VideoSession& session) { session.EndMovieClock(); }

    static void Pause(INx2VideoSession& session, Clock::time_point now) {
        session.m_clock_last_take = now - std::chrono::seconds{2};
        session.m_clock_start = session.m_clock_last_take - std::chrono::milliseconds{500};
        const auto previous = session.m_clock_start;
        session.NoteOutputTaken(7, now);
        REQUIRE(session.m_clock_start == previous + std::chrono::seconds{2});
        REQUIRE(session.m_clock_last_take == now);
    }

    static void Released(INx2VideoSession& session, Clock::time_point now) {
        auto& counters = Common::Nx2MovieClock::Get();
        const auto releases = counters.video_releases.load();
        const auto duration = counters.take_to_release_us.load();
        SCOPE_EXIT {
            counters.video_releases = releases;
            counters.take_to_release_us = duration;
        };
        session.m_take_times[7] = now - std::chrono::milliseconds{100};
        session.m_take_times[8] = now - std::chrono::milliseconds{20};
        session.RecordOutputRelease(7, now);
        REQUIRE(counters.video_releases.load() == releases + 1);
        REQUIRE(counters.take_to_release_us.load() == duration + 100'000);
        REQUIRE(session.m_take_times.contains(8));
        session.RecordOutputRelease(7, now);
        REQUIRE(counters.video_releases.load() == releases + 1);
        session.RecordOutputRelease(8, now);
        REQUIRE(counters.video_releases.load() == releases + 2);
        REQUIRE(counters.take_to_release_us.load() == duration + 120'000);
    }
};

} // namespace Service

namespace {
using Access = Service::Nx2VideoSessionTestAccess;

struct Fixture {
    SysmixTests::Context context;
    Service::INx2VideoSession session{context.system, std::make_shared<Service::Nx2NativeObjects>(),
                                     "nx2:vdec-test"};
};
} // namespace

TEST_CASE("Movie catch-up retains an output when the entire decoded batch is late",
          "[nx2_movie_clock]") {
    Fixture f;
    Access::Start(f.session, Access::Clock::now());
    const size_t count = GENERATE(1U, 3U);
    std::array<Tegra::Host1x::Nx2Picture, 3> pictures{};
    for (size_t i = 0; i < count; ++i) {
        pictures[i].pts = i * 16'667;
    }
    Access::Queue(f.session, std::span{pictures}.first(count));
    REQUIRE(Access::Decoded(f.session).size() == 1);
    REQUIRE(Access::Decoded(f.session).front().pts == (count - 1) * 16'667);
    REQUIRE_FALSE(Access::Decoded(f.session).front().end);
    REQUIRE(Access::Dropped(f.session) == count - 1);
    REQUIRE_FALSE(Access::Free(f.session).empty());
}

TEST_CASE("Movie catch-up preserves upcoming frames and end-of-stream output",
          "[nx2_movie_clock]") {
    Fixture f;
    Access::Start(f.session, Access::Clock::now());
    SECTION("Upcoming pictures remain in order") {
        std::array<Tegra::Host1x::Nx2Picture, 2> pictures{};
        pictures[0].pts = 1'000'000;
        pictures[1].pts = 1'016'667;
        Access::Queue(f.session, pictures);
        REQUIRE(Access::Decoded(f.session).size() == 2);
        REQUIRE(Access::Decoded(f.session).front().pts == 1'000'000);
        REQUIRE(Access::Dropped(f.session) == 0);
    }
    SECTION("A late final picture carries EOS") {
        std::array<Tegra::Host1x::Nx2Picture, 2> pictures{};
        pictures[1].pts = 16'667;
        Access::Queue(f.session, pictures, true);
        REQUIRE(Access::Decoded(f.session).size() == 1);
        REQUIRE(Access::Decoded(f.session).back().end);
        REQUIRE(Access::Decoded(f.session).back().pts == 16'667);
    }
    SECTION("An empty EOS packet still completes an output") {
        Access::Queue(f.session, {}, true);
        REQUIRE(Access::Decoded(f.session).size() == 1);
        REQUIRE(Access::Decoded(f.session).back().end);
        REQUIRE_FALSE(Access::Decoded(f.session).back().picture);
    }
}

TEST_CASE("Movie catch-up pauses its clock and resets across streams", "[nx2_movie_clock]") {
    Fixture f;
    const auto now = Access::Clock::now();
    Access::Start(f.session, now);
    REQUIRE(Access::Late(f.session, 0));
    Access::Pause(f.session, now);
    REQUIRE_FALSE(Access::Late(f.session, 500'000));
    Access::Stop(f.session);
    REQUIRE_FALSE(Access::Late(f.session, 0));
    REQUIRE(Access::Dropped(f.session) == 0);
}

TEST_CASE("Movie release measurements follow image identity across interleaved takes",
          "[nx2_movie_clock]") {
    Fixture f;
    Access::Released(f.session, Access::Clock::now());
}
