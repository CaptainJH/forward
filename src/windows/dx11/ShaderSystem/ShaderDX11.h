#pragma once

#include "dxCommon/ShaderDX.h"
#include "dx11/dx11Util.h"
#include "dxCommon/ShaderReflection/HLSLParameter.h"
#include "dxCommon/ShaderReflection/HLSLConstantBuffer.h"
#include "dxCommon/ShaderReflection/HLSLResourceBindInfo.h"
#include "dxCommon/ShaderReflection/HLSLTextureBuffer.h"
#include "dxCommon/ShaderReflection/HLSLTexture.h"
#include "dxCommon/ShaderReflection/HLSLTextureArray.h"

namespace forward
{
	class ShaderDX11 : public ShaderDX
	{
	public:
		ShaderDX11(forward::GraphicsObject* obj);
		virtual ~ShaderDX11();

		// Calls to ID3D11DeviceContext::XSSetShader.
		virtual void Bind(ID3D11DeviceContext* context) = 0;

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

		void ReflectShader();
		void PostSetDeviceObject(forward::GraphicsObject* obj) override;

	protected:

		void SetDescriptionDX11(const D3D11_SHADER_DESC& desc);

		static bool GetVariables(ID3D11ShaderReflectionConstantBuffer* cbuffer, 
			u32 numVariables, std::vector<HLSLBaseBuffer::Member>& members);
		static bool GetTypes(ID3D11ShaderReflectionType* rtype,
			u32 numMembers, HLSLShaderType& stype);
		static bool ShaderDX11::IsTextureArray(D3D_SRV_DIMENSION dim);

	protected:
		DeviceObjComPtr		m_deviceObjPtr;
	};
}