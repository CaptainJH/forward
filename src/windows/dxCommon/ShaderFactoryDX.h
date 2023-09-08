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

		static ID3DBlob* GenerateShader( ShaderType type, const std::string& shaderText, const std::string& function,
            const std::string& model, const D3D_SHADER_MACRO* pDefines = nullptr, bool enablelogging = true );

		static Vector<u8> GenerateShader6(ShaderType type, const WString& shaderPath, const WString& function, 
			const WString& model, std::function<void(Microsoft::WRL::ComPtr<ID3D12ShaderReflection>)>);
	};

};

