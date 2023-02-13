#include "HlslShaderGenerator.h"

#include <MaterialXGenHlsl/HlslSyntax.h>
#include <MaterialXGenHlsl/Nodes/PositionNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/NormalNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/TangentNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/BitangentNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/TexCoordNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/GeomColorNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/GeomPropValueNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/FrameNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/TimeNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/SurfaceNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/UnlitSurfaceNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/LightNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/LightCompoundNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/LightShaderNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/HeightToNormalNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/LightSamplerNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/NumLightsNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/TransformVectorNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/TransformPointNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/TransformNormalNodeHlsl.h>
#include <MaterialXGenHlsl/Nodes/BlurNodeHlsl.h>

#include <MaterialXGenShader/Nodes/MaterialNode.h>
#include <MaterialXGenShader/Nodes/SwizzleNode.h>
#include <MaterialXGenShader/Nodes/ConvertNode.h>
#include <MaterialXGenShader/Nodes/CombineNode.h>
#include <MaterialXGenShader/Nodes/SwitchNode.h>
#include <MaterialXGenShader/Nodes/IfNode.h>
#include <MaterialXGenShader/Nodes/HwImageNode.h>
#include <MaterialXGenShader/Nodes/ClosureSourceCodeNode.h>
#include <MaterialXGenShader/Nodes/ClosureCompoundNode.h>
#include <MaterialXGenShader/Nodes/ClosureLayerNode.h>
#include <MaterialXGenShader/Nodes/ClosureMixNode.h>
#include <MaterialXGenShader/Nodes/ClosureAddNode.h>
#include <MaterialXGenShader/Nodes/ClosureMultiplyNode.h>

#include <MaterialXGenHlsl/HlslResourceBindingContext.h>
#include <assert.h>

MATERIALX_NAMESPACE_BEGIN

const string HlslShaderGenerator::TARGET = "genhlsl";
const string HlslShaderGenerator::VERSION = "100";

//
// HlslShaderGenerator methods
//

