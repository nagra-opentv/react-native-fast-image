/*
 * Copyright (C) 1994-2021 OpenTV, Inc. and Nagravision S.A.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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
