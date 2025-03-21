//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#pragma once

#include <MaterialXGenHlsl/Export.h>

#include <MaterialXGenShader/Nodes/ConvolutionNode.h>

MATERIALX_NAMESPACE_BEGIN

/// HeightToNormal node implementation for HLSL
class MX_GENHLSL_API HeightToNormalNodeHlsl : public ConvolutionNode
{
  public:
    static ShaderNodeImplPtr create();

    void emitFunctionDefinition(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;
    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    const string& getTarget() const override;

  protected:
    /// Return if given type is an acceptible input
    bool acceptsInputType(const TypeDesc* type) const override;

    /// Compute offset strings for sampling
    void computeSampleOffsetStrings(const string& sampleSizeName, const string& offsetTypeString, 
                                    unsigned int filterWidth, StringVec& offsetStrings) const override;
};

MATERIALX_NAMESPACE_END
