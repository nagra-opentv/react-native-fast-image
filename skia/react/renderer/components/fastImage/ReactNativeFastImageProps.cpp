/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <react/renderer/core/propsConversions.h>

#include "ReactNativeFastImageProps.h"
#include <react/renderer/components/image/conversions.h>
#include "conversions.h"

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
          ImageResizeMode::Stretch)),
      tintColor(
          convertRawProp(rawProps, "tintColor", sourceProps.tintColor, {}))
           {}

} // namespace react
} // namespace facebook
