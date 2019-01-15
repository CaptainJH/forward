#pragma once

#include "dxCommon/ShaderDX.h"
#include "dx12/dx12Util.h"

namespace forward
{
	class ShaderDX12 : public ShaderDX
	{
	public:
		ShaderDX12(forward::FrameGraphObject* obj);
		virtual ~ShaderDX12();

		ShaderType GetType() const override;

	private:
		ShaderType m_shaderType;

	};
}
