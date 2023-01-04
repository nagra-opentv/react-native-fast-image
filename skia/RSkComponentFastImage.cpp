/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
* Copyright (C) Kudo Chien
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

/*
  Right now FastImage is duplicate of Image component.
  This can be avoided buy inheritance . But when we did so faced
  random crashes which was difficult to triage.
  Hence for now we have implemented this component as duplicate to Image component.
*/

#include "include/effects/SkImageFilters.h"
#include "src/core/SkMaskFilterBase.h"

#include "rns_shell/compositor/layers/PictureLayer.h"

#include "ReactSkia/views/common/RSkImageUtils.h"
#include "ReactSkia/views/common/RSkConversion.h"
#include "ReactSkia/utils/RnsUtils.h"
#include "RSkComponentFastImage.h"

namespace facebook {
namespace react {

using namespace RSkImageUtils;

RSkComponentFastImage::RSkComponentFastImage(const ShadowView &shadowView)
    : RSkComponent(shadowView) {
      imageEventEmitter_ = std::static_pointer_cast<FastImageViewEventEmitter const>(shadowView.eventEmitter);
}

void RSkComponentFastImage::OnPaint(SkCanvas *canvas) {
  sk_sp<SkImage> imageData{nullptr};
  string path;
  auto component = getComponentData();
  FastImageViewProps const &imageProps = *std::static_pointer_cast<FastImageViewProps const>(component.props);
  //First to check file entry presence. If not exist, generate imageData.
  do {
    if(networkImageData_) {
      imageData = networkImageData_;
      break;
    }
    if(imageProps.source.uri.empty()) break;
    imageData = RSkImageCacheManager::getImageCacheManagerInstance()->findImageDataInCache(imageProps.source.uri.c_str());
    if(imageData) break;

    if(imageProps.source.uri.substr(0, 14) == "file://assets/") {
      imageData = getLocalImageData(imageProps.source.uri);
    } else if((imageProps.source.uri.substr(0, 7) == "http://") || (imageProps.source.uri.substr(0, 8) == "https://")) {
      requestNetworkImageData(imageProps.source.uri);
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
  if(layerRef->isShadowVisible) {
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
    SkRect imageTargetRect = computeTargetRect({imageData->width(),imageData->height()},frameRect,ImageResizeMode::Cover);
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
    FastImageViewEventEmitter::OnFastImageLoadStart imagestart;
     if(!(imageProps.source.uri.substr(0, 7) == "http://" || (imageProps.source.uri.substr(0, 8) == "https://"))) {
      if(!hasToTriggerEvent_) {
        imageEventEmitter_->onFastImageLoadStart(imagestart);
        hasToTriggerEvent_ = true;
      }
      if(hasToTriggerEvent_) sendErrorEvents();
      RNS_LOG_ERROR("Image not loaded :"<<imageProps.source.uri);
    }
  }
}


sk_sp<SkImage> RSkComponentFastImage::getLocalImageData(string sourceUri) {
  sk_sp<SkImage> imageData{nullptr};
  sk_sp<SkData> data;
  string path;
  decodedimageCacheData imageCacheData;
  FastImageViewEventEmitter::OnFastImageLoadStart imagestart;
  path = generateUriPath(sourceUri.c_str());
  if(!path.c_str()) {
    RNS_LOG_ERROR("Invalid File");
    return nullptr;
  }
  data = SkData::MakeFromFileName(path.c_str());
  if (!data) {
    RNS_LOG_ERROR("Unable to make SkData for path : " << path.c_str());
    return nullptr;
  }
  imageData = SkImage::MakeFromEncoded(data);
  if(imageData) {
    imageCacheData.imageData = imageData;
    imageCacheData.expiryTime = (SkTime::GetMSecs() + DEFAULT_MAX_CACHE_EXPIRY_TIME);//convert min to millisecond 30 min *60 sec *1000
    RSkImageCacheManager::getImageCacheManagerInstance()->imageDataInsertInCache(sourceUri.c_str(), imageCacheData);
  }
  if(!hasToTriggerEvent_) {
    imageEventEmitter_->onFastImageLoadStart(imagestart);
    hasToTriggerEvent_ = true;
  }

#ifdef RNS_IMAGE_CACHE_USAGE_DEBUG
    printCacheUsage();
#endif //RNS_IMAGECACHING_DEBUG
  return imageData;
}

inline string RSkComponentFastImage::generateUriPath(string path) {
  if(path.substr(0, 14) == "file://assets/")
    path = "./" + path.substr(7);
  return path;
}

RnsShell::LayerInvalidateMask RSkComponentFastImage::updateComponentProps(SharedProps newviewProps,bool forceUpdate) {

    auto const &newimageProps = *std::static_pointer_cast<FastImageViewProps const>(newviewProps);
    auto component = getComponentData();
    auto const &oldimageProps = *std::static_pointer_cast<FastImageViewProps const>(component.props);
    RnsShell::LayerInvalidateMask updateMask=RnsShell::LayerInvalidateNone;
    FastImageViewEventEmitter::OnFastImageLoadStart imagestart;

    if((forceUpdate) || (oldimageProps.resizeMode != newimageProps.resizeMode)) {
      imageProps.resizeMode = newimageProps.resizeMode;
      updateMask =static_cast<RnsShell::LayerInvalidateMask>(updateMask | RnsShell::LayerInvalidateAll);
    }
    if((forceUpdate) || (oldimageProps.tintColor != newimageProps.tintColor )) {
      /* TODO : Needs implementation*/
      imageProps.tintColor = RSkColorFromSharedColor(newimageProps.tintColor,SK_ColorTRANSPARENT);
    }
    if((forceUpdate) || (oldimageProps.source.uri.compare(newimageProps.source.uri) != 0)) {
      if( isRequestInProgress_ && remoteCurlRequest_){
        // if url is changed, image component is get component property update.
        // cancel the onging request and made new request to network.
        CurlNetworking::sharedCurlNetworking()->abortRequest(remoteCurlRequest_);
        remoteCurlRequest_ = nullptr;
        //TODO - need to send the onEnd event to APP if it is abort.
        isRequestInProgress_=false;
      }
      imageEventEmitter_->onFastImageLoadStart(imagestart);
      hasToTriggerEvent_ = true;
    }
    return updateMask;
}

void RSkComponentFastImage::drawAndSubmit() {
  layer()->client().notifyFlushBegin();
  layer()->invalidate( RnsShell::LayerPaintInvalidate);
  if (layer()->type() == RnsShell::LAYER_TYPE_PICTURE) {
    RNS_PROFILE_API_OFF(getComponentData().componentName << " getPicture :", static_cast<RnsShell::PictureLayer*>(layer().get())->setPicture(getPicture()));
  }
  layer()->client().notifyFlushRequired();
}

// callback for remoteImageData
bool RSkComponentFastImage::processImageData(const char* path, char* response, int size) {
  decodedimageCacheData imageCacheData;
  auto component = getComponentData();
  auto const &imageProps = *std::static_pointer_cast<FastImageViewProps const>(component.props);
  // Responce callback from network. Get image data, insert in Cache and call Onpaint
  sk_sp<SkImage> remoteImageData = RSkImageCacheManager::getImageCacheManagerInstance()->findImageDataInCache(path);
  if(remoteImageData ) {
    if(strcmp(path,imageProps.source.uri.c_str()) == 0) {
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
      imageCacheData.imageData = remoteImageData;
      imageCacheData.expiryTime = (SkTime::GetMSecs() + cacheExpiryTime_);//convert sec to milisecond 60 *1000
      RSkImageCacheManager::getImageCacheManagerInstance()->imageDataInsertInCache(path, imageCacheData);
    }
    if(strcmp(path,imageProps.source.uri.c_str()) == 0){
      networkImageData_ = remoteImageData;
      drawAndSubmit();
    }
  }
  return true;
}

void RSkComponentFastImage::requestNetworkImageData(string sourceUri) {
  remoteCurlRequest_ = std::make_shared<CurlRequest>(nullptr,sourceUri,0,"GET");

  folly::dynamic query = folly::dynamic::object();

  //Before network request, reset the cache info with default values
  canCacheData_ = true;
  cacheExpiryTime_ = DEFAULT_MAX_CACHE_EXPIRY_TIME;

  // headercallback lambda fuction
  auto headerCallback =  [this, weakThis = this->weak_from_this()](void* curlresponseData,void *userdata)->size_t {
    auto isAlive = weakThis.lock();
    if(!isAlive) {
       RNS_LOG_WARN("This object is already destroyed. ignoring the completion callback");
       return 0;
     }

    CurlResponse *responseData =  (CurlResponse *)curlresponseData;
    CurlRequest *curlRequest = (CurlRequest *) userdata;

    // Parse server response headers and retrieve caching details
    auto responseCacheControlData = responseData->headerBuffer.find("Cache-Control");
    if(responseCacheControlData != responseData->headerBuffer.items().end()) {
      std::string responseCacheControlString = responseCacheControlData->second.asString();
      canCacheData_ = remoteCurlRequest_->shouldCacheData();
      if(canCacheData_) cacheExpiryTime_ = responseData->cacheExpiryTime;
    }
    RNS_LOG_DEBUG("url [" << curlRequest->URL.c_str() << "] canCacheData[" << canCacheData_ << "] cacheExpiryTime[" << cacheExpiryTime_ << "]");
    return 0;
  };


  // completioncallback lambda fuction
  auto completionCallback =  [this, weakThis = this->weak_from_this()](void* curlresponseData,void *userdata)->bool {
    auto isAlive = weakThis.lock();
    if(!isAlive) {
      RNS_LOG_WARN("This object is already destroyed. ignoring the completion callback");
      return 0;
    }
    CurlResponse *responseData =  (CurlResponse *)curlresponseData;
    CurlRequest * curlRequest = (CurlRequest *) userdata;
    if((!responseData
        || !processImageData(curlRequest->URL.c_str(),responseData->responseBuffer,responseData->contentSize)) && (hasToTriggerEvent_)) {
      sendErrorEvents();
    }
    isRequestInProgress_=false;
    remoteCurlRequest_ = nullptr;
    return 0;
  };

  FastImageViewEventEmitter::OnFastImageLoadStart imagestart;
  remoteCurlRequest_->curldelegator.delegatorData = remoteCurlRequest_.get();
  remoteCurlRequest_->curldelegator.CURLNetworkingHeaderCallback = headerCallback;
  remoteCurlRequest_->curldelegator.CURLNetworkingCompletionCallback=completionCallback;
  if(!hasToTriggerEvent_) {
    imageEventEmitter_->onFastImageLoadStart(imagestart);
    hasToTriggerEvent_ = true;
  }
  CurlNetworking::sharedCurlNetworking()->sendRequest(remoteCurlRequest_,query);
  isRequestInProgress_ = true;
}

void RSkComponentFastImage::sendErrorEvents() {
   FastImageViewEventEmitter::OnFastImageError imageError;
   FastImageViewEventEmitter::OnFastImageLoadEnd imageLoad;
   imageEventEmitter_->onFastImageError(imageError);
   imageEventEmitter_->onFastImageLoadEnd(imageLoad);
   hasToTriggerEvent_ = false;
 }

void RSkComponentFastImage::sendSuccessEvents() {
   auto component = getComponentData();
   FastImageViewEventEmitter::OnFastImageLoadEnd imageLoad;
   FastImageViewEventEmitter::OnFastImageLoad ImageLoadsize;
   ImageLoadsize.width=component.layoutMetrics.frame.size.width;
   ImageLoadsize.height = component.layoutMetrics.frame.size.height;
   imageEventEmitter_->onFastImageLoad(ImageLoadsize);
   imageEventEmitter_->onFastImageLoadEnd(imageLoad);
   hasToTriggerEvent_ = false;
 }


RSkComponentFastImage::~RSkComponentFastImage(){
  // Image component is request send to network by then component is deleted.
  // still the network component will process the request, by calling abort.
  // will reduces the load on network and improve the performance.
  if(isRequestInProgress_ && remoteCurlRequest_){
    //TODO - need to send the onEnd event to APP if it is abort.
    CurlNetworking::sharedCurlNetworking()->abortRequest(remoteCurlRequest_);
    isRequestInProgress_=false;
  }
}

} // namespace react
} // namespace facebook
