//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#pragma once

#include <MaterialXGenHlsl/Export.h>

#include <MaterialXGenShader/Nodes/CompoundNode.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

class HlslShaderGenerator;

/// LightCompound node implementation for HLSL
class MX_GENHLSL_API LightCompoundNodeHlsl : public CompoundNode
{
public:
    LightCompoundNodeHlsl();

    static ShaderNodeImplPtr create();

    const string& getTarget() const override;

    void initialize(const InterfaceElement& element, GenContext& context) override;

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

protected:
    void emitFunctionDefinition(ClosureContext* cct, GenContext& context, ShaderStage& stage) const;

    VariableBlock _lightUniforms;
};

MATERIALX_NAMESPACE_END
