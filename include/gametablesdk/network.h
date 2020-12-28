#pragma once

#include <tuple>
#include <string>
#include "rapidjson/document.h"
#include "pistache/client.h"
#include "playercontroller.h"
#include "event.h"
#include "piecetype.h"
#include "winds.h"

namespace GametableSDK {
  namespace Network {
    using Pistache::Http::Client;
    using Mahjong::PlayerController, Mahjong::Event, Mahjong::Wind, Mahjong::Piece;
    using rapidjson::Document;

    auto connectToMatch(PlayerController&, std::string) -> void;
    auto registerForMatch(Client&, std::string) -> std::tuple<bool, std::string>;
    auto blockUntilMatchReady(Client&, std::string) -> void;
    auto getMatchInfo(Client&, std::string baseAddress, std::string token) -> std::tuple<std::vector<Event>, bool, std::vector<Piece>, Wind, Wind>;
    auto sendDecision(Client&, std::string baseAddress, std::string token, Event) -> void;
  }
}
