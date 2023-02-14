#pragma once


#include <MaterialXGenHlsl/Export.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

class MX_GENHLSL_API HlslShaderGenerator : public HwShaderGenerator
{
public:
    HlslShaderGenerator();

    static ShaderGeneratorPtr create() { return std::make_shared<HlslShaderGenerator>(); }

    /// Generate a shader starting from the given element, translating
    /// the element and all dependencies upstream into shader code.
    ShaderPtr generate(const string& name, ElementPtr element, GenContext& context) const override;

    /// Return a unique identifier for the target this generator is for
    const string& getTarget() const override { return TARGET; }

    /// Return the version string for the GLSL version this generator is for
    virtual const string& getVersion() const { return VERSION; }

    /// Determine the prefix of vertex data variables. 
    virtual string getVertexDataPrefix(const VariableBlock& vertexData) const;

    void emitVariableDeclaration(const ShaderPort* variable, const string& qualifier, GenContext& context, ShaderStage& stage,
                                         bool assignValue = true) const override;

public:
    /// Unique identifier for this generator target
    static const string TARGET;

    /// Version string for the generator target
    static const string VERSION;

private:
    void emitVertexStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;
    void emitPixelStage(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;

    void emitDirectives(GenContext& context, ShaderStage& stage) const;
    void emitConstants(GenContext& context, ShaderStage& stage) const;
    void emitUniforms(GenContext& context, ShaderStage& stage) const;
    void emitLightData(GenContext& context, ShaderStage& stage) const;
    void emitInputs(GenContext& context, ShaderStage& stage) const;
    void emitOutputs(GenContext& context, ShaderStage& stage) const;

    HwResourceBindingContextPtr getResourceBindingContext(GenContext& context) const;

    /// Logic to indicate whether code to support direct lighting should be emitted.
    /// By default if the graph is classified as a shader, or BSDF node then lighting is assumed to be required.
    bool requiresLighting(const ShaderGraph& graph) const;

    /// Emit specular environment lookup code
    void emitSpecularEnvironment(GenContext& context, ShaderStage& stage) const;

    /// Emit transmission rendering code
    void emitTransmissionRender(GenContext& context, ShaderStage& stage) const;

    /// Emit function definitions for lighting code
    void emitLightFunctionDefinitions(const ShaderGraph& graph, GenContext& context, ShaderStage& stage) const;

    static void toFloat4(const TypeDesc* type, string& variable);

    /// Nodes used internally for light sampling.
    vector<ShaderNodePtr> _lightSamplingNodes;
};

/// Base class for common HLSL node implementations
class MX_GENHLSL_API HlslImplementation : public ShaderNodeImpl
{
public:
    const string& getTarget() const override;

    bool isEditable(const ShaderInput& input) const override;

protected:
    HlslImplementation() { }

    // Integer identifiers for coordinate spaces.
    // The order must match the order given for
    // the space enum string in stdlib.
    enum Space
    {
        MODEL_SPACE = 0,
        OBJECT_SPACE = 1,
        WORLD_SPACE = 2
    };

    /// Internal string constants
    static const string SPACE;
    static const string TO_SPACE;
    static const string FROM_SPACE;
    static const string WORLD;
    static const string OBJECT;
    static const string MODEL;
    static const string INDEX;
    static const string GEOMPROP;
};

MATERIALX_NAMESPACE_END