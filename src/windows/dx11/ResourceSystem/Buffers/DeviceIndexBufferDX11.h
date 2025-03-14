//***************************************************************************************
// DeviceIndexBufferDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "DeviceBufferDX11.h"

namespace forward
{
	class IndexBuffer;

	class DeviceIndexBufferDX11 : public DeviceBufferDX11
	{
	public:

		DeviceIndexBufferDX11(ID3D11Device* device, IndexBuffer* ib);

		void Bind(ID3D11DeviceContext* context);
		void Unbind(ID3D11DeviceContext* context);

	private:
		DXGI_FORMAT m_format;
	};
}
