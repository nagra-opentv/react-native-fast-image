/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "ReactSkia/components/RSkComponentProvider.h"

namespace facebook {
namespace react {

class RSkComponentProviderFastImage : public RSkComponentProvider {
 public:
  RSkComponentProviderFastImage();

 public:
  ComponentDescriptorProvider GetDescriptorProvider() override;
  std::shared_ptr<RSkComponent> CreateComponent(
      const ShadowView &shadowView) override;
};

#ifdef __cplusplus
extern "C" {
#endif
RNS_USED RSkComponentProvider *RSkComponentProviderFastImageCls() {
  return new RSkComponentProviderFastImage();
}

#ifdef __cplusplus
}
#endif

} // namespace react
} // namespace facebook
