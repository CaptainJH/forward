//
// TM & (c) 2022 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#pragma once

#include <MaterialXGenHlsl/Export.h>
#include <MaterialXGenHlsl/HlslShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// Unlit surface node implementation for HLSL
class MX_GENHLSL_API UnlitSurfaceNodeHlsl : public HlslImplementation
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END
