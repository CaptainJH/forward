#pragma once
#include "HLSLBaseBuffer.h"

namespace forward
{
	class HLSLResourceBindInfo : public HLSLBaseBuffer
	{
	public:
		// Construction and destruction.
		virtual ~HLSLResourceBindInfo() {}

		template<class D3D_SHADER_INPUT_BIND_DESC>
		HLSLResourceBindInfo(D3D_SHADER_INPUT_BIND_DESC const& desc,
			u32 numBytes, std::vector<Member> const& members)
			: HLSLBaseBuffer(desc, numBytes, members)
		{

		}

		template<class D3D_SHADER_INPUT_BIND_DESC>
		HLSLResourceBindInfo(D3D_SHADER_INPUT_BIND_DESC const& desc,
			u32 index, u32 numBytes,
			std::vector<Member> const& members)
			: HLSLBaseBuffer(desc, index, numBytes, members)
		{

		}
	};
}