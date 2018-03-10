//***************************************************************************************
// DeviceVertexBufferDX11.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "DeviceBufferDX11.h"

namespace forward
{
	class FrameGraphVertexBuffer;

	class DeviceVertexBufferDX11 : public DeviceBufferDX11
	{
	public:

		DeviceVertexBufferDX11(ID3D11Device* device, FrameGraphVertexBuffer* vb);

		ResourceType	GetType() override;

		void Bind(ID3D11DeviceContext* context);
		void Unbind(ID3D11DeviceContext* context);
	};
}
