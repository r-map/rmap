// Copyright Benoit Blanchon 2014-2017
// MIT License
//
// Arduino JSON library
// https://bblanchon.github.io/ArduinoJson/
// If you like this project, please add a star!

#include <ArduinoJson.h>
#include <catch.hpp>
#include <string>

TEST_CASE("JsonObject::remove()") {
  DynamicJsonBuffer jb;

  SECTION("SizeDecreased_WhenValuesAreRemoved") {
    JsonObject& obj = jb.createObject();
    obj["hello"] = 1;

    obj.remove("hello");

    REQUIRE(0 == obj.size());
  }

  SECTION("SizeUntouched_WhenRemoveIsCalledWithAWrongKey") {
    JsonObject& obj = jb.createObject();
    obj["hello"] = 1;

    obj.remove("world");

    REQUIRE(1 == obj.size());
  }

  SECTION("RemoveByIterator") {
    JsonObject& obj = jb.parseObject("{\"a\":0,\"b\":1,\"c\":2}");

    for (JsonObject::iterator it = obj.begin(); it != obj.end(); ++it) {
      if (it->value == 1) obj.remove(it);
    }

    std::string result;
    obj.printTo(result);
    REQUIRE("{\"a\":0,\"c\":2}" == result);
  }
}
