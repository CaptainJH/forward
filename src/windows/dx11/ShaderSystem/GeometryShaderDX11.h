#pragma once

#include "ShaderDX11.h"
#include "render/ShaderSystem/Shader.h"

namespace forward
{
	class GeometryShaderDX11 : public ShaderDX11
	{
	public:
		GeometryShaderDX11(ID3D11Device* device, forward::GraphicsObject* obj);
		virtual ~GeometryShaderDX11();

		// Calls to ID3D11DeviceContext::XSSetShader.
		void Bind(ID3D11DeviceContext* context) override;
		static void Unbind(ID3D11DeviceContext* context);

		// Calls to ID3D11DeviceContext::XSSetConstantBuffers.
		void BindCBuffer(ID3D11DeviceContext* context,
			u32 bindPoint, ID3D11Buffer* buffer) override;
		void UnbindCBuffer(ID3D11DeviceContext* context,
			u32 bindPoint) override;

		// Calls to ID3D11DeviceContext::XSSetShaderResources.
		void BindSRView(ID3D11DeviceContext* context,
			u32 bindPoint, ID3D11ShaderResourceView* srView) override;
		void UnbindSRView(ID3D11DeviceContext* context,
			u32 bindPoint) override;

		// Calls to ID3D11DeviceContext::XSSetUnorderedAccessViews.
		//void BindUAView(ID3D11DeviceContext* context,
		//	u32 bindPoint, ID3D11UnorderedAccessView* uaView,
		//	u32 initialCount) = 0;
		//void UnbindUAView(ID3D11DeviceContext* context,
		//	u32 bindPoint) = 0;

		// Calls to ID3D11DeviceContext::XSSetSamplers.
		void BindSampler(ID3D11DeviceContext* context,
			u32 bindPoint, ID3D11SamplerState* state) override;
		void UnbindSampler(ID3D11DeviceContext* context,
			u32 bindPoint) override;

		ShaderType GetType() const override { return GEOMETRY_SHADER; }
	};
}