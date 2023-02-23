//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenHlsl/Nodes/TexCoordNodeHlsl.h>

#include <MaterialXGenShader/Shader.h>

MATERIALX_NAMESPACE_BEGIN

ShaderNodeImplPtr TexCoordNodeHlsl::create()
{
    return std::make_shared<TexCoordNodeHlsl>();
}

void TexCoordNodeHlsl::createVariables(const ShaderNode& node, GenContext&, Shader& shader) const
{
    const ShaderOutput* output = node.getOutput();
    const ShaderInput* indexInput = node.getInput(INDEX);
    const string index = indexInput ? indexInput->getValue()->getValueString() : "0";

    ShaderStage& vs = shader.getStage(Stage::VERTEX);
    ShaderStage& ps = shader.getStage(Stage::PIXEL);

    addStageInput(HW::VERTEX_INPUTS, output->getType(), HW::T_IN_TEXCOORD + "_" + index, vs);
    addStageConnector(HW::VERTEX_DATA, output->getType(), HW::T_TEXCOORD + "_" + index, vs, ps);
}

void TexCoordNodeHlsl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    const HlslShaderGenerator& shadergen = static_cast<const HlslShaderGenerator&>(context.getShaderGenerator());

    const ShaderInput* indexInput = node.getInput(INDEX);
    const string index = indexInput ? indexInput->getValue()->getValueString() : "0";
    const string variable = HW::T_TEXCOORD + "_" + index;

    BEGIN_SHADER_STAGE(stage, Stage::VERTEX)
        const VariableBlock& vertexData_Input = stage.getInputBlock(HW::VERTEX_INPUTS);
        VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
        ShaderPort* texcoord = vertexData[variable];
        if (!texcoord->isEmitted())
        {
            shadergen.emitLine(prefix + texcoord->getVariable() + " = " + vertexData_Input.getInstance() + "." + HW::T_IN_TEXCOORD + "_" + index, stage);
            texcoord->setEmitted();
        }
    END_SHADER_STAGE(shader, Stage::VERTEX)

    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
        const string prefix = shadergen.getVertexDataPrefix(vertexData);
        ShaderPort* texcoord = vertexData[variable];
            shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = " + prefix + texcoord->getVariable(), stage);
        shadergen.emitLineEnd(stage);
    END_SHADER_STAGE(shader, Stage::PIXEL)
}

MATERIALX_NAMESPACE_END
