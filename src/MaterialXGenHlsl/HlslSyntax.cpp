//***************************************************************************************
// HlslSyntax.cpp by Heqi Ju (C) 2023 All Rights Reserved.
//***************************************************************************************

#include <MaterialXGenHlsl/HlslSyntax.h>

#include <MaterialXGenShader/ShaderGenerator.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// Since HLSL doesn't support strings we use integers instead.
// TODO: Support options strings by converting to a corresponding enum integer
class HlslStringTypeSyntax : public StringTypeSyntax
{
  public:
    HlslStringTypeSyntax() :
        StringTypeSyntax("int", "0", "0") { }

    string getValue(const Value& /*value*/, bool /*uniform*/) const override
    {
        return "0";
    }
};

class HlslArrayTypeSyntax : public ScalarTypeSyntax
{
  public:
    HlslArrayTypeSyntax(const string& name) :
        ScalarTypeSyntax(name, EMPTY_STRING, EMPTY_STRING, EMPTY_STRING)
    {
    }

    string getValue(const Value& value, bool /*uniform*/) const override
    {
        size_t arraySize = getSize(value);
        if (arraySize > 0)
        {
            return _name + "[" + std::to_string(arraySize) + "](" + value.getValueString() + ")";
        }
        return EMPTY_STRING;
    }

    string getValue(const StringVec& values, bool /*uniform*/) const override
    {
        if (values.empty())
        {
            throw ExceptionShaderGenError("No values given to construct an array value");
        }

        string result = _name + "[" + std::to_string(values.size()) + "](" + values[0];
        for (size_t i = 1; i < values.size(); ++i)
        {
            result += ", " + values[i];
        }
        result += ")";

        return result;
    }

  protected:
    virtual size_t getSize(const Value& value) const = 0;
};

class HlslFloatArrayTypeSyntax : public HlslArrayTypeSyntax
{
  public:
    explicit HlslFloatArrayTypeSyntax(const string& name) :
        HlslArrayTypeSyntax(name)
    {
    }

  protected:
    size_t getSize(const Value& value) const override
    {
        vector<float> valueArray = value.asA<vector<float>>();
        return valueArray.size();
    }
};

class GlslIntegerArrayTypeSyntax : public HlslArrayTypeSyntax
{
  public:
    explicit GlslIntegerArrayTypeSyntax(const string& name) :
        HlslArrayTypeSyntax(name)
    {
    }

  protected:
    size_t getSize(const Value& value) const override
    {
        vector<int> valueArray = value.asA<vector<int>>();
        return valueArray.size();
    }
};

} // anonymous namespace

const string HlslSyntax::INPUT_QUALIFIER = "in";
const string HlslSyntax::OUTPUT_QUALIFIER = "out";
const string HlslSyntax::UNIFORM_QUALIFIER = "cbuffer";
const string HlslSyntax::CONSTANT_QUALIFIER = "const";
const string HlslSyntax::FLAT_QUALIFIER = "flat";
const string HlslSyntax::SOURCE_FILE_EXTENSION = ".hlsl";
const StringVec HlslSyntax::VEC2_MEMBERS = { ".x", ".y" };
const StringVec HlslSyntax::VEC3_MEMBERS = { ".x", ".y", ".z" };
const StringVec HlslSyntax::VEC4_MEMBERS = { ".x", ".y", ".z", ".w" };

//
// GlslSyntax methods
//

