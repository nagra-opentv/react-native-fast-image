/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#pragma once

#include "ReactSkia/components/RSkComponentImage.h"
#include "ReactSkia/views/common/RSkImageCacheManager.h"
#include "react/renderer/components/fastImage/ReactNativeFastImageShadowNode.h"

namespace facebook {
namespace react {

struct FastImgProps{
    ImageResizeMode resizeMode;
    SkColor tintColor;
};

class RSkComponentFastImage final : public RSkComponentImage {
 public:
   RSkComponentFastImage(const ShadowView &shadowView);
   ~RSkComponentFastImage();
   RnsShell::LayerInvalidateMask updateComponentProps(const ShadowView &newShadowView,bool forceUpdate) override;
 private:
   FastImgProps imageProps;
   std::shared_ptr<FastImageEventEmitter const> imageEventEmitter_;
   sk_sp<SkImage> networkImageData_{nullptr};
   bool processImageData(const char* path, char* response, int size) override;
   void requestNetworkImageData(std::string uri);
   void sendErrorEvents() override;
   void sendSuccessEvents() override;
 protected:
   void OnPaint(SkCanvas *canvas) override;
};

} // namespace react
} // namespace facebook
