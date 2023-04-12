/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cxxreact/JsArgumentHelpers.h>

#include "ReactSkia/utils/RnsLog.h"
#include "ReactSkia/views/common/RSkImageCacheManager.h"

#include "FastImageViewModule.h"

using namespace folly;
namespace facebook {
using namespace react;
namespace xplat {

FastImageViewModule::FastImageViewModule() { }

auto FastImageViewModule::getConstants() -> std::map<std::string, folly::dynamic> {
  return {};
}

std::string FastImageViewModule::getName() {
  return "FastImageView";
}

auto FastImageViewModule::getMethods() -> std::vector<Method> {
  return {
    Method(
      "preload",
      [this](dynamic args) {
        RNS_LOG_NOT_IMPL;
        RNS_UNUSED(this);
        return ;
      }),
    Method(
      "clearMemoryCache",
      [this] (dynamic args, Callback cb, Callback cbError) {
        RNS_LOG_NOT_IMPL;
        RNS_UNUSED(this);
        RSkImageCacheManager::getImageCacheManagerInstance()->clearMemory();
        return ;
      }),
    Method(
      "clearDiskCache",
      [this] (dynamic args, Callback cb, Callback cbError) {
        RNS_LOG_NOT_IMPL;
        RSkImageCacheManager::getImageCacheManagerInstance()->clearDisk();
        RNS_UNUSED(this);
        return ;
      }),
  };
}

#ifdef __cplusplus
extern "C" {
#endif
RNS_EXPORT_MODULE(FastImageView)
#ifdef __cplusplus
}
#endif

} // namespace xplat
} // namespace facebook
