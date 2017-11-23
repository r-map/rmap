// Copyright Benoit Blanchon 2014-2017
// MIT License
//
// Arduino JSON library
// https://bblanchon.github.io/ArduinoJson/
// If you like this project, please add a star!

#include <ArduinoJson.h>
#include <catch.hpp>

TEST_CASE("DynamicJsonBuffer::strdup()") {
  DynamicJsonBuffer buffer;

  SECTION("Should return a copy") {
    char original[] = "hello";
    char* copy = buffer.strdup(original);
    strcpy(original, "world");
    REQUIRE(std::string("hello") == copy);
  }

  SECTION("Given NULL, return NULL") {
    const char* original = NULL;
    char* copy = buffer.strdup(original);
    REQUIRE(0 == copy);
  }
}
