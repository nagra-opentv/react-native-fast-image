/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ReactNativeFastImageProps.h"
#include "conversions.h"
#include <react/renderer/core/propsConversions.h>

namespace facebook {
namespace react {

ReactNativeFastImageProps::ReactNativeFastImageProps(const ReactNativeFastImageProps &sourceProps, const RawProps &rawProps)
    : ViewProps(sourceProps, rawProps),
        sources(convertRawProp(rawProps, "source", sourceProps.sources, {})),
      defaultSources(convertRawProp(
          rawProps,
          "defaultSource",
          sourceProps.defaultSources,
          {})),
      resizeMode(convertRawProp(
          rawProps,
          "resizeMode",
          sourceProps.resizeMode,
          FastImageResizeMode::Stretch)),
      blurRadius(
          convertRawProp(rawProps, "blurRadius", sourceProps.blurRadius, {})),
      capInsets(
          convertRawProp(rawProps, "capInsets", sourceProps.capInsets, {})),
      tintColor(
          convertRawProp(rawProps, "tintColor", sourceProps.tintColor, {})),
      internal_analyticTag(convertRawProp(
          rawProps,
          "internal_analyticTag",
          sourceProps.internal_analyticTag,
          {})) {}

} // namespace react
} // namespace facebook