HlslSyntax::HlslSyntax()
{
    // Add in all reserved words and keywords in GLSL
    registerReservedWords({ 
        // keywords: https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-appendix-keywords
        "AppendStructuredBuffer", "asm", "asm_fragment",
        "BlendState", "bool", "break", "Buffer", "ByteAddressBuffer",
        "case", "cbuffer", "centroid", "class", "column_major", "compile", "compile_fragment", "CompileShader", "const", "continue", "ComputeShader", "ConsumeStructuredBuffer",
        "default", "DepthStencilState", "DepthStencilView", "discard", "do", "double", "DomainShader", "dword",
        "else", "export", "extern",
        "false", "float", "for", "fxgroup",
        "GeometryShader", "groupshared",
        "half", "Hullshader",
        "if", "in", "inline", "inout", "InputPatch", "int", "interface",
        "line", "lineadj", "linear", "LineStream",
        "matrix", "min16float", "min10float", "min16int", "min12int", "min16uint",
        "namespace", "nointerpolation", "noperspective", "NULL",
        "out", "OutputPatch",
        "packoffset", "pass", "pixelfragment", "PixelShader", "point", "PointStream", "precise",
        "RasterizerState", "RenderTargetView", "return", "register", "row_major", "RWBuffer", "RWByteAddressBuffer", "RWStructuredBuffer", "RWTexture1D", "RWTexture1DArray", "RWTexture2D", "RWTexture2DArray", "RWTexture3D",
        "sample", "sampler", "SamplerState", "SamplerComparisonState", "shared", "snorm", "stateblock", "stateblock_state", "static", "string", "struct", "switch", "StructuredBuffer",
        "tbuffer", "technique", "technique10", "technique11", "texture", "Texture1D", "Texture1DArray", "Texture2D", "Texture2DArray", "Texture2DMS", "Texture2DMSArray", "Texture3D", "TextureCube", "TextureCubeArray", "true", "typedef", "triangle", "triangleadj", "TriangleStream",
        "uint", "uniform", "unorm", "unsigned",
        "vector", "vertexfragment", "VertexShader", "void", "volatile",
        "while",

        // Reserved Words: https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-appendix-reserved-words
        "auto", "case", "catch", "char", "class", "const_cast", "default", "delete", "dynamic_cast", "enum", "explicit", "friend", "goto", "long", "mutable", "new", "operator", "private", "protected", "public", "reinterpret_cast", "short", "signed", "sizeof", "static_cast", "template", "this", "throw", "try", "typename", "union", "unsigned", "using", "virtual",
        });

    // Register restricted tokens in HLSL
    StringMap tokens;
    tokens["__"] = "_";
    registerInvalidTokens(tokens);

    //
    // Register syntax handlers for each data type.
    //

    registerTypeSyntax(
        Type::FLOAT,
        std::make_shared<ScalarTypeSyntax>(
            "float",
            "0.0",
            "0.0"));

    registerTypeSyntax(
        Type::FLOATARRAY,
        std::make_shared<HlslFloatArrayTypeSyntax>(
            "float"));

    registerTypeSyntax(
        Type::INTEGER,
        std::make_shared<ScalarTypeSyntax>(
            "int",
            "0",
            "0"));

    registerTypeSyntax(
        Type::INTEGERARRAY,
        std::make_shared<GlslIntegerArrayTypeSyntax>(
            "int"));

    registerTypeSyntax(
        Type::BOOLEAN,
        std::make_shared<ScalarTypeSyntax>(
            "bool",
            "false",
            "false"));

    registerTypeSyntax(
        Type::COLOR3,
        std::make_shared<AggregateTypeSyntax>(
            "float3",
            "float3(0.0)",
            "float3(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS));

    registerTypeSyntax(
        Type::COLOR4,
        std::make_shared<AggregateTypeSyntax>(
            "float4",
            "float4(0.0)",
            "float4(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR2,
        std::make_shared<AggregateTypeSyntax>(
            "float2",
            "float2(0.0)",
            "float2(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC2_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR3,
        std::make_shared<AggregateTypeSyntax>(
            "float3",
            "float3(0.0)",
            "float3(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC3_MEMBERS));

    registerTypeSyntax(
        Type::VECTOR4,
        std::make_shared<AggregateTypeSyntax>(
            "float4",
            "float4(0.0)",
            "float4(0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            VEC4_MEMBERS));

    registerTypeSyntax(
        Type::MATRIX33,
        std::make_shared<AggregateTypeSyntax>(
            "float3x3",
            "float3x3(1.0)",
            "float3x3(1.0)"));

    registerTypeSyntax(
        Type::MATRIX44,
        std::make_shared<AggregateTypeSyntax>(
            "float4x4",
            "float4x4(1.0)",
            "float4x4(1.0)"));

    registerTypeSyntax(
        Type::STRING,
        std::make_shared<HlslStringTypeSyntax>());

    registerTypeSyntax(
        Type::FILENAME,
        std::make_shared<ScalarTypeSyntax>(
            "Texture2D",
            EMPTY_STRING,
            EMPTY_STRING));

    registerTypeSyntax(
        Type::BSDF,
        std::make_shared<AggregateTypeSyntax>(
            "BSDF",
            "BSDF(float3(0.0),float3(1.0), 0.0, 0.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct BSDF { float3 response; float3 throughput; float thickness; float ior; };"));

    registerTypeSyntax(
        Type::EDF,
        std::make_shared<AggregateTypeSyntax>(
            "EDF",
            "EDF(0.0)",
            "EDF(0.0)",
            "float3",
            "#define EDF float3"));

    registerTypeSyntax(
        Type::VDF,
        std::make_shared<AggregateTypeSyntax>(
            "BSDF",
            "BSDF(float3(0.0),float3(1.0), 0.0, 0.0)",
            EMPTY_STRING));

    registerTypeSyntax(
        Type::SURFACESHADER,
        std::make_shared<AggregateTypeSyntax>(
            "surfaceshader",
            "surfaceshader(float3(0.0),float3(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct surfaceshader { float3 color; float3 transparency; };"));

    registerTypeSyntax(
        Type::VOLUMESHADER,
        std::make_shared<AggregateTypeSyntax>(
            "volumeshader",
            "volumeshader(float3(0.0),float3(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct volumeshader { float3 color; float3 transparency; };"));

    registerTypeSyntax(
        Type::DISPLACEMENTSHADER,
        std::make_shared<AggregateTypeSyntax>(
            "displacementshader",
            "displacementshader(float3(0.0),1.0)",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct displacementshader { float3 offset; float scale; };"));

    registerTypeSyntax(
        Type::LIGHTSHADER,
        std::make_shared<AggregateTypeSyntax>(
            "lightshader",
            "lightshader(float3(0.0),float3(0.0))",
            EMPTY_STRING,
            EMPTY_STRING,
            "struct lightshader { float3 intensity; float3 direction; };"));

    registerTypeSyntax(
        Type::MATERIAL,
        std::make_shared<AggregateTypeSyntax>(
            "material",
            "material(float3(0.0),float3(0.0))",
            EMPTY_STRING,
            "surfaceshader",
            "#define material surfaceshader"));
}

bool HlslSyntax::typeSupported(const TypeDesc* type) const
{
    return type != Type::STRING;
}

bool HlslSyntax::remapEnumeration(const string& value, const TypeDesc* type, const string& enumNames, std::pair<const TypeDesc*, ValuePtr>& result) const
{
    // Early out if not an enum input.
    if (enumNames.empty())
    {
        return false;
    }

    // Don't convert already supported types
    // or filenames and arrays.
    if (typeSupported(type) ||
        type == Type::FILENAME || (type && type->isArray()))
    {
        return false;
    }

    // For GLSL we always convert to integer,
    // with the integer value being an index into the enumeration.
    result.first = Type::INTEGER;
    result.second = nullptr;

    // Try remapping to an enum value.
    if (!value.empty())
    {
        StringVec valueElemEnumsVec = splitString(enumNames, ",");
        for (size_t i = 0; i < valueElemEnumsVec.size(); i++)
        {
            valueElemEnumsVec[i] = trimSpaces(valueElemEnumsVec[i]);
        }
        auto pos = std::find(valueElemEnumsVec.begin(), valueElemEnumsVec.end(), value);
        if (pos == valueElemEnumsVec.end())
        {
            throw ExceptionShaderGenError("Given value '" + value + "' is not a valid enum value for input.");
        }
        const int index = static_cast<int>(std::distance(valueElemEnumsVec.begin(), pos));
        result.second = Value::createValue<int>(index);
    }

    return true;
}

MATERIALX_NAMESPACE_END
