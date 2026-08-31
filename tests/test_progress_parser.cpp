#include "../src/core/progress_parser.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("parseOperationProgress recognizes status keywords", "[progress_parser]")
{
    REQUIRE(parseOperationProgress("downloading foo-1.0-1-x86_64.pkg.tar.zst").statusText == "Downloading packages...");
    REQUIRE(parseOperationProgress("installing foo").statusText == "Installing packages...");
    REQUIRE(parseOperationProgress("Building foo (1/1)").statusText == "Building packages...");
    REQUIRE(parseOperationProgress("checking dependencies...").statusText == "Checking dependencies...");
    REQUIRE(parseOperationProgress("resolving dependencies...").statusText == "Resolving dependencies...");
    REQUIRE_FALSE(parseOperationProgress("some unrelated line").statusText.has_value());
}

TEST_CASE("parseOperationProgress extracts (n/total) package progress", "[progress_parser]")
{
    const auto result = parseOperationProgress("(2/5) checking package integrity");

    REQUIRE(result.currentPackage == 2);
    REQUIRE(result.totalPackages == 5);
    REQUIRE(result.progressPercent == 40);
}

TEST_CASE("parseOperationProgress tolerates a space before the numerator", "[progress_parser]")
{
    const auto result = parseOperationProgress("( 1/5) installing foo");

    REQUIRE(result.currentPackage == 1);
    REQUIRE(result.totalPackages == 5);
    REQUIRE(result.progressPercent == 20);
}

TEST_CASE("parseOperationProgress prefers a trailing NN% over (n/total)", "[progress_parser]")
{
    const auto result = parseOperationProgress("(1/5) downloading foo  75%");

    REQUIRE(result.currentPackage == 1);
    REQUIRE(result.totalPackages == 5);
    REQUIRE(result.progressPercent == 75);
}

TEST_CASE("parseOperationProgress returns no progress fields for plain output", "[progress_parser]")
{
    const auto result = parseOperationProgress("nothing interesting here");

    REQUIRE_FALSE(result.currentPackage.has_value());
    REQUIRE_FALSE(result.totalPackages.has_value());
    REQUIRE_FALSE(result.progressPercent.has_value());
}
