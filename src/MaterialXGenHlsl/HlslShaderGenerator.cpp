#include "HlslShaderGenerator.h"

#include <MaterialXGenHlsl/HlslSyntax.h>

MATERIALX_NAMESPACE_BEGIN

const string HlslShaderGenerator::TARGET = "genhlsl";
const string HlslShaderGenerator::VERSION = "100";

//
// GlslShaderGenerator methods
//

HlslShaderGenerator::HlslShaderGenerator() :
    HwShaderGenerator(HlslSyntax::create())
{
    //
    // Register all custom node implementation classes
    //

}

MATERIALX_NAMESPACE_END