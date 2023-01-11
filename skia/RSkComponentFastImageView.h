/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
* Copyright (C) Kudo Chien
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/
#pragma once

#include <mutex>
#include "include/core/SkRect.h"

#include "react/renderer/components/rnfastimage/ShadowNodes.h"

#include "ReactSkia/components/RSkComponent.h"
#include "ReactSkia/sdk/CurlNetworking.h"
#include "ReactSkia/views/common/RSkImageCacheManager.h"

#define DEFAULT_IMAGE_FILTER_QUALITY kLow_SkFilterQuality /*Skia's Defualt is kNone_SkFilterQuality*/
#define DEFAULT_MAX_CACHE_EXPIRY_TIME 1800000 // 30mins in milliseconds 1800000
#define RNS_NO_CACHE_STR "no-cache"
#define RNS_NO_STORE_STR "no-store"
#define RNS_MAX_AGE_0_STR "max-age=0"
#define RNS_MAX_AGE_STR "max-age"
namespace facebook {
namespace react {

struct FastImgProps{
    FastImageViewResizeMode resizeMode;
    SkColor tintColor;
};

class RSkComponentFastImage final : public RSkComponent {
 public:
  RSkComponentFastImage(const ShadowView &shadowView);
  ~RSkComponentFastImage();
  RnsShell::LayerInvalidateMask updateComponentProps(SharedProps newviewProps,bool forceUpdate) override;
 private :
  FastImgProps imageProps;
  std::shared_ptr<CurlRequest> remoteCurlRequest_{nullptr};
  atomic<bool> isRequestInProgress_{false};
  std::shared_ptr<FastImageViewEventEmitter const> fastImageViewEventEmitter_;
  sk_sp<SkImage> networkFastImageData_;
  bool hasToTriggerEvent_{false};
  bool canCacheData_{true};
  double cacheExpiryTime_{DEFAULT_MAX_CACHE_EXPIRY_TIME};

  sk_sp<SkImage> getLocalImageData(string sourceUri);
  void requestNetworkImageData(string sourceUri);

  inline bool checkRemoteUri(string sourceUri);
  inline string generateUriPath(string path);
  void drawAndSubmit();
  bool processImageData(const char* path, char* response, int size);
  inline void sendErrorEvents();
  inline void sendSuccessEvents(sk_sp<SkImage> imageData);

 protected:
  void OnPaint(SkCanvas *canvas) override;
};

} // namespace react
} // namespace facebook
