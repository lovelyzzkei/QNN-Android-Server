//==============================================================================
//
//  Copyright (c) 2019-2022 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#pragma once

#include <vector>
#include <string>
#include "QnnTypeMacros.hpp"

namespace qnn {
namespace tools {
namespace dynamicloadutil {
enum class StatusCode {
  SUCCESS,
  FAILURE,
  FAIL_LOAD_BACKEND,
  FAIL_LOAD_MODEL,
  FAIL_SYM_FUNCTION,
  FAIL_GET_INTERFACE_PROVIDERS,
  FAIL_LOAD_SYSTEM_LIB,
};

StatusCode getQnnFunctionPointers(std::string backendPath,
                                  std::string modelPath,
                                  QnnFunctionPointers* qnnFunctionPointers,
                                  void** backendHandle,
                                  bool loadModelLib,
                                  void** modelHandleRtn);

StatusCode getQnnMultiFunctionPointers(std::string backendPath,
                                  const std::vector<std::string>& modelPaths,
                                  QnnFunctionPointers* qnnFunctionPointers,
                                  void** backendHandle,
                                  bool loadModelLib,
                                  std::vector<void*>& modelHandlesRtn,          // one handle per loaded model
                                  std::vector<QnnModelFunctionPointers>& modelFuncsVec);  // one set of function pointers per loaded model

StatusCode loadModelLibrary(const std::string& modelPath,
                            QnnModelFunctionPointers& modelFuncs,
                            void** modelHandleRtn);

StatusCode getQnnSystemFunctionPointers(std::string systemLibraryPath,
                                        QnnFunctionPointers* qnnFunctionPointers);
}  // namespace dynamicloadutil
}  // namespace tools
}  // namespace qnn
