/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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
