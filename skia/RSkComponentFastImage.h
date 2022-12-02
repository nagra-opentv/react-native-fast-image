/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#pragma once

#include "include/core/SkRect.h"
#include "external/react-native-fast-image/skia/react/renderer/components/fastImage/ReactNativeFastImageShadowNode.h"
#include "external/react-native-fast-image/skia/react/renderer/components/fastImage/ReactNativeFastImageEventEmitter.h"
#include "ReactSkia/components/RSkComponent.h"
#include "ReactSkia/components/RSkComponentImage.h"
#include "ReactSkia/views/common/RSkImageCacheManager.h"

namespace facebook {
namespace react {

struct FastImgProps{
    FastImageResizeMode resizeMode;
    SkColor tintColor;
};
class RSkComponentFastImage : public RSkComponentImage {
 public:
   RSkComponentFastImage(const ShadowView &shadowView);
   ~RSkComponentFastImage();
   RnsShell::LayerInvalidateMask updateComponentProps(const ShadowView &newShadowView,bool forceUpdate) override;
 private:
  FastImgProps imageProps;
  std::shared_ptr<FastImageEventEmitter const> imageEventEmitter_;
  sk_sp<SkImage> networkImageData_{nullptr};
  inline void drawContentShadow(SkCanvas *canvas,
                              SkRect frameRect,/*actual image frame*/
                              SkRect imageTargetRect,/*area of draw image and shadow*/
                              sk_sp<SkImage> imageData,
                              const ReactNativeFastImageProps  &imageProps,
                              SkSize shadowOffset,
                              SkColor shadowColor,
                              float shadowOpacity);
  inline void setPaintFilters (SkPaint &paintObj,const ReactNativeFastImageProps  &imageProps,
                              SkRect targetRect,SkRect frameRect,
                              bool  filterForShadow, bool isOpaque);
   void sendErrorEvents() override;
   void sendSuccessEvents() override;
   void drawAndSubmit() override;
 protected:
  void OnPaint(SkCanvas *canvas) override;
};

} // namespace react
} // namespace facebook
