//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenHlsl/Nodes/SurfaceShaderNodeHlsl.h>
#include <MaterialXGenHlsl/HlslShaderGenerator.h>

#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr SurfaceShaderNodeHlsl::create()
{
    return std::make_shared<SurfaceShaderNodeHlsl>();
}

const string& SurfaceShaderNodeHlsl::getTarget() const
{
    return HlslShaderGenerator::TARGET;
}

void SurfaceShaderNodeHlsl::createVariables(const ShaderNode&, GenContext& context, Shader& shader) const
{
    // TODO: 
    // The surface shader needs position, view position and light sources. We should solve this by adding some 
    // dependency mechanism so this implementation can be set to depend on the PositionNodeHlsl,  
    // ViewDirectionNodeHlsl and LightNodeHlsl nodes instead? This is where the MaterialX attribute "internalgeomprops" 
    // is needed.
    //
    ShaderStage& vs = shader.getStage(Stage::VERTEX);
    ShaderStage& ps = shader.getStage(Stage::PIXEL);

    addStageInput(HW::VERTEX_INPUTS, Type::VECTOR3, HW::T_IN_POSITION, vs);
    addStageConnector(HW::VERTEX_DATA, Type::VECTOR3, HW::T_POSITION_WORLD, vs, ps);

    addStageUniform(HW::PRIVATE_UNIFORMS, Type::VECTOR3, HW::T_VIEW_POSITION, ps);

    const HlslShaderGenerator& shadergen = static_cast<const HlslShaderGenerator&>(context.getShaderGenerator());
    shadergen.addStageLightingUniforms(context, ps);
}

void SurfaceShaderNodeHlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const HlslShaderGenerator& shadergen = static_cast<const HlslShaderGenerator&>(context.getShaderGenerator());

    BEGIN_SHADER_STAGE(stage, Stage::VERTEX)
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
        ShaderPort* position = vertexData[HW::T_POSITION_WORLD];
        if (!position->isEmitted())
        {
            position->setEmitted();
            context.getShaderGenerator().emitLine(prefix + position->getVariable() + " = hPositionWorld.xyz", stage);
        }
    END_SHADER_STAGE(shader, Stage::VERTEX)

    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        SourceCodeNode::emitFunctionCall(node, context, stage);
    END_SHADER_STAGE(shader, Stage::PIXEL)
}

MATERIALX_NAMESPACE_END
