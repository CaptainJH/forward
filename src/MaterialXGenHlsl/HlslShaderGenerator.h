#pragma once


#include <MaterialXGenHlsl/Export.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

class MX_GENHLSL_API HlslShaderGenerator : public HwShaderGenerator
{
public:
    HlslShaderGenerator();

    /// Determine the prefix of vertex data variables. 
    virtual string getVertexDataPrefix(const VariableBlock& vertexData) const;

public:
    /// Unique identifier for this generator target
    static const string TARGET;

    /// Version string for the generator target
    static const string VERSION;
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