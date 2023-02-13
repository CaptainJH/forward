//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#pragma once

#include <MaterialXGenHlsl/HlslShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// TransformVector node implementation for HLSL
class MX_GENHLSL_API TransformVectorNodeHlsl : public HlslImplementation
{
public:
    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

protected:
    virtual const string& getMatrix(const string& fromSpace, const string& toSpace) const;
    virtual string getHomogeneousCoordinate(const ShaderInput* in, GenContext& context) const;
};

MATERIALX_NAMESPACE_END
