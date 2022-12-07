/*
* Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "ReactNativeFastImageEventEmitter.h"

namespace facebook {
namespace react {

void FastImageEventEmitter::onLoadStart() const {
  dispatchEvent("onFastImageLoadStart");
}

void FastImageEventEmitter::onLoad(Size size) const {
  dispatchEvent("onFastImageLoad", [=](jsi::Runtime &runtime) {
    auto payload = jsi::Object(runtime);
    payload.setProperty(runtime, "width", size.width);
    payload.setProperty(runtime, "height", size.height);
    return payload;
  });
}

void FastImageEventEmitter::onLoadEnd() const {
  dispatchEvent("onFastImageLoadEnd");
}

void FastImageEventEmitter::onProgress(double progress) const {
  dispatchEvent("progress", [=](jsi::Runtime &runtime) {
    auto payload = jsi::Object(runtime);
    payload.setProperty(runtime, "progress", progress);
    return payload;
  });
}

void FastImageEventEmitter::onError() const {
  dispatchEvent("onFastImageError");
}

} // namespace react
} // namespace facebook
