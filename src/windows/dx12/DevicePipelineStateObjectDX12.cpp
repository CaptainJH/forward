//***************************************************************************************
// DevicePipelineStateObjectDX12.cpp by Heqi Ju (C) 2019 All Rights Reserved.
//***************************************************************************************
#include "DevicePipelineStateObjectDX12.h"
#include "dxCommon/ShaderDX.h"
#include "FrameGraph/PipelineStateObjects.h"

using namespace forward;

DevicePipelineStateObjectDX12::DevicePipelineStateObjectDX12(ID3D12Device* /*device*/, const PipelineStateObject& pso)
	: DeviceObject(nullptr)
	, m_numElements(0)
{
	ZeroMemory(&m_elements[0], VA_MAX_ATTRIBUTES * sizeof(m_elements[0]));

	const FrameGraphVertexBuffer* vbuffer = pso.m_IAState.m_vertexBuffers[0].get();
	const FrameGraphVertexShader* vshader = pso.m_VSState.m_shader.get();

	if (vbuffer && vshader)
	{
		const auto& vertexFormat = vbuffer->GetVertexFormat();
		m_numElements = vertexFormat.GetNumAttributes();
		for (auto i = 0U; i < m_numElements; ++i)
		{
			VASemantic semantic;
			DataFormatType type;
			u32 unit, offset;
			vertexFormat.GetAttribute(i, semantic, type, unit, offset);

			D3D12_INPUT_ELEMENT_DESC& element = m_elements[i];
			element.SemanticName = msSemantic[semantic];
			element.SemanticIndex = unit;
			element.Format = static_cast<DXGI_FORMAT>(type);
			element.InputSlot = 0;  // TODO: Streams not yet supported.
			element.AlignedByteOffset = offset;
			element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			element.InstanceDataStepRate = 0;
		}

	}
}

DevicePipelineStateObjectDX12::~DevicePipelineStateObjectDX12()
{

}