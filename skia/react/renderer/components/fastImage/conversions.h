/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
  if (value.hasType<std::string>()) {
    result = {
        /* .type = */ FastImageSource::Type::Remote,
        /* .uri = */ (std::string)value,
    };
    return;
  }

  if (value.hasType<better::map<std::string, RawValue>>()) {
    auto items = (better::map<std::string, RawValue>)value;
    result = {};

    result.type = FastImageSource::Type::Remote;

    if (items.find("__packager_asset") != items.end()) {
      result.type = FastImageSource::Type::Local;
    }

    if (items.find("width") != items.end() &&
        items.find("height") != items.end() &&
        // The following checks have to be removed after codegen is shipped.
        // See T45151459.
        items.at("width").hasType<Float>() &&
        items.at("height").hasType<Float>()) {
      result.size = {(Float)items.at("width"), (Float)items.at("height")};
    }

    if (items.find("scale") != items.end() &&
        // The following checks have to be removed after codegen is shipped.
        // See T45151459.
        items.at("scale").hasType<Float>()) {
      result.scale = (Float)items.at("scale");
    } else {
      result.scale = items.find("deprecated") != items.end() ? 0.0 : 1.0;
    }

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

    if (items.find("bundle") != items.end() &&
        // The following should be removed after codegen is shipped.
        // See T45151459.
        items.at("bundle").hasType<std::string>()) {
      result.bundle = (std::string)items.at("bundle");
      result.type = FastImageSource::Type::Local;
    }

    return;
  }

  // The following should be removed after codegen is shipped.
  // See T45151459.
  result = {};
  result.type = FastImageSource::Type::Invalid;
}

inline std::string toString(const FastImageSource &value) {
  return "{uri: " + value.uri + "}";
}

inline void fromRawValue(const RawValue &value, FastImageResizeMode &result) {
  assert(value.hasType<std::string>());
  auto stringValue = (std::string)value;
  if (stringValue == "cover") {
    result = FastImageResizeMode::Cover;
    return;
  }
  if (stringValue == "contain") {
    result = FastImageResizeMode::Contain;
    return;
  }
  if (stringValue == "stretch") {
    result = FastImageResizeMode::Stretch;
    return;
  }
  if (stringValue == "center") {
    result = FastImageResizeMode::Center;
    return;
  }
  abort();
}

inline std::string toString(const FastImageResizeMode &value) {
  switch (value) {
    case FastImageResizeMode::Cover:
      return "cover";
    case FastImageResizeMode::Contain:
      return "contain";
    case FastImageResizeMode::Stretch:
      return "stretch";
    case FastImageResizeMode::Center:
      return "center";
  }
}

} // namespace react
} // namespace facebook
