// Copyright Benoit Blanchon 2014-2017
// MIT License
//
// Arduino JSON library
// https://bblanchon.github.io/ArduinoJson/
// If you like this project, please add a star!

#include <ArduinoJson.h>
#include <catch.hpp>

TEST_CASE("JsonArray::copyFrom()") {
  SECTION("OneDimension") {
    DynamicJsonBuffer jsonBuffer;
    JsonArray& array = jsonBuffer.createArray();
    char json[32];
    int source[] = {1, 2, 3};

    bool ok = array.copyFrom(source);
    REQUIRE(ok);

    array.printTo(json, sizeof(json));
    REQUIRE(std::string("[1,2,3]") == json);
  }

  SECTION("OneDimension_JsonBufferTooSmall") {
    const size_t SIZE = JSON_ARRAY_SIZE(2);
    StaticJsonBuffer<SIZE> jsonBuffer;
    JsonArray& array = jsonBuffer.createArray();
    char json[32];
    int source[] = {1, 2, 3};

    bool ok = array.copyFrom(source);
    REQUIRE_FALSE(ok);

    array.printTo(json, sizeof(json));
    REQUIRE(std::string("[1,2]") == json);
  }

  SECTION("TwoDimensions") {
    DynamicJsonBuffer jsonBuffer;
    JsonArray& array = jsonBuffer.createArray();
    char json[32];
    int source[][3] = {{1, 2, 3}, {4, 5, 6}};

    bool ok = array.copyFrom(source);
    REQUIRE(ok);

    array.printTo(json, sizeof(json));
    REQUIRE(std::string("[[1,2,3],[4,5,6]]") == json);
  }

  SECTION("TwoDimensions_JsonBufferTooSmall") {
    const size_t SIZE =
        JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(3) + JSON_ARRAY_SIZE(2);
    StaticJsonBuffer<SIZE> jsonBuffer;
    JsonArray& array = jsonBuffer.createArray();
    char json[32];
    int source[][3] = {{1, 2, 3}, {4, 5, 6}};

    bool ok = array.copyFrom(source);
    REQUIRE_FALSE(ok);

    array.printTo(json, sizeof(json));
    REQUIRE(std::string("[[1,2,3],[4,5]]") == json);
  }
}
