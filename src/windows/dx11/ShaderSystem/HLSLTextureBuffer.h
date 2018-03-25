#pragma once
#include "HLSLBaseBuffer.h"

namespace forward
{
	class HLSLTextureBuffer : public HLSLBaseBuffer
	{
	public:
		// Construction and destruction.
		virtual ~HLSLTextureBuffer();

		HLSLTextureBuffer(D3D11_SHADER_INPUT_BIND_DESC const& desc,
			u32 numBytes, std::vector<Member> const& members);

		HLSLTextureBuffer(D3D11_SHADER_INPUT_BIND_DESC const& desc,
			u32 index, u32 numBytes,
			std::vector<Member> const& members);
	};
}