/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

void FastImageEventEmitter::onLoad() const {
  dispatchEvent("onFastImageLoad");
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
