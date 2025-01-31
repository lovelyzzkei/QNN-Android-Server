//=============================================================================
//
//  Copyright (c) 2024 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//=============================================================================

/** @file
 *  @brief QNN LPAI Graph Preparation component.
 *
 */

#ifndef QNN_LPAI_GRAPH_PREPARE_H
#define QNN_LPAI_GRAPH_PREPARE_H

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#include "QnnLpaiGraphInternal.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//   Implementation Definition
//------------------------------------------------------------------------------

typedef enum {
  QNN_LPAI_GRAPH_SET_CFG_PREPARE = QNN_LPAI_GRAPH_SET_CFG_PREPARE_DEFAULT
} QnnLpaiGraph_ConfigPrepareOption_t;

/**
 * @brief Structure describing the set of configurations supported by the graph config prepare.
 *        Objects of this type are to be referenced through QnnGraph_CustomConfig_t.
 *
 */

typedef struct {
  uint32_t enableLayerFusion;
  uint32_t enableBatchnormFold;
  uint32_t enableChannelAlign;
  uint32_t enablePadSplit;
  uint32_t excludeIo;
  uint32_t enablePerLayer;
} QnnLpaiGraph_CustomConfigPrepare_t;

// clang-format off
/// QnnLpaiGraph_CustomConfigPrepare_t initializer macro
#define QNN_LPAI_GRAPH_CUSTOM_CONFIG_PREPARE_INIT               \
  {                                                             \
      1u,                               /*enableLayerFusion*/   \
      1u,                               /*enableBatchnormFold*/ \
      0u,                               /*enableChannelAlign*/  \
      0u,                               /*enablePadSplit*/      \
      0u,                               /*excludeIo*/           \
      0u                                /*enablePerLayer*/      \
  }

// clang-format on

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
