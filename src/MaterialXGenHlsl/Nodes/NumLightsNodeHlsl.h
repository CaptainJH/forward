//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#pragma once

#include <MaterialXGenHlsl/HlslShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// Utility node for getting number of active lights for HLSL.
class MX_GENHLSL_API NumLightsNodeHlsl : public HlslImplementation
{
public:
    NumLightsNodeHlsl();

    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END
