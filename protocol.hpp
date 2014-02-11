#pragma once
#include "utils.hpp"
#include "protocol/game.pb.h"


namespace swarm
{
  void ToProtocol(game::Vector2* lhs, const Vector2f& rhs);
  void FromProtocol(Vector2f* lhs, const game::Vector2& rhs);

  template <typename T>
  bool ProtobufFromFile(const char* filename, T* msg)
  {
    string str;
    if (!LoadFile(filename, &str))
      return false;

    return google::protobuf::TextFormat::ParseFromString(str, msg);
  }
}