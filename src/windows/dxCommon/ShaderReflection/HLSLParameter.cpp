#include "HLSLParameter.h"

using namespace forward;

std::string const& HLSLParameter::GetSemanticName() const
{
	return m_desc.semanticName;
}

u32 HLSLParameter::GetSemanticIndex() const
{
	return m_desc.semanticIndex;
}

u32 HLSLParameter::GetRegisterIndex() const
{
	return m_desc.registerIndex;
}

D3D_NAME HLSLParameter::GetSystemValueType() const
{
	return m_desc.systemValueType;
}

D3D_REGISTER_COMPONENT_TYPE HLSLParameter::GetComponentType() const
{
	return m_desc.componentType;
}

u32 HLSLParameter::GetMask() const
{
	return m_desc.mask;
}

u32 HLSLParameter::GetReadWriteMask() const
{
	return m_desc.readWriteMask;
}

u32 HLSLParameter::GetStream() const
{
	return m_desc.stream;
}

D3D_MIN_PRECISION HLSLParameter::GetMinPrecision() const
{
	return m_desc.minPrecision;
}

void HLSLParameter::Print(std::ofstream& output) const
{
	output << "semantic name = " << m_desc.semanticName << std::endl;
	output << "semantic index = " << m_desc.semanticIndex << std::endl;
	output << "register index = " << m_desc.registerIndex << std::endl;
	output << "system value type = "
		<< msSVName[m_desc.systemValueType] << std::endl;
	output << "register component type = "
		<< msComponentType[m_desc.componentType] << std::endl;

	output << std::hex << std::showbase;
	output << "mask = " << m_desc.mask << std::endl;
	output << "read-write mask = " << m_desc.readWriteMask << std::endl;
	output << std::dec << std::noshowbase;

	output << "stream = " << m_desc.stream << std::endl;

	auto i = static_cast<i32>(m_desc.minPrecision);
	if (i & 0x000000F0)
	{
		i = 6 + (i & 1);
	}
	output << "min precision = " << msMinPrecision[i] << std::endl;
}


std::string const HLSLParameter::msSVName[] =
{
	"D3D_NAME_UNDEFINED",
	"D3D_NAME_POSITION",
	"D3D_NAME_CLIP_DISTANCE",
	"D3D_NAME_CULL_DISTANCE",
	"D3D_NAME_RENDER_TARGET_ARRAY_INDEX",
	"D3D_NAME_VIEWPORT_ARRAY_INDEX",
	"D3D_NAME_VERTEX_ID",
	"D3D_NAME_PRIMITIVE_ID",
	"D3D_NAME_INSTANCE_ID",
	"D3D_NAME_IS_FRONT_FACE",
	"D3D_NAME_SAMPLE_INDEX",
	"D3D_NAME_FINAL_QUAD_EDGE_TESSFACTOR",
	"D3D_NAME_FINAL_QUAD_INSIDE_TESSFACTOR",
	"D3D_NAME_FINAL_TRI_EDGE_TESSFACTOR",
	"D3D_NAME_FINAL_TRI_INSIDE_TESSFACTOR",
	"D3D_NAME_FINAL_LINE_DETAIL_TESSFACTOR",
	"D3D_NAME_FINAL_LINE_DENSITY_TESSFACTOR",
	"", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", // 17-63 unused
	"D3D_NAME_TARGET",
	"D3D_NAME_DEPTH",
	"D3D_NAME_COVERAGE",
	"D3D_NAME_DEPTH_GREATER_EQUAL",
	"D3D_NAME_DEPTH_LESS_EQUAL"
};

std::string const HLSLParameter::msComponentType[] =
{
	"D3D_REGISTER_COMPONENT_UNKNOWN",
	"D3D_REGISTER_COMPONENT_UINT32",
	"D3D_REGISTER_COMPONENT_SINT32",
	"D3D_REGISTER_COMPONENT_FLOAT32"
};

std::string const HLSLParameter::msMinPrecision[] =
{
	"D3D_MIN_PRECISION_DEFAULT",
	"D3D_MIN_PRECISION_FLOAT_16",
	"D3D_MIN_PRECISION_FLOAT_2_8",
	"D3D_MIN_PRECISION_RESERVED",
	"D3D_MIN_PRECISION_SINT_16",
	"D3D_MIN_PRECISION_UINT_16",
	"D3D_MIN_PRECISION_ANY_16",
	"D3D_MIN_PRECISION_ANY_10"
};