#pragma once

#include "windows/dxCommon/ShaderDX.h"
#include "windows/dx12/dx12Util.h"

namespace forward
{
	class Shader;

	class ShaderDX12 : public ShaderDX
	{
	public:
		ShaderDX12(forward::Shader* shader);
		virtual ~ShaderDX12();

		ShaderType GetType() const override;

	private:
		ShaderType m_shaderType;

	private:
		void ReflectShader(Microsoft::WRL::ComPtr<ID3D12ShaderReflection> reflector);
		void ReflectLibrary(Microsoft::WRL::ComPtr<ID3D12LibraryReflection> reflector);

		static bool GetVariables(ID3D12ShaderReflectionConstantBuffer* cbuffer,
			u32 numVariables, std::vector<HLSLBaseBuffer::Member>& members);
		static bool GetTypes(ID3D12ShaderReflectionType* rtype,
			u32 numMembers, HLSLShaderType& stype);
		static bool IsTextureArray(D3D_SRV_DIMENSION dim);
	};
}
