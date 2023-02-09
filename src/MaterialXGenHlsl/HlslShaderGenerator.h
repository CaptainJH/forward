#pragma once


#include "MaterialXGenHlsl/Export.h"
#include <MaterialXGenShader/HwShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

class MX_GENHLSL_API HlslShaderGenerator : public HwShaderGenerator
{
public:
    HlslShaderGenerator();

public:
    /// Unique identifier for this generator target
    static const string TARGET;

    /// Version string for the generator target
    static const string VERSION;
};

MATERIALX_NAMESPACE_END