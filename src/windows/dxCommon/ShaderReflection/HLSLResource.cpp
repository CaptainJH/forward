#include "HLSLResource.h"

using namespace forward;

HLSLResource::~HLSLResource()
{
}

std::string const& HLSLResource::GetName() const
{
	return m_desc.name;
}

D3D_SHADER_INPUT_TYPE HLSLResource::GetType() const
{
	return m_desc.type;
}

u32 HLSLResource::GetBindPoint() const
{
	return m_desc.bindPoint;
}

u32 HLSLResource::GetBindCount() const
{
	return m_desc.bindCount;
}

u32 HLSLResource::GetFlags() const
{
	return m_desc.flags;
}

D3D_RESOURCE_RETURN_TYPE HLSLResource::GetReturnType() const
{
	return m_desc.returnType;
}

D3D_SRV_DIMENSION HLSLResource::GetDimension() const
{
	return m_desc.dimension;
}

u32 HLSLResource::GetNumSamples() const
{
	return m_desc.numSamples;
}

u32 HLSLResource::GetNumBytes() const
{
	return m_numBytes;
}

void HLSLResource::Print(std::ofstream& output) const
{
	output << "name = " << m_desc.name << std::endl;
	output << "shader input type = " << msSIType[m_desc.type] << std::endl;
	output << "bind point = " << m_desc.bindPoint << std::endl;
	output << "bind count = " << m_desc.bindCount << std::endl;
	output << "flags = " << m_desc.flags << std::endl;
	output << "return type = " << msReturnType[m_desc.returnType] << std::endl;
	output << "dimension = " << msSRVDimension[m_desc.dimension] << std::endl;
	if (m_desc.numSamples == 0xFFFFFFFFu)
	{
		output << "samples = -1" << std::endl;
	}
	else
	{
		output << "samples = " << m_desc.numSamples << std::endl;
	}
	output << "number of bytes = " << m_numBytes << std::endl;
}


std::string const HLSLResource::msSIType[] =
{
	"D3D_SIT_CBUFFER",
	"D3D_SIT_TBUFFER",
	"D3D_SIT_TEXTURE",
	"D3D_SIT_SAMPLER",
	"D3D_SIT_UAV_RWTYPED",
	"D3D_SIT_STRUCTURED",
	"D3D_SIT_UAV_RWSTRUCTURED",
	"D3D_SIT_BYTEADDRESS",
	"D3D_SIT_UAV_RWBYTEADDRESS",
	"D3D_SIT_UAV_APPEND_STRUCTURED",
	"D3D_SIT_UAV_CONSUME_STRUCTURED",
	"D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER"
};

std::string const HLSLResource::msReturnType[] =
{
	"none",  // There is no D3D_RESOURCE_RETURN_TYPE for value 0.
	"D3D_RETURN_TYPE_UNORM",
	"D3D_RETURN_TYPE_SNORM",
	"D3D_RETURN_TYPE_SINT",
	"D3D_RETURN_TYPE_UINT",
	"D3D_RETURN_TYPE_FLOAT",
	"D3D_RETURN_TYPE_MIXED",
	"D3D_RETURN_TYPE_DOUBLE",
	"D3D_RETURN_TYPE_CONTINUED"
};

std::string const HLSLResource::msSRVDimension[] =
{
	"D3D_SRV_DIMENSION_UNKNOWN",
	"D3D_SRV_DIMENSION_BUFFER",
	"D3D_SRV_DIMENSION_TEXTURE1D",
	"D3D_SRV_DIMENSION_TEXTURE1DARRAY",
	"D3D_SRV_DIMENSION_TEXTURE2D",
	"D3D_SRV_DIMENSION_TEXTURE2DARRAY",
	"D3D_SRV_DIMENSION_TEXTURE2DMS",
	"D3D_SRV_DIMENSION_TEXTURE2DMSARRAY",
	"D3D_SRV_DIMENSION_TEXTURE3D",
	"D3D_SRV_DIMENSION_TEXTURECUBE",
	"D3D_SRV_DIMENSION_TEXTURECUBEARRAY",
	"D3D_SRV_DIMENSION_BUFFEREX"
};