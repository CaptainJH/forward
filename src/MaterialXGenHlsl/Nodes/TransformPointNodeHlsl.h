//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#pragma once

#include <MaterialXGenHlsl/Nodes/TransformVectorNodeHlsl.h>

MATERIALX_NAMESPACE_BEGIN

/// TransformPoint node implementation for HLSL
class MX_GENHLSL_API TransformPointNodeHlsl : public TransformVectorNodeHlsl
{
public:
    static ShaderNodeImplPtr create();

protected:
    virtual string getHomogeneousCoordinate(const ShaderInput* in, GenContext& context) const override;
};

MATERIALX_NAMESPACE_END
