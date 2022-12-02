/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace facebook {
namespace react {

/*
 * State for <ReactNativeFastImage> component.
 */
class ReactNativeFastImageState final {
 public:
  ReactNativeFastImageState() {}
  /*
   * Returns stored ImageSource object.
   */
  FastImageSource getImageSource() const;

  /*
   * Exposes for reading stored `ImageRequest` object.
   * `ImageRequest` object cannot be copied or moved from `ImageLocalData`.
   */

  Float getBlurRadius() const;
  #ifdef ANDROID
  ReactNativeFastImageState(ReactNativeFastImageState const &previousState, folly::dynamic data)
      : blurRadius_{0} {};

  /*
   * Empty implementation for Android because it doesn't use this class.
   */
  folly::dynamic getDynamic() const {
    return {};
  };
#endif

 private:
  FastImageSource imageSource_;

};

} // namespace react
} // namespace facebook
