/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#pragma once

#include <react/renderer/components/view/ViewProps.h>
#include "react/renderer/components/image/ImageShadowNode.h"

#include "external/react-native-fast-image/skia/react/renderer/fastImageManager/primitives.h"

namespace facebook {
namespace react {

// TODO (T28334063): Consider for codegen.
class ReactNativeFastImageProps final : public ViewProps {
 public:
  ReactNativeFastImageProps() = default;
  ReactNativeFastImageProps(const ReactNativeFastImageProps &sourceProps, const RawProps &rawProps);

#pragma mark - Props

  const FastImageSources sources{};
  const FastImageSources defaultSources{};
  const ImageResizeMode resizeMode{ImageResizeMode::Stretch};
  const SharedColor tintColor{};
};

} // namespace react
} // namespace facebook
