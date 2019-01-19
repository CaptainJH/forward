#include "ShaderDX12.h"
#include "render/ShaderSystem/FrameGraphShader.h"
#include "dxCommon/ShaderFactoryDX.h"

using namespace forward;

ShaderDX12::ShaderDX12(forward::FrameGraphShader* shader)
	: ShaderDX(shader)
{
	auto type = shader->GetType();
	switch (type)
	{
	case FGOT_VERTEX_SHADER:
		m_shaderType = VERTEX_SHADER;
		ShaderModel = L"vs";
		break;

	case FGOT_PIXEL_SHADER:
		m_shaderType = PIXEL_SHADER;
		ShaderModel = L"ps";
		break;

	case FGOT_GEOMETRY_SHADER:
		m_shaderType = GEOMETRY_SHADER;
		ShaderModel = L"gs";
		break;

	case FGOT_COMPUTE_SHADER:
		m_shaderType = COMPUTE_SHADER;
		ShaderModel = L"cs";
		break;

	default:
		assert(false);
	}

	ShaderModel += L"_5_0";

	m_pCompiledShader = ShaderFactoryDX::GenerateShader(GetType(),
		shader->GetShaderFile(), shader->GetShaderEntry(), ShaderModel);
	assert(m_pCompiledShader);
}

ShaderDX12::~ShaderDX12()
{}

ShaderType ShaderDX12::GetType() const
{
	return m_shaderType;
}
