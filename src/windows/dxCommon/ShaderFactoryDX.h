#pragma once
//--------------------------------------------------------------------------------
#include "PCH.h"
#include "ShaderDX.h"
#include <functional>
//--------------------------------------------------------------------------------
namespace forward
{
	class ShaderFactoryDX
	{
	public:
		ShaderFactoryDX() = delete;

		static ID3DBlob* GenerateShader(const WString& shaderFileName, const String& shaderText, const String& function,
            const String& model, const D3D_SHADER_MACRO* pDefines = nullptr, bool enablelogging = true );

		static Vector<u8> GenerateShader6(const WString& shaderFileName, const String& shaderText, const String& function, 
			const String& model, std::function<void(Microsoft::WRL::ComPtr<ID3D12ShaderReflection>)>);
	};

};

