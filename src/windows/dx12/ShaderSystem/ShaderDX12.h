#pragma once

#include "dxCommon/ShaderDX.h"
#include "dx12/dx12Util.h"

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
		void ReflectShader();

	};
}
