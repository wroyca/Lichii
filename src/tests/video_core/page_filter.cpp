// SPDX-License-Identifier: GPL-3.0-or-later

#include <catch2/catch_test_macros.hpp>

#include "video_core/page_filter.h"

TEST_CASE("Cache page filters conservatively track partial pages and word boundaries",
          "[video_core][page_filter]") {
    VideoCommon::PageFilter filter;
    constexpr u64 page = 4096;
    REQUIRE_FALSE(filter.Any(0, page * 128));
    filter.Mark(page * 64 - 1, 2);
    REQUIRE(filter.Any(page * 63, 1));
    REQUIRE(filter.Any(page * 65 - 1, 1));
    REQUIRE_FALSE(filter.Any(page * 62, page));
    REQUIRE_FALSE(filter.Any(page * 65, page));

    filter.Mark(page * 128, page * 128);
    REQUIRE(filter.Any(page * 128, 1));
    REQUIRE(filter.Any(page * 255, page));
    REQUIRE_FALSE(filter.Any(page * 127, page));
    REQUIRE_FALSE(filter.Any(page * 256, page));
    REQUIRE(filter.Any(page * 63, 1));
}

TEST_CASE("Cache page filters retain empty and out-of-range fallbacks",
          "[video_core][page_filter]") {
    VideoCommon::PageFilter filter;
    constexpr u64 limit = u64{1} << 36;
    filter.Mark(0, 0);
    REQUIRE_FALSE(filter.Any(0, 1));
    REQUIRE(filter.Any(limit, 1));
    REQUIRE_FALSE(filter.Any(0, 1));
    filter.Mark(limit - 1, 1);
    REQUIRE(filter.Any(limit - 1, 1));
    REQUIRE_FALSE(filter.Any(0, 1));
    filter.Mark(limit - 1, 2);
    REQUIRE(filter.Any(0, 1));
    REQUIRE_FALSE(filter.Any(limit, 0));
}
