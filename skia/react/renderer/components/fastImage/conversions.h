/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <better/map.h>
#include <folly/dynamic.h>
#include <react/renderer/graphics/conversions.h>
#include "external/react-native-fast-image/skia/react/renderer/fastImageManager/primitives.h"
namespace facebook {
namespace react {

inline void fromRawValue(const RawValue &value, FastImageSource &result) {

   if (value.hasType<better::map<std::string, RawValue>>()) {
    auto items = (better::map<std::string, RawValue>)value;
    result = {};
    if (items.find("url") != items.end() &&
        // The following should be removed after codegen is shipped.
        // See T45151459.
        items.at("url").hasType<std::string>()) {
      result.uri = (std::string)items.at("url");
    }

    if (items.find("uri") != items.end() &&
        // The following should be removed after codegen is shipped.
        // See T45151459.
        items.at("uri").hasType<std::string>()) {
      result.uri = (std::string)items.at("uri");
    }

    if (items.find("priority") != items.end() &&
      items.at("priority").hasType<std::string>()) {
      auto stringValue = (std::string)items.at("priority");
      if(stringValue == "low") {
        result.priority = FastImageSource::Priority::low;
      } else if (stringValue == "normal") {
        result.priority = FastImageSource::Priority::normal;
      }else if (stringValue == "high") {
        result.priority = FastImageSource::Priority::high;
      }
    }

    if (items.find("cache") != items.end() &&
      items.at("cache").hasType<std::string>()) {
      auto stringValue = (std::string)items.at("cache");
      if(stringValue == "immutable") {
        result.cache = FastImageSource::Cache::immutable;
      } else if (stringValue == "web") {
        result.cache = FastImageSource::Cache::web;
      }else if (stringValue == "cacheOnly") {
        result.cache = FastImageSource::Cache::cacheOnly;
      }
    }
    return;
  }

  // The following should be removed after codegen is shipped.
  // See T45151459.
  result = {};
}

} // namespace react
} // namespace facebook