HlslShaderGenerator::HlslShaderGenerator() :
    HwShaderGenerator(HlslSyntax::create())
{
    //
     // Register all custom node implementation classes
     //

    // <!-- <if*> -->
    static const string SEPARATOR = "_";
    static const string INT_SEPARATOR = "I_";
    static const string BOOL_SEPARATOR = "B_";
    static const StringVec IMPL_PREFIXES = { "IM_ifgreater_", "IM_ifgreatereq_", "IM_ifequal_" };
    static const vector<CreatorFunction<ShaderNodeImpl>> IMPL_CREATE_FUNCTIONS = { IfGreaterNode::create, IfGreaterEqNode::create, IfEqualNode::create };
    static const vector<bool> IMPL_HAS_INTVERSION = { true, true, true };
    static const vector<bool> IMPL_HAS_BOOLVERSION = { false, false, true };
    static const StringVec IMPL_TYPES = { "float", "color3", "color4", "vector2", "vector3", "vector4" };
    for (size_t i = 0; i < IMPL_PREFIXES.size(); i++)
    {
        const string& implPrefix = IMPL_PREFIXES[i];
        for (const string& implType : IMPL_TYPES)
        {
            const string implRoot = implPrefix + implType;
            registerImplementation(implRoot + SEPARATOR + HlslShaderGenerator::TARGET, IMPL_CREATE_FUNCTIONS[i]);
            if (IMPL_HAS_INTVERSION[i])
            {
                registerImplementation(implRoot + INT_SEPARATOR + HlslShaderGenerator::TARGET, IMPL_CREATE_FUNCTIONS[i]);
            }
            if (IMPL_HAS_BOOLVERSION[i])
            {
                registerImplementation(implRoot + BOOL_SEPARATOR + HlslShaderGenerator::TARGET, IMPL_CREATE_FUNCTIONS[i]);
            }
        }
    }

     // <!-- <switch> -->
     // <!-- 'which' type : float -->
    registerImplementation("IM_switch_float_" + HlslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_color3_" + HlslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_color4_" + HlslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_vector2_" + HlslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_vector3_" + HlslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_vector4_" + HlslShaderGenerator::TARGET, SwitchNode::create);
    // <!-- 'which' type : integer -->
    registerImplementation("IM_switch_floatI_" + HlslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_color3I_" + HlslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_color4I_" + HlslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_vector2I_" + HlslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_vector3I_" + HlslShaderGenerator::TARGET, SwitchNode::create);
    registerImplementation("IM_switch_vector4I_" + HlslShaderGenerator::TARGET, SwitchNode::create);

    // <!-- <swizzle> -->
    // <!-- from type : float -->
    registerImplementation("IM_swizzle_float_color3_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_color4_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector2_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector3_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector4_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    // <!-- from type : color3 -->
    registerImplementation("IM_swizzle_color3_float_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color3_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color4_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector2_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector3_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector4_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    // <!-- from type : color4 -->
    registerImplementation("IM_swizzle_color4_float_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color3_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color4_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector2_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector3_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector4_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    // <!-- from type : vector2 -->
    registerImplementation("IM_swizzle_vector2_float_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color3_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color4_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector2_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector3_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector4_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    // <!-- from type : vector3 -->
    registerImplementation("IM_swizzle_vector3_float_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color3_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color4_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector2_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector3_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector4_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    // <!-- from type : vector4 -->
    registerImplementation("IM_swizzle_vector4_float_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color3_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color4_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector2_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector3_" + HlslShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector4_" + HlslShaderGenerator::TARGET, SwizzleNode::create);

    // <!-- <convert> -->
    registerImplementation("IM_convert_float_color3_" + HlslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_float_color4_" + HlslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_float_vector2_" + HlslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_float_vector3_" + HlslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_float_vector4_" + HlslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector2_vector3_" + HlslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector3_vector2_" + HlslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector3_color3_" + HlslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector3_vector4_" + HlslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector4_vector3_" + HlslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector4_color4_" + HlslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_color3_vector3_" + HlslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_color4_vector4_" + HlslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_color3_color4_" + HlslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_color4_color3_" + HlslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_boolean_float_" + HlslShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_integer_float_" + HlslShaderGenerator::TARGET, ConvertNode::create);

    // <!-- <combine> -->
    registerImplementation("IM_combine2_vector2_" + HlslShaderGenerator::TARGET, CombineNode::create);
    registerImplementation("IM_combine2_color4CF_" + HlslShaderGenerator::TARGET, CombineNode::create);
    registerImplementation("IM_combine2_vector4VF_" + HlslShaderGenerator::TARGET, CombineNode::create);
    registerImplementation("IM_combine2_vector4VV_" + HlslShaderGenerator::TARGET, CombineNode::create);
    registerImplementation("IM_combine3_color3_" + HlslShaderGenerator::TARGET, CombineNode::create);
    registerImplementation("IM_combine3_vector3_" + HlslShaderGenerator::TARGET, CombineNode::create);
    registerImplementation("IM_combine4_color4_" + HlslShaderGenerator::TARGET, CombineNode::create);
    registerImplementation("IM_combine4_vector4_" + HlslShaderGenerator::TARGET, CombineNode::create);

    // <!-- <position> -->
    registerImplementation("IM_position_vector3_" + HlslShaderGenerator::TARGET, PositionNodeHlsl::create);
    // <!-- <normal> -->
    registerImplementation("IM_normal_vector3_" + HlslShaderGenerator::TARGET, NormalNodeHlsl::create);
    // <!-- <tangent> -->
    registerImplementation("IM_tangent_vector3_" + HlslShaderGenerator::TARGET, TangentNodeHlsl::create);
    // <!-- <bitangent> -->
    registerImplementation("IM_bitangent_vector3_" + HlslShaderGenerator::TARGET, BitangentNodeHlsl::create);
    // <!-- <texcoord> -->
    registerImplementation("IM_texcoord_vector2_" + HlslShaderGenerator::TARGET, TexCoordNodeHlsl::create);
    registerImplementation("IM_texcoord_vector3_" + HlslShaderGenerator::TARGET, TexCoordNodeHlsl::create);
    // <!-- <geomcolor> -->
    registerImplementation("IM_geomcolor_float_" + HlslShaderGenerator::TARGET, GeomColorNodeHlsl::create);
    registerImplementation("IM_geomcolor_color3_" + HlslShaderGenerator::TARGET, GeomColorNodeHlsl::create);
    registerImplementation("IM_geomcolor_color4_" + HlslShaderGenerator::TARGET, GeomColorNodeHlsl::create);
    // <!-- <geompropvalue> -->
    registerImplementation("IM_geompropvalue_integer_" + HlslShaderGenerator::TARGET, GeomPropValueNodeHlsl::create);
    registerImplementation("IM_geompropvalue_boolean_" + HlslShaderGenerator::TARGET, GeomPropValueNodeHlslAsUniform::create);
    registerImplementation("IM_geompropvalue_string_" + HlslShaderGenerator::TARGET, GeomPropValueNodeHlslAsUniform::create);
    registerImplementation("IM_geompropvalue_float_" + HlslShaderGenerator::TARGET, GeomPropValueNodeHlsl::create);
    registerImplementation("IM_geompropvalue_color3_" + HlslShaderGenerator::TARGET, GeomPropValueNodeHlsl::create);
    registerImplementation("IM_geompropvalue_color4_" + HlslShaderGenerator::TARGET, GeomPropValueNodeHlsl::create);
    registerImplementation("IM_geompropvalue_vector2_" + HlslShaderGenerator::TARGET, GeomPropValueNodeHlsl::create);
    registerImplementation("IM_geompropvalue_vector3_" + HlslShaderGenerator::TARGET, GeomPropValueNodeHlsl::create);
    registerImplementation("IM_geompropvalue_vector4_" + HlslShaderGenerator::TARGET, GeomPropValueNodeHlsl::create);

    // <!-- <frame> -->
    registerImplementation("IM_frame_float_" + HlslShaderGenerator::TARGET, FrameNodeHlsl::create);
    // <!-- <time> -->
    registerImplementation("IM_time_float_" + HlslShaderGenerator::TARGET, TimeNodeHlsl::create);

    // <!-- <surface> -->
    registerImplementation("IM_surface_" + HlslShaderGenerator::TARGET, SurfaceNodeHlsl::create);
    registerImplementation("IM_surface_unlit_" + HlslShaderGenerator::TARGET, UnlitSurfaceNodeHlsl::create);

    // <!-- <light> -->
    registerImplementation("IM_light_" + HlslShaderGenerator::TARGET, LightNodeHlsl::create);

    // <!-- <point_light> -->
    registerImplementation("IM_point_light_" + HlslShaderGenerator::TARGET, LightShaderNodeHlsl::create);
    // <!-- <directional_light> -->
    registerImplementation("IM_directional_light_" + HlslShaderGenerator::TARGET, LightShaderNodeHlsl::create);
    // <!-- <spot_light> -->
    registerImplementation("IM_spot_light_" + HlslShaderGenerator::TARGET, LightShaderNodeHlsl::create);

    // <!-- <heighttonormal> -->
    registerImplementation("IM_heighttonormal_vector3_" + HlslShaderGenerator::TARGET, HeightToNormalNodeHlsl::create);

    // <!-- <blur> -->
    registerImplementation("IM_blur_float_" + HlslShaderGenerator::TARGET, BlurNodeHlsl::create);
    registerImplementation("IM_blur_color3_" + HlslShaderGenerator::TARGET, BlurNodeHlsl::create);
    registerImplementation("IM_blur_color4_" + HlslShaderGenerator::TARGET, BlurNodeHlsl::create);
    registerImplementation("IM_blur_vector2_" + HlslShaderGenerator::TARGET, BlurNodeHlsl::create);
    registerImplementation("IM_blur_vector3_" + HlslShaderGenerator::TARGET, BlurNodeHlsl::create);
    registerImplementation("IM_blur_vector4_" + HlslShaderGenerator::TARGET, BlurNodeHlsl::create);

    // <!-- <ND_transformpoint> ->
    registerImplementation("IM_transformpoint_vector3_" + HlslShaderGenerator::TARGET, TransformPointNodeHlsl::create);

    // <!-- <ND_transformvector> ->
    registerImplementation("IM_transformvector_vector3_" + HlslShaderGenerator::TARGET, TransformVectorNodeHlsl::create);

    // <!-- <ND_transformnormal> ->
    registerImplementation("IM_transformnormal_vector3_" + HlslShaderGenerator::TARGET, TransformNormalNodeHlsl::create);

    // <!-- <image> -->
    registerImplementation("IM_image_float_" + HlslShaderGenerator::TARGET, HwImageNode::create);
    registerImplementation("IM_image_color3_" + HlslShaderGenerator::TARGET, HwImageNode::create);
    registerImplementation("IM_image_color4_" + HlslShaderGenerator::TARGET, HwImageNode::create);
    registerImplementation("IM_image_vector2_" + HlslShaderGenerator::TARGET, HwImageNode::create);
    registerImplementation("IM_image_vector3_" + HlslShaderGenerator::TARGET, HwImageNode::create);
    registerImplementation("IM_image_vector4_" + HlslShaderGenerator::TARGET, HwImageNode::create);

    // <!-- <layer> -->
    registerImplementation("IM_layer_bsdf_" + HlslShaderGenerator::TARGET, ClosureLayerNode::create);
    registerImplementation("IM_layer_vdf_" + HlslShaderGenerator::TARGET, ClosureLayerNode::create);
    // <!-- <mix> -->
    registerImplementation("IM_mix_bsdf_" + HlslShaderGenerator::TARGET, ClosureMixNode::create);
    registerImplementation("IM_mix_edf_" + HlslShaderGenerator::TARGET, ClosureMixNode::create);
    // <!-- <add> -->
    registerImplementation("IM_add_bsdf_" + HlslShaderGenerator::TARGET, ClosureAddNode::create);
    registerImplementation("IM_add_edf_" + HlslShaderGenerator::TARGET, ClosureAddNode::create);
    // <!-- <multiply> -->
    registerImplementation("IM_multiply_bsdfC_" + HlslShaderGenerator::TARGET, ClosureMultiplyNode::create);
    registerImplementation("IM_multiply_bsdfF_" + HlslShaderGenerator::TARGET, ClosureMultiplyNode::create);
    registerImplementation("IM_multiply_edfC_" + HlslShaderGenerator::TARGET, ClosureMultiplyNode::create);
    registerImplementation("IM_multiply_edfF_" + HlslShaderGenerator::TARGET, ClosureMultiplyNode::create);

    // <!-- <thin_film> -->
    registerImplementation("IM_thin_film_bsdf_" + HlslShaderGenerator::TARGET, NopNode::create);

    // <!-- <surfacematerial> -->
    registerImplementation("IM_surfacematerial_" + HlslShaderGenerator::TARGET, MaterialNode::create);

    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "numActiveLightSources", NumLightsNodeHlsl::create()));
    _lightSamplingNodes.push_back(ShaderNode::create(nullptr, "sampleLightSource", LightSamplerNodeHlsl::create()));

}

ShaderPtr HlslShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    // Set binding context to handle resource binding layouts
    HlslResourceBindingContextPtr hlslresourceBinding(HlslResourceBindingContext::create());
    context.pushUserData(HW::USER_DATA_BINDING_CONTEXT, hlslresourceBinding);

    ShaderPtr shader = createShader(name, element, context);

    // Turn on fixed float formatting to make sure float values are
    // emitted with a decimal point and not as integers, and to avoid
    // any scientific notation which isn't supported by all OpenGL targets.
    ScopedFloatFormatting fmt(Value::FloatFormatFixed);

    // Make sure we initialize/reset the binding context before generation.
    HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);
    if (resourceBindingCtx)
    {
        resourceBindingCtx->initialize();
    }

    // Emit code for vertex shader stage
    ShaderStage& vs = shader->getStage(Stage::VERTEX);
    emitVertexStage(shader->getGraph(), context, vs);
    replaceTokens(_tokenSubstitutions, vs);

    //// Emit code for pixel shader stage
    //ShaderStage& ps = shader->getStage(Stage::PIXEL);
    //emitPixelStage(shader->getGraph(), context, ps);
    //replaceTokens(_tokenSubstitutions, ps);

    context.popUserData(HW::USER_DATA_BINDING_CONTEXT);

    return shader;
}

void HlslShaderGenerator::emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const
{
    HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);

    emitDirectives(context, stage);
    if (resourceBindingCtx)
    {
        resourceBindingCtx->emitDirectives(context, stage);
    }
    emitLineBreak(stage);

    // Add all constants
    emitConstants(context, stage);

    // Add all uniforms
    emitUniforms(context, stage);

    // Add vertex inputs
    emitInputs(context, stage);

    // Add vertex data outputs block
    emitOutputs(context, stage);

    emitFunctionDefinitions(graph, context, stage);

    const VariableBlock& vertexData_Input = stage.getInputBlock(HW::VERTEX_INPUTS);
    const VariableBlock& vertexData_Output = stage.getOutputBlock(HW::VERTEX_DATA);
    // Add main function
    setFunctionName("main", stage);
    emitLine(vertexData_Output.getName() + " main( " + _syntax->getInputQualifier() + " " + vertexData_Input.getName() + " " + vertexData_Input.getInstance() + " )", stage, false);
    emitFunctionBodyBegin(graph, context, stage);
    emitLine(vertexData_Output.getName() + " " + vertexData_Output.getInstance() + " = (" + vertexData_Output.getName() + ")0", stage, true);
    emitLine(std::string("float4 hPositionWorld = mul(") + "float4(" + HW::T_IN_POSITION + ", 1.0), " + HW::T_WORLD_MATRIX + ")", stage);
    emitLine(vertexData_Output.getInstance() + ".position = mul(hPositionWorld, " + HW::T_VIEW_PROJECTION_MATRIX + ")", stage);

    // For vertex stage just emit all function calls in order
    // and ignore conditional scope.
    for (const ShaderNode* node : graph.getNodes())
    {
        emitFunctionCall(*node, context, stage, false);
    }

    emitFunctionBodyEnd(graph, context, stage);
}

void HlslShaderGenerator::emitDirectives(GenContext&, ShaderStage& stage) const
{
    emitLine("// Generated by JHQ using MaterialXGenHlsl ", stage, false);
}

void HlslShaderGenerator::emitConstants(GenContext& context, ShaderStage& stage) const
{
    const VariableBlock& constants = stage.getConstantBlock();
    if (!constants.empty())
    {
        emitVariableDeclarations(constants, _syntax->getConstantQualifier(), Syntax::SEMICOLON, context, stage);
        emitLineBreak(stage);
    }
}

void HlslShaderGenerator::emitUniforms(GenContext& context, ShaderStage& stage) const
{
    for (const auto& it : stage.getUniformBlocks())
    {
        const VariableBlock& uniforms = *it.second;

        // Skip light uniforms as they are handled separately
        if (!uniforms.empty() && uniforms.getName() != HW::LIGHT_DATA)
        {
            emitComment("Uniform block: " + uniforms.getName(), stage);
            HwResourceBindingContextPtr resourceBindingCtx = getResourceBindingContext(context);
            if (resourceBindingCtx)
            {
                resourceBindingCtx->emitResourceBindings(context, uniforms, stage);
            }
            else
            {
                emitVariableDeclarations(uniforms, _syntax->getUniformQualifier(), Syntax::SEMICOLON, context, stage);
                emitLineBreak(stage);
            }
        }
    }
}

void HlslShaderGenerator::emitInputs(GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::VERTEX)
    const VariableBlock& vertexInputs = stage.getInputBlock(HW::VERTEX_INPUTS);
    if (!vertexInputs.empty())
    {
        emitComment("Inputs block: " + vertexInputs.getName(), stage);
        emitLine("struct " + vertexInputs.getName(), stage, false);
        emitScopeBegin(stage);
        emitVariableDeclarations(vertexInputs, _syntax->getInputQualifier(), Syntax::SEMICOLON, context, stage, false);
        emitScopeEnd(stage, false, false);
        emitString(Syntax::SEMICOLON, stage);
        emitLineBreak(stage);
        emitLineBreak(stage);
    }
    END_SHADER_STAGE(stage, Stage::VERTEX)

    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
    const VariableBlock& vertexData = stage.getInputBlock(HW::VERTEX_DATA);
    if (!vertexData.empty())
    {
        emitLine("in " + vertexData.getName(), stage, false);
        emitScopeBegin(stage);
        emitVariableDeclarations(vertexData, EMPTY_STRING, Syntax::SEMICOLON, context, stage, false);
        emitScopeEnd(stage, false, false);
        emitString(" " + vertexData.getInstance() + Syntax::SEMICOLON, stage);
        emitLineBreak(stage);
        emitLineBreak(stage);
    }
    END_SHADER_STAGE(stage, Stage::PIXEL)
}

void HlslShaderGenerator::emitOutputs(GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::VERTEX)
    const VariableBlock& vertexData = stage.getOutputBlock(HW::VERTEX_DATA);
    if (!vertexData.empty())
    {
        emitLine("struct " + vertexData.getName(), stage, false);
        emitScopeBegin(stage);
        emitVariableDeclarations(vertexData, _syntax->getOutputQualifier(), Syntax::SEMICOLON, context, stage, false);
        emitLine("float3 position : SV_POSITION", stage, true);
        emitScopeEnd(stage, false, false);
        emitString(Syntax::SEMICOLON, stage);
        emitLineBreak(stage);
        emitLineBreak(stage);
    }
    END_SHADER_STAGE(stage, Stage::VERTEX)

    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
    emitComment("Pixel shader outputs", stage);
    const VariableBlock& outputs = stage.getOutputBlock(HW::PIXEL_OUTPUTS);
    emitVariableDeclarations(outputs, _syntax->getOutputQualifier(), Syntax::SEMICOLON, context, stage, false);
    emitLineBreak(stage);
    END_SHADER_STAGE(stage, Stage::PIXEL)
}

