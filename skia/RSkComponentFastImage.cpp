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

#include "rns_shell/compositor/layers/PictureLayer.h"
#include "RSkComponentFastImage.h"

namespace facebook {
namespace react {

using namespace RSkImageUtils;
RSkComponentFastImage::RSkComponentFastImage(const ShadowView &shadowView)
  : RSkComponent(shadowView) {
       imageEventEmitter_ = std::static_pointer_cast<FastImageViewEventEmitter const>(shadowView.eventEmitter);
       networkImageData_= nullptr;
}
RSkComponentFastImage::~RSkComponentFastImage(){};

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
      imageData = getLocalImageData(imageProps.source.uri.c_str());
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

// To Do : For event, duplicating code for processing image data.In future will be remove.
bool RSkComponentFastImage::processImageData(const char* path, char* response, int size) {
  decodedimageCacheData imageCacheData;
  auto component = getComponentData();
  auto const &imageProps = *std::static_pointer_cast<FastImageViewProps const>(component.props);
  /* Responce callback from network. Get image data, insert in Cache and call Onpaint*/
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

inline bool shouldCacheData(std::string cacheControlData) {
  if(cacheControlData.find(RNS_NO_CACHE_STR) != std::string::npos) return false;
  else if(cacheControlData.find(RNS_NO_STORE_STR) != std::string::npos) return false;
  else if(cacheControlData.find(RNS_MAX_AGE_0_STR) != std::string::npos) return false;

  return true;
}

inline double getCacheMaxAgeDuration(std::string cacheControlData) {
  size_t maxAgePos = cacheControlData.find(RNS_MAX_AGE_STR);
  if(maxAgePos != std::string::npos) {
    size_t maxAgeEndPos = cacheControlData.find(';',maxAgePos);
    return std::stoi(cacheControlData.substr(maxAgePos+8,maxAgeEndPos));
  }
  return DEFAULT_MAX_CACHE_EXPIRY_TIME;
}

// To Do : For event, duplicating code for requesting network image data.In future will be remove.
void RSkComponentFastImage::requestNetworkImageData(std::string uri) {
  auto sharedCurlNetworking = CurlNetworking::sharedCurlNetworking();
  std::shared_ptr<CurlRequest> remoteCurlRequest = std::make_shared<CurlRequest>(nullptr,uri,0,"GET");
  folly::dynamic query = folly::dynamic::object();

  //Before network request, reset the cache info with default values
  canCacheData_ = true;
  cacheExpiryTime_ = DEFAULT_MAX_CACHE_EXPIRY_TIME;

  // headercallback lambda fuction
  auto headerCallback =  [this, remoteCurlRequest](void* curlresponseData,void *userdata)->bool {
    CurlResponse *responseData =  (CurlResponse *)curlresponseData;
    CurlRequest *curlRequest = (CurlRequest *) userdata;
    
    double responseMaxAgeTime = DEFAULT_MAX_CACHE_EXPIRY_TIME;
    // Parse server response headers and retrieve caching details
    auto responseCacheControlData = responseData->headerBuffer.find("Cache-Control");
    if(responseCacheControlData != responseData->headerBuffer.items().end()) {
      std::string responseCacheControlString = responseCacheControlData->second.asString();
      canCacheData_ = shouldCacheData(responseCacheControlString);
      if(canCacheData_) responseMaxAgeTime = getCacheMaxAgeDuration(responseCacheControlString);
    }

    // TODO : Parse request headers and retrieve caching details

    cacheExpiryTime_ = std::min(responseMaxAgeTime,static_cast<double>(DEFAULT_MAX_CACHE_EXPIRY_TIME));
    RNS_LOG_DEBUG("url [" << curlRequest->URL.c_str() << "] canCacheData[" << canCacheData_ << "] cacheExpiryTime[" << cacheExpiryTime_ << "]");
    return 0;
  };

  // completioncallback lambda fuction
  auto completionCallback =  [this, remoteCurlRequest](void* curlresponseData,void *userdata)->bool {
    CurlResponse *responseData =  (CurlResponse *)curlresponseData;
    CurlRequest * curlRequest = (CurlRequest *) userdata;
    if((!responseData
        || !processImageData(curlRequest->URL.c_str(),responseData->responseBuffer,responseData->contentSize)) && (hasToTriggerEvent_)) {
      this->sendErrorEvents();
    }
    //Reset the lamda callback so that curlRequest shared pointer dereffered from the lamda
    // and gets auto destructored after the completion callback.
    remoteCurlRequest->curldelegator.CURLNetworkingHeaderCallback = nullptr;
    remoteCurlRequest->curldelegator.CURLNetworkingCompletionCallback = nullptr;
    return 0;
  };
  FastImageViewEventEmitter::OnFastImageLoadStart imagestart;
  remoteCurlRequest->curldelegator.delegatorData = remoteCurlRequest.get();
  remoteCurlRequest->curldelegator.CURLNetworkingHeaderCallback = headerCallback;
  remoteCurlRequest->curldelegator.CURLNetworkingCompletionCallback=completionCallback;
  if(!hasToTriggerEvent_) {
    this->imageEventEmitter_->onFastImageLoadStart(imagestart);
    hasToTriggerEvent_ = true;
  }
  sharedCurlNetworking->sendRequest(remoteCurlRequest,query);
}

// To Do : For event, duplicating the code for success and error event.In future will be remove.
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

} // namespace react
} // namespace facebook
