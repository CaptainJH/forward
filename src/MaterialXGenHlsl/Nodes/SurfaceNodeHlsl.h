//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#pragma once

#include <MaterialXGenHlsl/Export.h>
#include <MaterialXGenHlsl/HlslShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

/// Surface node implementation for HLSL
class MX_GENHLSL_API SurfaceNodeHlsl : public HlslImplementation
{
  public:
    SurfaceNodeHlsl();

    static ShaderNodeImplPtr create();

    void createVariables(const ShaderNode& node, GenContext& context, Shader& shader) const override;

    void emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const override;

    virtual void emitLightLoop(const ShaderNode& node, GenContext& context, ShaderStage& stage, const string& outColor) const;

  protected:
    /// Closure contexts for calling closure functions.
    mutable ClosureContext _callReflection;
    mutable ClosureContext _callTransmission;
    mutable ClosureContext _callIndirect;
    mutable ClosureContext _callEmission;
};

MATERIALX_NAMESPACE_END
