/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "external/react-native-fast-image/skia/RSkComponentProviderFastImage.h"
#include "external/react-native-fast-image/skia/RSkComponentFastImage.h"
#include "external/react-native-fast-image/skia/react/renderer/components/fastImage/ReactNativeFastImageDescriptor.h"

namespace facebook {
namespace react {

RSkComponentProviderFastImage::RSkComponentProviderFastImage() {}

ComponentDescriptorProvider RSkComponentProviderFastImage::GetDescriptorProvider() {
  return concreteComponentDescriptorProvider<ReactNativeFastImageComponentDescriptor>();
}

std::shared_ptr<RSkComponent> RSkComponentProviderFastImage::CreateComponent(
    const ShadowView &shadowView) {
  return std::static_pointer_cast<RSkComponent>(
      std::make_shared<RSkComponentFastImage>(shadowView));
}

} // namespace react
} // namespace facebook
