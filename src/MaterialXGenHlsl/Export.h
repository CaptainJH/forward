//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <MaterialXCore/Library.h>

/// @file
/// Macros for declaring imported and exported symbols.

#if defined(MATERIALX_GENHLSL_EXPORTS)
    #define MX_GENHLSL_API MATERIALX_SYMBOL_EXPORT
    #define MX_GENHLSL_EXTERN_TEMPLATE(...) MATERIALX_EXPORT_EXTERN_TEMPLATE(__VA_ARGS__)
#else
    #define MX_GENGLSL_API MATERIALX_SYMBOL_IMPORT
    #define MX_GENGLSL_EXTERN_TEMPLATE(...) MATERIALX_IMPORT_EXTERN_TEMPLATE(__VA_ARGS__)
#endif
