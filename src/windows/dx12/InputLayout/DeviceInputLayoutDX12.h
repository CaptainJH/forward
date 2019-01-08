//***************************************************************************************
// DeviceInputLayoutDX12.h by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "render/ResourceSystem/DeviceObject.h"
#include "render/ResourceSystem/Buffers/FrameGraphBuffer.h"
#include "render/ShaderSystem/FrameGraphShader.h"
#include "dx12/dx12Util.h"

namespace forward
{
	class DeviceInputLayoutDX12 : public DeviceObject
	{
	public:
		DeviceInputLayoutDX12(ID3D12Device* device, const FrameGraphVertexBuffer* vbuffer, FrameGraphVertexShader* vshader);
		virtual ~DeviceInputLayoutDX12();

		//InputLayoutComPtr	GetInputLayout();

		//void	Bind(ID3D11DeviceContext* deviceContext);
		//void	Unbind(ID3D11DeviceContext* deviceContext);

	private:
		//InputLayoutComPtr			m_inputLayout;
		u32							m_numElements;
		D3D12_INPUT_ELEMENT_DESC	m_elements[VA_MAX_ATTRIBUTES];


		// Conversions from FrameGraph values to DX11 values.
		static i8 const* msSemantic[VA_NUM_SEMANTICS];
	};
}