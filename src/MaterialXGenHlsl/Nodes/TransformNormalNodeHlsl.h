//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#pragma once

#include <MaterialXGenHlsl/Nodes/TransformVectorNodeHlsl.h>

MATERIALX_NAMESPACE_BEGIN

/// TransformNormal node implementation for HLSL
class MX_GENHLSL_API TransformNormalNodeHlsl : public TransformVectorNodeHlsl
{
public:
    static ShaderNodeImplPtr create();

protected:
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    const string& getMatrix(const string& fromSpace, const string& toSpace) const override;
};

MATERIALX_NAMESPACE_END
