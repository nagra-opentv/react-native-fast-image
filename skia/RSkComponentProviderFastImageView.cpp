/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
 */

#include "RSkComponentProviderFastImageView.h"
#include "RSkComponentFastImageView.h"
#include "react/renderer/components/rnfastimage/ComponentDescriptors.h"

namespace facebook {
namespace react {

RSkComponentProviderFastImage::RSkComponentProviderFastImage() {}

ComponentDescriptorProvider RSkComponentProviderFastImage::GetDescriptorProvider() {
  return concreteComponentDescriptorProvider<FastImageViewComponentDescriptor>();
}

std::shared_ptr<RSkComponent> RSkComponentProviderFastImage::CreateComponent(
    const ShadowView &shadowView) {
  return std::static_pointer_cast<RSkComponent>(
      std::make_shared<RSkComponentFastImage>(shadowView));
}

} // namespace react
} // namespace facebook
