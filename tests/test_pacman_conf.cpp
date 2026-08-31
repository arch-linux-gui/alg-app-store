#include "../src/core/pacman_conf.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("PacmanConf::isMultilibEnabled detects an active Include line", "[pacman_conf]")
{
    const QString contents = R"(
[options]
Architecture = auto

[multilib]
Include = /etc/pacman.d/mirrorlist

[extra]
Include = /etc/pacman.d/mirrorlist
)";

    REQUIRE(PacmanConf::isMultilibEnabled(contents));
}

TEST_CASE("PacmanConf::isMultilibEnabled ignores a commented-out section", "[pacman_conf]")
{
    const QString contents = R"(
[options]
Architecture = auto

#[multilib]
#Include = /etc/pacman.d/mirrorlist
)";

    REQUIRE_FALSE(PacmanConf::isMultilibEnabled(contents));
}

TEST_CASE("PacmanConf::isMultilibEnabled is false when section header present but body commented", "[pacman_conf]")
{
    const QString contents = R"(
[multilib]
#Include = /etc/pacman.d/mirrorlist

[extra]
Include = /etc/pacman.d/mirrorlist
)";

    REQUIRE_FALSE(PacmanConf::isMultilibEnabled(contents));
}

TEST_CASE("PacmanConf::isMultilibEnabled is false when section is absent", "[pacman_conf]")
{
    const QString contents = R"(
[options]
Architecture = auto

[extra]
Include = /etc/pacman.d/mirrorlist
)";

    REQUIRE_FALSE(PacmanConf::isMultilibEnabled(contents));
}

TEST_CASE("PacmanConf::isChaoticAurEnabled detects Server directive", "[pacman_conf]")
{
    const QString contents = R"(
[options]
Architecture = auto

[chaotic-aur]
Include = /etc/pacman.d/chaotic-mirrorlist
)";

    REQUIRE(PacmanConf::isChaoticAurEnabled(contents));
}

TEST_CASE("PacmanConf::isChaoticAurEnabled detects direct Server line", "[pacman_conf]")
{
    const QString contents = R"(
[chaotic-aur]
Server = https://geo-mirror.chaotic.cx/$repo/$arch
)";

    REQUIRE(PacmanConf::isChaoticAurEnabled(contents));
}

TEST_CASE("PacmanConf::isChaoticAurEnabled is false when section is absent", "[pacman_conf]")
{
    const QString contents = R"(
[options]
Architecture = auto
)";

    REQUIRE_FALSE(PacmanConf::isChaoticAurEnabled(contents));
}
