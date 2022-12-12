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
  enum class Priority { low, normal, high };
  enum class Cache { immutable, web, cacheOnly };
  std::string uri{};
  Priority priority{};
  Cache cache{};

};

using FastImageSources = std::vector<FastImageSource>;

} // namespace react
} // namespace facebook
