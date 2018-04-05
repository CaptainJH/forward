#include "HLSLTextureBuffer.h"

using namespace forward;

HLSLTextureBuffer::~HLSLTextureBuffer()
{
}

HLSLTextureBuffer::HLSLTextureBuffer(
	D3D11_SHADER_INPUT_BIND_DESC const& desc, u32 numBytes,
	std::vector<Member> const& members)
	: HLSLBaseBuffer(desc, numBytes, members)
{
}

HLSLTextureBuffer::HLSLTextureBuffer(
	D3D11_SHADER_INPUT_BIND_DESC const& desc, u32 index,
	u32 numBytes, std::vector<Member> const& members)
	: HLSLBaseBuffer(desc, index, numBytes, members)
{
}