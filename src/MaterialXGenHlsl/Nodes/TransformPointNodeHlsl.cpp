//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenHlsl/Nodes/TransformPointNodeHlsl.h>

#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr TransformPointNodeHlsl::create()
{
    return std::make_shared<TransformPointNodeHlsl>();
}

string TransformPointNodeHlsl::getHomogeneousCoordinate(const ShaderInput* in, GenContext& context) const
{
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    return "float4(" + shadergen.getUpstreamResult(in, context) + ", 1.0)";
}

MATERIALX_NAMESPACE_END
