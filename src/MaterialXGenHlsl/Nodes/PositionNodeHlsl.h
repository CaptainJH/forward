//***************************************************************************************
// PositionNodeHlsl.h by Heqi Ju (C) 2023 All Rights Reserved.
//***************************************************************************************

#pragma once

#include <MaterialXGenHlsl/HlslShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// Position node implementation for GLSL
class MX_GENHLSL_API PositionNodeHlsl : public HlslImplementation
{
  public:
    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
};

MATERIALX_NAMESPACE_END
