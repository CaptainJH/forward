#pragma once

#include "dxCommon/ShaderDX.h"
#include "dx11/dx11Util.h"

namespace forward
{
	class ShaderDX11 : public ShaderDX
	{
	public:
		ShaderDX11(forward::FrameGraphObject* obj);
		virtual ~ShaderDX11();

		// Calls to ID3D11DeviceContext::XSSetShader.
		virtual void Bind(ID3D11DeviceContext* context) = 0;
		virtual void Unbind(ID3D11DeviceContext* context) = 0;

		// Calls to ID3D11DeviceContext::XSSetConstantBuffers.
		virtual void BindCBuffer(ID3D11DeviceContext* context,
			u32 bindPoint, ID3D11Buffer* buffer) = 0;
		virtual void UnbindCBuffer(ID3D11DeviceContext* context,
			u32 bindPoint) = 0;

		// Calls to ID3D11DeviceContext::XSSetShaderResources.
		virtual void BindSRView(ID3D11DeviceContext* context,
			u32 bindPoint, ID3D11ShaderResourceView* srView) = 0;
		virtual void UnbindSRView(ID3D11DeviceContext* context,
			u32 bindPoint) = 0;

		// Calls to ID3D11DeviceContext::XSSetUnorderedAccessViews.
		//virtual void BindUAView(ID3D11DeviceContext* context,
		//	u32 bindPoint, ID3D11UnorderedAccessView* uaView,
		//	u32 initialCount) = 0;
		//virtual void UnbindUAView(ID3D11DeviceContext* context,
		//	u32 bindPoint) = 0;

		// Calls to ID3D11DeviceContext::XSSetSamplers.
		virtual void BindSampler(ID3D11DeviceContext* context,
			u32 bindPoint, ID3D11SamplerState* state) = 0;
		virtual void UnbindSampler(ID3D11DeviceContext* context,
			u32 bindPoint) = 0;

		DeviceObjComPtr		GetDeviceObject();

	protected:
		DeviceObjComPtr		m_deviceObjPtr;

	};
}