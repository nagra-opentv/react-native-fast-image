/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "include/core/SkPaint.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkMaskFilterBase.h"
#include "rns_shell/compositor/layers/PictureLayer.h"
#include "RSkComponentFastImage.h"
#include "ReactSkia/views/common/RSkImageUtils.h"
#include "ReactSkia/views/common/RSkConversion.h"

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
      drawContentShadow(canvas,frameRect,imageTargetRect,imageData,imageProps,layerRef->shadowOffset,layerRef->shadowColor,layerRef->shadowOpacity);
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
    setPaintFilters(paint,imageProps,imageTargetRect,frameRect,false,imageData->isOpaque());
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

void RSkComponentFastImage::drawAndSubmit() {
  layer()->client().notifyFlushBegin();
  layer()->invalidate( RnsShell::LayerPaintInvalidate);
  if (layer()->type() == RnsShell::LAYER_TYPE_PICTURE) {
    RNS_PROFILE_API_OFF(getComponentData().componentName << " getPicture :", static_cast<RnsShell::PictureLayer*>(layer().get())->setPicture(getPicture()));
  }
  layer()->client().notifyFlushRequired();
}

inline void RSkComponentFastImage::drawContentShadow( SkCanvas *canvas,
                            SkRect frameRect,
                            SkRect imageTargetRect,
                            sk_sp<SkImage> imageData ,
                            const ReactNativeFastImageProps  &imageProps,
                            SkSize shadowOffset,
                            SkColor shadowColor,
                            float shadowOpacity){
  /*TO DO :When Frame doesn't have background, has border with Jpeg Image and no resize.
    currently drawing shadow for both border and content.
    Need to cross verify with reference and confirm the behaviour.*/
  SkRect shadowBounds;
  SkIRect shadowFrame;
  SkRect  frameBound;
/*On below special cases, content Shadow to be drawn on complete frame/Layout instead on Image/content frame :
  ------------------------------------------------------------------------------------------------------------
   1. The target size of Image > Frame's size. In that case, clipping will be done to contain the image
      within the frame, So shadow to be drawn conidering frame size.
   2. For Repeat mode, Target frame size is the size of the frame itself.[Image will be repeated to fill the frame]
*/
  bool shadowOnFrame=(( frameRect.width() < imageTargetRect.width()) || ( frameRect.height() < imageTargetRect.height()));
  if(shadowOnFrame) {
    //Shadow on Frame Boundary
    frameBound=frameRect;
    shadowFrame.setXYWH(frameRect.x() + shadowOffset.width(), frameRect.y() + shadowOffset.height(), frameRect.width(), frameRect.height());
  } else {
    //Shadow on Image/Content Boundary
    frameBound=imageTargetRect;
    shadowFrame.setXYWH(imageTargetRect.x() + shadowOffset.width(), imageTargetRect.y() + shadowOffset.height(), imageTargetRect.width(), imageTargetRect.height());
  }
  SkIRect shadowIBounds=RSkDrawUtils::getShadowBounds(shadowFrame,layer()->shadowMaskFilter,layer()->shadowImageFilter);
  shadowBounds=SkRect::Make(shadowIBounds);

  bool saveLayerDone=false;
//Apply Opacity
  if(shadowOpacity) {
    canvas->saveLayerAlpha(&shadowBounds,shadowOpacity);
    saveLayerDone=true;
  }

  SkPaint shadowPaint;
  setPaintFilters(shadowPaint,imageProps,imageTargetRect,frameRect,true,imageData->isOpaque());

  if(!imageData->isOpaque() ) {
//Apply Shadow for transparent image
    canvas->drawImageRect(imageData, imageTargetRect, &shadowPaint);
  } else {
//Apply Shadow for opaque image
    if(!saveLayerDone) {
      canvas->saveLayer(&shadowBounds,&shadowPaint);
      saveLayerDone =true;
    }
    // clipping done to avoid drawing on non visible area [Area under opque frame]
    canvas->clipRect(frameBound,SkClipOp::kDifference);
    shadowPaint.setColor(shadowColor);
    canvas->drawIRect(shadowFrame, shadowPaint);
  }
  if(saveLayerDone) {
    canvas->restore();
  }
#ifdef SHOW_SHADOW_BOUND
  SkPaint paint;
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setColor(SK_ColorGREEN);
  paint.setStrokeWidth(2);
  shadowBounds.join(frameBound);
  canvas->drawRect(shadowBounds,paint);
#endif
}

inline void RSkComponentFastImage::setPaintFilters (SkPaint &paintObj,const ReactNativeFastImageProps  &imageProps,
                                                      SkRect imageTargetRect,SkRect frameRect ,
                                                      bool  setFilterForShadow, bool opaqueImage) {
  
  //This function applies appropriate filter on paint to draw Shadow or Image.
   /*  Image Filter will be used on below scenario :
       -------------------------------------------
      1. For shadow on Image with transparent pixel
      2. For Image draw with Resize mide as "repeat"
      3. For Image Draw with with blur Effect.
   */
  if((setFilterForShadow && !opaqueImage)||
      (! setFilterForShadow &&
         ((imageProps.blurRadius > 0))
      )) {
      sk_sp<SkImageFilter> shadowFilter{nullptr};
      if(setFilterForShadow && (layer()->shadowImageFilter != nullptr) ) {
         shadowFilter=layer()->shadowImageFilter;
      }
      if(imageProps.blurRadius > 0) {
        shadowFilter = SkImageFilters::Blur(imageProps.blurRadius, imageProps.blurRadius,shadowFilter);
      }
      paintObj.setImageFilter(std::move(shadowFilter));
   } else if(setFilterForShadow && (layer()->shadowMaskFilter != nullptr)) {
      paintObj.setMaskFilter(layer()->shadowMaskFilter);
  }
}

inline bool shouldCacheData(std::string cacheControlData) {
  if(cacheControlData.find(RNS_NO_CACHE_STR) != std::string::npos) return false;
  else if(cacheControlData.find(RNS_NO_STORE_STR) != std::string::npos) return false;
  else if(cacheControlData.find(RNS_MAX_AGE_0_STR) != std::string::npos) return false;

  return true;
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