HwResourceBindingContextPtr HlslShaderGenerator::getResourceBindingContext(GenContext& context) const
{
    return context.getUserData<HwResourceBindingContext>(HW::USER_DATA_BINDING_CONTEXT);
}

string HlslShaderGenerator::getVertexDataPrefix(const VariableBlock& vertexData) const
{
    return vertexData.getInstance() + ".";
}

void HlslShaderGenerator::emitVariableDeclaration(const ShaderPort* variable, const string& qualifier,
                                              GenContext& ctx, ShaderStage& stage, bool assignValue) const
{
    if (qualifier == _syntax->getInputQualifier())
    {
        string str = _syntax->getTypeName(variable->getType());

        str += " " + variable->getVariable();
        if (variable->getName().ends_with("inPosition"))
            str += " : POSITION";
        else if (variable->getName().ends_with("inNormal"))
            str += " : NORMAL";
        else if (variable->getName().ends_with("inTangent"))
            str += " : TANGENT";
        else
            assert(false && "Shouldn't come to this place");

        stage.addString(str);
    }
    else if (qualifier == _syntax->getOutputQualifier())
    {
        string str = _syntax->getTypeName(variable->getType());

        str += " " + variable->getVariable();
        if (variable->getName().ends_with("positionWorld"))
            str += " : POSITION";
        else if (variable->getName().ends_with("normalWorld"))
            str += " : NORMAL";
        else if (variable->getName().ends_with("tangentWorld"))
            str += " : TANGENT";
        else
            assert(false && "Shouldn't come to this place");

        stage.addString(str);
    }
    else
        ShaderGenerator::emitVariableDeclaration(variable, qualifier, ctx, stage, assignValue);
}


const string HlslImplementation::SPACE = "space";
const string HlslImplementation::TO_SPACE = "tospace";
const string HlslImplementation::FROM_SPACE = "fromspace";
const string HlslImplementation::WORLD = "world";
const string HlslImplementation::OBJECT = "object";
const string HlslImplementation::MODEL = "model";
const string HlslImplementation::INDEX = "index";
const string HlslImplementation::GEOMPROP = "geomprop";

namespace
{

    // List name of inputs that are not to be editable and
    // published as shader uniforms in GLSL.
    const std::set<string> IMMUTABLE_INPUTS =
    {
        // Geometric node inputs are immutable since a shader needs regeneration if they change.
        "index", "space", "attrname"
    };

} // anonymous namespace

const string& HlslImplementation::getTarget() const
{
    return HlslShaderGenerator::TARGET;
}

bool HlslImplementation::isEditable(const ShaderInput& input) const
{
    return IMMUTABLE_INPUTS.count(input.getName()) == 0;
}

MATERIALX_NAMESPACE_END