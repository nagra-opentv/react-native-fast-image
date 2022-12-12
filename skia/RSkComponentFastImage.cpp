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
      if(imageProps.sources[0].uri.substr(0, 14) == "file://assets/") {
      imageData = getLocalImageData(imageProps.sources[0].uri.c_str());
    } else if((imageProps.sources[0].uri.substr(0, 7) == "http://") || (imageProps.sources[0].uri.substr(0, 8) == "https://")) {
      requestNetworkImageData(imageProps.sources[0].uri);
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
     if(!(imageProps.sources[0].uri.substr(0, 7) == "http://" || (imageProps.sources[0].uri.substr(0, 8) == "https://"))) {
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

// To Do : For event, duplicating code for processing image data.
bool RSkComponentFastImage::processImageData(const char* path, char* response, int size) {
  auto component = getComponentData();
  auto const &imageProps = *std::static_pointer_cast<ReactNativeFastImageProps const>(component.props);
  /* Responce callback from network. Get image data, insert in Cache and call Onpaint*/
  sk_sp<SkImage> remoteImageData = RSkImageCacheManager::getImageCacheManagerInstance()->findImageDataInCache(path);
  if(remoteImageData ) {
    if(strcmp(path,imageProps.sources[0].uri.c_str()) == 0) {
      drawAndSubmit();
    }
  } else {
    if(!response) return false;
    sk_sp<SkData> data = SkData::MakeWithCopy(response,size);
    if (!data){
      RNS_LOG_ERROR("Unable to make SkData for path : " << path);
      return false;
    }
    remoteImageData = SkImage::MakeFromEncoded(data);
    if(!remoteImageData) return false;

    //Add in cache if image data is valid
    if(remoteImageData && canCacheData_){
      decodedimageCacheData imageCacheData;
      imageCacheData.imageData = remoteImageData;
      imageCacheData.expiryTime = (SkTime::GetMSecs() + cacheExpiryTime_);//convert sec to milisecond 60 *1000
      RSkImageCacheManager::getImageCacheManagerInstance()->imageDataInsertInCache(path, imageCacheData);
    }
    if(strcmp(path,imageProps.sources[0].uri.c_str()) == 0){
      networkImageData_ = remoteImageData;
      drawAndSubmit();
    }
  }
  return true;
}

// To Do : For event, duplicating the code for success and error event.
 void RSkComponentFastImage::sendErrorEvents() {
   imageEventEmitter_->onError();
   imageEventEmitter_->onLoadEnd();
   hasToTriggerEvent_ = false;
 }

void RSkComponentFastImage::sendSuccessEvents() {
   auto component = getComponentData();
   imageEventEmitter_->onLoad(component.layoutMetrics.frame.size);
   imageEventEmitter_->onLoadEnd();
   hasToTriggerEvent_ = false;
 }


} // namespace react
} // namespace facebook
