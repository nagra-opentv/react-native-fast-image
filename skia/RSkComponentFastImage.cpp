/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "include/effects/SkImageFilters.h"
#include "src/core/SkMaskFilterBase.h"

#include "ReactSkia/views/common/RSkImageUtils.h"
#include "ReactSkia/views/common/RSkConversion.h"

#include "RSkComponentFastImage.h"

namespace facebook {
namespace react {

using namespace RSkImageUtils;
RSkComponentFastImage::RSkComponentFastImage(const ShadowView &shadowView)
  : RSkComponentImage(shadowView) {
       imageEventEmitter_ = std::static_pointer_cast<FastImageEventEmitter const>(shadowView.eventEmitter);
}
RSkComponentFastImage::~RSkComponentFastImage(){};

void RSkComponentFastImage::OnPaint(SkCanvas *canvas) {
  sk_sp<SkImage> imageData{nullptr};
  string path;
  auto component = getComponentData();
  ReactNativeFastImageProps const &imageProps = *std::static_pointer_cast<ReactNativeFastImageProps const>(component.props);
  //First to check file entry presence. If not exist, generate imageData.
  do {
    if(networkImageData_) {
        imageData = networkImageData_;
      break;
    }
    if(imageProps.sources.empty()) break;
    imageData = RSkImageCacheManager::getImageCacheManagerInstance()->findImageDataInCache(imageProps.sources[0].uri.c_str());
    if(imageData) break;
    if (imageProps.sources[0].type == FastImageSource::Type::Local) {
      imageData = getLocalImageData(imageProps.sources[0].uri.c_str());
    } else if(imageProps.sources[0].type == FastImageSource::Type::Remote) {
      requestNetworkImageData(imageProps.sources[0].uri.c_str());
    }
  } while(0);

  Rect frame = component.layoutMetrics.frame;
  SkRect frameRect = SkRect::MakeXYWH(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
  auto const &imageBorderMetrics=imageProps.resolveBorderMetrics(component.layoutMetrics);

  // Draw order 1.Shadow 2. Background 3.Image Shadow 4. Image 5.Border
  bool hollowFrame = false;
  bool needClipAndRestore =false;
  sk_sp<SkImageFilter> imageFilter;
  auto  layerRef=layer();
  if(layer()->isShadowVisible) {
    /*Draw Shadow on Frame*/
    hollowFrame=drawShadow(canvas,frame,imageBorderMetrics,
                              imageProps.backgroundColor,
                              layerRef->shadowColor,layerRef->shadowOffset,layerRef->shadowOpacity,
                              layerRef->opacity,
                              layerRef->shadowImageFilter,layerRef->shadowMaskFilter
                          );
  }
  /*Draw Frame BackGround*/
  drawBackground(canvas,frame,imageBorderMetrics,imageProps.backgroundColor);
  if(imageData) {
    SkRect imageTargetRect = computeTargetRect({imageData->width(),imageData->height()},frameRect,imageProps.resizeMode);
    SkPaint paint;
    /*Draw Image Shadow on below scenario:
      ------------------------------------
      1. Has visible shadow. but both border & background not avialble [case of ShadowDrawnMode::ShadowOnContent]
      2. Shadow Drawn on Border[case of ShadowDrawnMode::ShadowOnBorder], But Image is either transparent  or smaller than the same
    */
    if(hollowFrame) {
        //TODO: For the content Shadow, currently Shadow drawn for both Border[if avaialble] & Content[Image].
        //      This behaviour to be cross verified with reference.
      drawContentShadow(canvas,frameRect,imageTargetRect,imageData,imageProps.resizeMode,imageProps.blurRadius,layerRef->shadowOffset,layerRef->shadowColor,layerRef->shadowOpacity);
    }
    /*Draw Image */
    if(( frameRect.width() < imageTargetRect.width()) || ( frameRect.height() < imageTargetRect.height())) {
      needClipAndRestore= true;
    }
    /* clipping logic to be applied if computed Frame is greater than the target.*/
    if(needClipAndRestore) {
        canvas->save();
        canvas->clipRect(frameRect,SkClipOp::kIntersect);
    }
    /* TODO: Handle filter quality based of configuration. Setting Low Filter Quality as default for now*/
    paint.setFilterQuality(DEFAULT_IMAGE_FILTER_QUALITY);
    setPaintFilters(paint,imageProps.resizeMode,imageProps.blurRadius,imageTargetRect,frameRect,false,imageData->isOpaque());
    canvas->drawImageRect(imageData,imageTargetRect,&paint);
    if(needClipAndRestore) {
      canvas->restore();
    }
    networkImageData_ = nullptr;
    drawBorder(canvas,frame,imageBorderMetrics,imageProps.backgroundColor);
    // Emitting Load completed Event
    if(hasToTriggerEvent_) sendSuccessEvents();

  } else {
  /* Emitting Image Load failed Event*/
    if(imageProps.sources[0].type != FastImageSource::Type::Remote) {
      if(!hasToTriggerEvent_) {
        imageEventEmitter_->onLoadStart();
        hasToTriggerEvent_ = true;
      }
      if(hasToTriggerEvent_) sendErrorEvents();
      RNS_LOG_ERROR("Image not loaded :"<<imageProps.sources[0].uri.c_str());
    }
  }
}

RnsShell::LayerInvalidateMask RSkComponentFastImage::updateComponentProps(const ShadowView &newShadowView,bool forceUpdate) {

    auto const &newimageProps = *std::static_pointer_cast<ReactNativeFastImageProps const>(newShadowView.props);
    auto component = getComponentData();
    auto const &oldimageProps = *std::static_pointer_cast<ReactNativeFastImageProps const>(component.props);
    RnsShell::LayerInvalidateMask updateMask=RnsShell::LayerInvalidateNone;

    if((forceUpdate) || (oldimageProps.resizeMode != newimageProps.resizeMode)) {
      imageProps.resizeMode = newimageProps.resizeMode;
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
    }
    if((forceUpdate) || (oldimageProps.tintColor != newimageProps.tintColor )) {
      /* TODO : Needs implementation*/
      imageProps.tintColor = RSkColorFromSharedColor(newimageProps.tintColor,SK_ColorTRANSPARENT);
    }
    if((forceUpdate) || (oldimageProps.sources[0].uri.compare(newimageProps.sources[0].uri) != 0)) {
      imageEventEmitter_->onLoadStart();
      hasToTriggerEvent_ = true;
    }
    return updateMask;
}
// To Do : For event, duplicating the code for success and error event.
 void RSkComponentFastImage::sendErrorEvents() {
   imageEventEmitter_->onError();
   imageEventEmitter_->onLoadEnd();
   hasToTriggerEvent_ = false;
 }

void RSkComponentFastImage::sendSuccessEvents() {
   imageEventEmitter_->onLoad();
   imageEventEmitter_->onLoadEnd();
   hasToTriggerEvent_ = false;
 }


} // namespace react
} // namespace facebook
