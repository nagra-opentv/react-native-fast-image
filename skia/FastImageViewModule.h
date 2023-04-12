/*
 * Copyright (C) 1994-2022 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cxxreact/CxxModule.h>

#include "ReactSkia/utils/RnsUtils.h"

using namespace std;
namespace facebook {
namespace xplat {

class FastImageViewModule : public module::CxxModule {
 public:
  FastImageViewModule();
  std::map<std::string, folly::dynamic> getConstants() override;
  std::vector<Method> getMethods() override;
  std::string getName() override;
};

} // namespace xplat
} // namespace facebook
