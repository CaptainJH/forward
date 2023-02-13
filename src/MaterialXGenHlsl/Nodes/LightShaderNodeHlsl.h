//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#pragma once

#include <MaterialXGenHlsl/HlslShaderGenerator.h>
#include <MaterialXGenShader/Nodes/SourceCodeNode.h>

MATERIALX_NAMESPACE_BEGIN

/// LightShader node implementation for HLSL
/// Used for all light shaders implemented in source code.
class MX_GENHLSL_API LightShaderNodeHlsl : public SourceCodeNode
{
public:
    LightShaderNodeHlsl();

    static ShaderNodeImplPtr create();

    const string& getTarget() const override;

    void initialize(const InterfaceElement& element, GenContext& context) override;

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

protected:
    VariableBlock _lightUniforms;
};

MATERIALX_NAMESPACE_END
