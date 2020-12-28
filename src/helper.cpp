#include "gametablesdk/helper.h"

#include "libmahjonghttp/jsonhelper.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "event.h"

using namespace GametableSDK;

using Mahjong::Event, Mahjong::Piece;
using rapidjson::StringBuffer, rapidjson::Writer, rapidjson::SizeType;
using MahjongHttp::JSONHelper::writeValue;

auto Helper::eventToJson(Event event) -> std::string {
  StringBuffer sb;
  Writer<StringBuffer> writer(sb);
  writeValue(writer, event);
  return sb.GetString();
}

auto Helper::parseHand(Value& list) -> std::vector<Piece> {
  std::vector<Piece> result;
  for (SizeType i = 0; i < list.Size(); i++) {
    result.push_back(Piece(list[i].GetUint()));
  }
  return result;
}
