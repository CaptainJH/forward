#pragma once
#include "HLSLBaseBuffer.h"

namespace forward
{
	class HLSLResourceBindInfo : public HLSLBaseBuffer
	{
	public:
		// Construction and destruction.
		virtual ~HLSLResourceBindInfo();

		HLSLResourceBindInfo(D3D11_SHADER_INPUT_BIND_DESC const& desc,
			u32 numBytes, std::vector<Member> const& members);

		HLSLResourceBindInfo(D3D11_SHADER_INPUT_BIND_DESC const& desc,
			u32 index, u32 numBytes,
			std::vector<Member> const& members);
	};
}