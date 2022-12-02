/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "react/renderer/components/view/ConcreteViewShadowNode.h"
#include "ReactNativeFastImageEventEmitter.h"
#include "ReactNativeFastImageProps.h"
#include "ReactNativeFastImageState.h"
#include "external/react-native-fast-image/skia/react/renderer/fastImageManager/primitives.h"

namespace facebook {
namespace react {

extern const char ReactNativeFastImageComponentName[];

/*
* `ShadowNode` for <ReactNativeFastImage> component.
*/
using ReactNativeFastImageShadowNode = ConcreteViewShadowNode<
    ReactNativeFastImageComponentName,
    ReactNativeFastImageProps,
    FastImageEventEmitter,
    ReactNativeFastImageState>;

} // namespace react
} // namespace facebook
