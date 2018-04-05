#include "HLSLResourceBindInfo.h"

using namespace forward;

HLSLResourceBindInfo::~HLSLResourceBindInfo()
{
}

HLSLResourceBindInfo::HLSLResourceBindInfo(
	D3D11_SHADER_INPUT_BIND_DESC const& desc, u32 numBytes,
	std::vector<Member> const& members)
	: HLSLBaseBuffer(desc, numBytes, members)
{
}

HLSLResourceBindInfo::HLSLResourceBindInfo(
	D3D11_SHADER_INPUT_BIND_DESC const& desc, u32 index,
	u32 numBytes, std::vector<Member> const& members)
	: HLSLBaseBuffer(desc, index, numBytes, members)
{
}