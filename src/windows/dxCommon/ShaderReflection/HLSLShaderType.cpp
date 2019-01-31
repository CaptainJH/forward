#include "HLSLShaderType.h"
#include <assert.h>

using namespace forward;

HLSLShaderType::HLSLShaderType()
	: mName("")
{
}

void HLSLShaderType::SetName(std::string const& name)
{
	mName = name;
}

HLSLShaderType& HLSLShaderType::GetChild(u32 i)
{
	assert(i < mDesc.numChildren);
	return mChildren[i];
}

HLSLShaderType const& HLSLShaderType::GetChild(u32 i) const
{
	assert(i < mDesc.numChildren);
	return mChildren[i];
}

std::string const& HLSLShaderType::GetName() const
{
	return mName;
}

D3D_SHADER_VARIABLE_CLASS HLSLShaderType::GetClass() const
{
	return mDesc.varClass;
}

D3D_SHADER_VARIABLE_TYPE HLSLShaderType::GetType() const
{
	return mDesc.varType;
}

u32 HLSLShaderType::GetNumRows() const
{
	return mDesc.numRows;
}

u32 HLSLShaderType::GetNumColumns() const
{
	return mDesc.numColumns;
}

u32 HLSLShaderType::GetNumElements() const
{
	return mDesc.numElements;
}

u32 HLSLShaderType::GetNumChildren() const
{
	return mDesc.numChildren;
}

u32 HLSLShaderType::GetOffset() const
{
	return mDesc.offset;
}

std::string const& HLSLShaderType::GetTypeName() const
{
	return mDesc.typeName;
}

std::vector<HLSLShaderType> const& HLSLShaderType::GetChildren() const
{
	return mChildren;
}

void HLSLShaderType::Print(std::ofstream& output, i32 indent) const
{
	std::string prefix = "";
	for (auto i = 0; i < indent; ++i)
	{
		prefix += "    ";
	}

	output << prefix << "name = " << mName << std::endl;
	output << prefix << "variable class = " << msVarClass[mDesc.varClass]
		<< std::endl;
	output << prefix << "variable type = " << msVarType[mDesc.varType]
		<< std::endl;
	output << prefix << "rows = " << mDesc.numRows << std::endl;
	output << prefix << "columns = " << mDesc.numColumns << std::endl;
	output << prefix << "elements = " << mDesc.numElements << std::endl;
	output << prefix << "children = " << mDesc.numChildren << std::endl;
	output << prefix << "offset = " << mDesc.offset << std::endl;
	output << prefix << "type name = " << mDesc.typeName << std::endl;

	++indent;
	for (auto const& child : mChildren)
	{
		child.Print(output, indent);
	}
}


std::string const HLSLShaderType::msVarClass[] =
{
	"D3D_SVC_SCALAR",
	"D3D_SVC_VECTOR",
	"D3D_SVC_MATRIX_ROWS",
	"D3D_SVC_MATRIX_COLUMNS",
	"D3D_SVC_OBJECT",
	"D3D_SVC_STRUCT",
	"D3D_SVC_INTERFACE_CLASS",
	"D3D_SVC_INTERFACE_POINTER"
};

std::string const HLSLShaderType::msVarType[] =
{
	"D3D_SVT_VOID",
	"D3D_SVT_BOOL",
	"D3D_SVT_INT",
	"D3D_SVT_FLOAT",
	"D3D_SVT_STRING",
	"D3D_SVT_TEXTURE",
	"D3D_SVT_TEXTURE1D",
	"D3D_SVT_TEXTURE2D",
	"D3D_SVT_TEXTURE3D",
	"D3D_SVT_TEXTURECUBE",
	"D3D_SVT_SAMPLER",
	"D3D_SVT_SAMPLER1D",
	"D3D_SVT_SAMPLER2D",
	"D3D_SVT_SAMPLER3D",
	"D3D_SVT_SAMPLERCUBE",
	"D3D_SVT_PIXELSHADER",
	"D3D_SVT_VERTEXSHADER",
	"D3D_SVT_PIXELFRAGMENT",
	"D3D_SVT_VERTEXFRAGMENT",
	"D3D_SVT_UINT",
	"D3D_SVT_UINT8",
	"D3D_SVT_GEOMETRYSHADER",
	"D3D_SVT_RASTERIZER",
	"D3D_SVT_DEPTHSTENCIL",
	"D3D_SVT_BLEND",
	"D3D_SVT_BUFFER",
	"D3D_SVT_CBUFFER",
	"D3D_SVT_TBUFFER",
	"D3D_SVT_TEXTURE1DARRAY",
	"D3D_SVT_TEXTURE2DARRAY",
	"D3D_SVT_RENDERTARGETVIEW",
	"D3D_SVT_DEPTHSTENCILVIEW",
	"D3D_SVT_TEXTURE2DMS",
	"D3D_SVT_TEXTURE2DMSARRAY",
	"D3D_SVT_TEXTURECUBEARRAY",
	"D3D_SVT_HULLSHADER",
	"D3D_SVT_DOMAINSHADER",
	"D3D_SVT_INTERFACE_POINTER",
	"D3D_SVT_COMPUTESHADER",
	"D3D_SVT_DOUBLE",
	"D3D_SVT_RWTEXTURE1D",
	"D3D_SVT_RWTEXTURE1DARRAY",
	"D3D_SVT_RWTEXTURE2D",
	"D3D_SVT_RWTEXTURE2DARRAY",
	"D3D_SVT_RWTEXTURE3D",
	"D3D_SVT_RWBUFFER",
	"D3D_SVT_BYTEADDRESS_BUFFER",
	"D3D_SVT_RWBYTEADDRESS_BUFFER",
	"D3D_SVT_STRUCTURED_BUFFER",
	"D3D_SVT_RWSTRUCTURED_BUFFER",
	"D3D_SVT_APPEND_STRUCTURED_BUFFER",
	"D3D_SVT_CONSUME_STRUCTURED_BUFFER",
	"D3D_SVT_MIN8FLOAT",
	"D3D_SVT_MIN10FLOAT",
	"D3D_SVT_MIN16FLOAT",
	"D3D_SVT_MIN12INT",
	"D3D_SVT_MIN16INT",
	"D3D_SVT_MIN16UINT"
};