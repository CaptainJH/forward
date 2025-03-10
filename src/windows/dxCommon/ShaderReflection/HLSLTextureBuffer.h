#pragma once
#include "windows/dxCommon/ShaderReflection/HLSLBaseBuffer.h"

namespace forward
{
	class HLSLTextureBuffer : public HLSLBaseBuffer
	{
	public:
		// Construction and destruction.
		virtual ~HLSLTextureBuffer() {}

		template<class D3D_SHADER_INPUT_BIND_DESC>
		HLSLTextureBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc,
			u32 numBytes, std::vector<Member> const& members)
			: HLSLBaseBuffer(desc, numBytes, members)
		{

		}

		template<class D3D_SHADER_INPUT_BIND_DESC>
		HLSLTextureBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc,
			u32 index, u32 numBytes,
			std::vector<Member> const& members)
			: HLSLBaseBuffer(desc, index, numBytes, members)
		{

		}
	};
}