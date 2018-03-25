#include "HLSLConstantBuffer.h"

using namespace forward;

HLSLConstantBuffer::~HLSLConstantBuffer()
{
}

HLSLConstantBuffer::HLSLConstantBuffer(
	D3D11_SHADER_INPUT_BIND_DESC const& desc, u32 numBytes,
	std::vector<Member> const& members)
	: HLSLBaseBuffer(desc, numBytes, members)
{
}

HLSLConstantBuffer::HLSLConstantBuffer(
	D3D11_SHADER_INPUT_BIND_DESC const& desc, u32 index,
	u32 numBytes, std::vector<Member> const& members)
	: HLSLBaseBuffer(desc, index, numBytes, members)
{
}