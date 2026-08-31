#include "../src/core/aur_helper.h"

#include <QJsonArray>
#include <QJsonObject>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("AurHelper::parseAurPackage maps basic fields", "[aur_helper]")
{
    QJsonObject obj;
    obj["Name"] = "yay";
    obj["Version"] = "12.3.5-1";
    obj["Description"] = "Yet another yogurt - an AUR helper";
    obj["Maintainer"] = "someone";
    obj["URL"] = "https://github.com/Jguer/yay";
    obj["LastModified"] = 1700000000;

    const PackageInfo info = AurHelper::parseAurPackage(obj);

    REQUIRE(info.name == "yay");
    REQUIRE(info.version == "12.3.5-1");
    REQUIRE(info.description == "Yet another yogurt - an AUR helper");
    REQUIRE(info.repository == "AUR");
    REQUIRE(info.maintainer == "someone");
    REQUIRE(info.upstreamUrl == "https://github.com/Jguer/yay");
    REQUIRE(info.lastUpdated.toSecsSinceEpoch() == 1700000000);
}

TEST_CASE("AurHelper::parseAurPackage combines Depends and MakeDepends", "[aur_helper]")
{
    QJsonObject obj;
    obj["Name"] = "example";
    obj["Version"] = "1.0-1";

    QJsonArray depends;
    depends.append("glibc");
    depends.append("openssl");
    obj["Depends"] = depends;

    QJsonArray makeDepends;
    makeDepends.append("cmake");
    obj["MakeDepends"] = makeDepends;

    const PackageInfo info = AurHelper::parseAurPackage(obj);

    REQUIRE(info.dependList.size() == 3);
    REQUIRE(info.dependList[0] == "glibc");
    REQUIRE(info.dependList[1] == "openssl");
    REQUIRE(info.dependList[2] == "cmake (make)");
}

TEST_CASE("AurHelper::parseAurPackage handles missing optional fields", "[aur_helper]")
{
    QJsonObject obj;
    obj["Name"] = "minimal";
    obj["Version"] = "1.0-1";

    const PackageInfo info = AurHelper::parseAurPackage(obj);

    REQUIRE(info.name == "minimal");
    REQUIRE(info.maintainer.isEmpty());
    REQUIRE(info.upstreamUrl.isEmpty());
    REQUIRE(info.dependList.isEmpty());
}
