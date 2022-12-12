/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "RSkComponentProviderFastImage.h"
#include "RSkComponentFastImage.h"
#include "react/renderer/components/fastImage/ReactNativeFastImageDescriptor.h"

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
