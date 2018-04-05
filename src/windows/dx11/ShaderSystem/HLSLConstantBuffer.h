#pragma once
#include "HLSLBaseBuffer.h"

namespace forward
{
	class HLSLConstantBuffer : public HLSLBaseBuffer
	{
	public:
		// TODO: Global constants in the HLSL file are placed as variables into
		// an implicit constant buffer named $Global.  We need to modify
		// Variable to store number of bytes and default values (if any).
		// 1. If the application writer must create the constant buffer and
		//    attach, how do we get the default data into the buffer?
		// 2. Maybe $Global needs to be created implicitly, filled with
		//    default values, then application queries for it to modify.  The
		//    idea of shader->Set("SomeCBuffer",cbuffer) was to allow sharing
		//    of constant buffers between shaders.

		// Construction and destruction.
		virtual ~HLSLConstantBuffer();

		HLSLConstantBuffer(D3D11_SHADER_INPUT_BIND_DESC const& desc,
			u32 numBytes, std::vector<Member> const& members);

		HLSLConstantBuffer(D3D11_SHADER_INPUT_BIND_DESC const& desc,
			u32 index, u32 numBytes,
			std::vector<Member> const& members);
	};
}