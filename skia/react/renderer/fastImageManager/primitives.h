/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <string>
#include <vector>

#include <react/renderer/graphics/Geometry.h>

namespace facebook {
namespace react {

class FastImageSource {
 public:
  enum class Type { Invalid, Remote, Local };

  Type type{};
  std::string uri{};
  std::string bundle{};
  Float scale{3};
  Size size{0};

  bool operator==(const FastImageSource &rhs) const {
    return std::tie(this->type, this->uri,this->size) == std::tie(rhs.type, rhs.uri,rhs.size);
  }

  bool operator!=(const FastImageSource &rhs) const {
    return !(*this == rhs);
  }
};

using FastImageSources = std::vector<FastImageSource>;

} // namespace react
} // namespace facebook
