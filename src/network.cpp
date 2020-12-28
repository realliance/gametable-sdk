#include "gametablesdk/network.h"
#include "gametablesdk/helper.h"

#include <string>
#include <chrono>
#include <thread>

#include "winds.h"
#include "piecetype.h"
#include "event.h"
#include "gentlemanbot.h"
#include "spdlog/spdlog.h"
#include "rapidjson/document.h"
#include "pistache/client.h"
#include "libmahjonghttp/jsonhelper.h"

using namespace GametableSDK;

using rapidjson::Document, rapidjson::SizeType;
using Pistache::Http::Client,
      Pistache::Http::Response,
      Pistache::Async::Barrier,
      Pistache::Async::IgnoreException,
      Pistache::Http::Code;
using Mahjong::PlayerController, Mahjong::Wind, Mahjong::Piece;

using GametableSDK::Helper::eventToJson, GametableSDK::Helper::parseHand;
using MahjongHttp::JSONHelper::parseEvent;

auto Network::connectToMatch(PlayerController& controller, std::string baseAddress) -> void {
  spdlog::info("Connecting with instance of ", controller.Name());
  Client client;
  client.init();

  auto [registered, token] = registerForMatch(client, baseAddress);

  if (!registered) {
    spdlog::error("Was unable to register with gametable sever successfully.");
    return;
  }

  blockUntilMatchReady(client, token);

  bool gameRunning = true;

  while (gameRunning) {
    auto [events, makeDecision, startingHand, seatWind, prevalentWind] = getMatchInfo(client, baseAddress, token);
    for (const auto& event : events) {
      if (event.type == Event::Type::Dora) {
        controller.RoundStart(startingHand, seatWind, prevalentWind);
      }

      if (event.type == Event::Type::End) {
        gameRunning = false;
      }

      controller.ReceiveEvent(event);
    }

    if (makeDecision) {
      sendDecision(client, baseAddress, token, controller.RetrieveDecision());
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

auto Network::registerForMatch(Client& client, std::string baseAddress) -> std::tuple<bool, std::string> {
  auto registerClient = client.post(baseAddress + "/register");
  auto registerResponse = registerClient.send();
  
  auto registered = false;
  std::string token;

  registerResponse.then([&token, &registered](Response res) {
    if (res.code() == Code::Ok) {
      Document d;
      d.Parse(res.body().c_str());
      token = d["playerToken"].GetString();
      spdlog::info("Registered Successfully. token ", token);
      registered = true;
    }
  }, IgnoreException);

  Barrier<Response> barrier(registerResponse);
  barrier.wait_for(std::chrono::seconds(5));

  return std::make_tuple(registered, token);
}

auto Network::blockUntilMatchReady(Client& client, std::string baseAddress) -> void {
  bool gameRunning = false;
  auto checkGame = client.get(baseAddress + "/status");
  while (!gameRunning) {
    auto statusResponse = checkGame.send();
    statusResponse.then([&gameRunning](Response res) {
      if (res.code() == Code::Ok) {
        Document d;
        d.Parse(res.body().c_str());
        gameRunning = d["gameRunning"].GetBool();
        spdlog::info(gameRunning);
      }
    }, IgnoreException);
    
    Barrier<Response> barrier(statusResponse);
    barrier.wait_for(std::chrono::seconds(5));

    spdlog::info("Waiting for game to start...");
    std::this_thread::sleep_for(std::chrono::seconds(2));
  }
}

auto Network::getMatchInfo(Client& client, std::string baseAddress, std::string token) -> std::tuple<std::vector<Event>, bool, std::vector<Piece>, Wind, Wind> {
  std::vector<Event> events;
  auto makeDecision = false;
  std::vector<Piece> startingHand;
  Wind seatWind;
  Wind prevalentWind;
  
  auto getEvents = client.get(baseAddress + "/events/" + token);
  auto eventResponse = getEvents.send();

  eventResponse.then([&events, &makeDecision, &startingHand, &seatWind, &prevalentWind](Response res) {
      if (res.code() == Code::Ok) {
        Document d;
        d.Parse(res.body().c_str());
        auto& eventList = d["queue"];
        for (SizeType i = 0; i < eventList.Size(); i++) {
          events.push_back(parseEvent(eventList[i]));
        }
        makeDecision = d["waitingOnDecision"].GetBool();
        startingHand = parseHand(d["round"]["startingHand"]);
        seatWind = Wind(d["round"]["seatWind"].GetUint());
        prevalentWind = Wind(d["round"]["prevalentWind"].GetUint());
      }
    }, IgnoreException);

    Barrier<Response> barrier(eventResponse);
    barrier.wait_for(std::chrono::seconds(5));

    return std::make_tuple(events, makeDecision, startingHand, seatWind, prevalentWind);
}

auto Network::sendDecision(Client& client, std::string baseAddress, std::string token, Event decision) -> void {
  auto sendDecision = client.post(baseAddress + "/makedecision/" + token);
  sendDecision.body(eventToJson(decision));
  sendDecision.send();
}
