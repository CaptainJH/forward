#include "ShaderDX12.h"
#include "render/ShaderSystem/FrameGraphShader.h"

using namespace forward;

ShaderDX12::ShaderDX12(forward::FrameGraphObject* obj)
	: ShaderDX(obj)
{
	auto type = m_frameGraphObjPtr.lock_down<FrameGraphShader>()->GetType();
	switch (type)
	{
	case FGOT_VERTEX_SHADER:
		m_shaderType = VERTEX_SHADER;
		break;

	case FGOT_PIXEL_SHADER:
		m_shaderType = PIXEL_SHADER;
		break;

	case FGOT_GEOMETRY_SHADER:
		m_shaderType = GEOMETRY_SHADER;
		break;

	case FGOT_COMPUTE_SHADER:
		m_shaderType = COMPUTE_SHADER;
		break;

	default:
		assert(false);
	}
}

ShaderDX12::~ShaderDX12()
{}

ShaderType ShaderDX12::GetType() const
{
	return m_shaderType;
}
