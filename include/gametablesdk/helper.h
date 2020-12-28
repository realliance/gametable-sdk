#pragma once

#include <vector>

#include "piecetype.h"
#include "event.h"
#include "rapidjson/document.h"

namespace GametableSDK {
  namespace Helper {
    using rapidjson::Value;
    using Mahjong::Event, Mahjong::Piece;

    auto parseHand(Value&) -> std::vector<Piece>;
    auto eventToJson(Event) -> std::string;
  }
}
