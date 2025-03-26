//***************************************************************************************
// PipelineStateObject.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "PipelineStateObjects.h"
#include "utilities/pystring.h"
#include "math/Vector2f.h"
#include "math/Vector3f.h"
#include "RHI/SceneData.h"

using namespace forward;

DrawingState::DrawingState(const std::string& name, GraphicsObjectType type)
{
	m_name = name;
	m_type = type;
}

BlendState::BlendState()
	:
	DrawingState("", FGOT_BLEND_STATE),
	enableAlphaToCoverage(false),
	enableIndependentBlend(false),
	blendColor({ 0.0f, 0.0f, 0.0f, 0.0f }),
	sampleMask(0xFFFFFFFFu)
{
	for (auto i = 0U; i < NUM_TARGETS; ++i)
	{
		Target& trg = target[i];
		trg.enable = false;
		trg.srcColor = BM_ONE;
		trg.dstColor = BM_ZERO;
		trg.opColor = OP_ADD;
		trg.srcAlpha = BM_ONE;
		trg.dstAlpha = BM_ZERO;
		trg.opAlpha = OP_ADD;
		trg.mask = CW_ENABLE_ALL;
	}
}

DepthStencilState::DepthStencilState()
	:
	DrawingState("", FGOT_DEPTH_STENCIL_STATE),
	depthEnable(true),
	writeMask(MASK_ALL),
	comparison(LESS_EQUAL),
	stencilEnable(false),
	stencilReadMask(0xFF),
	stencilWriteMask(0xFF),
	reference(0)
{
	frontFace.fail = OP_KEEP;
	frontFace.depthFail = OP_KEEP;
	frontFace.pass = OP_KEEP;
	frontFace.comparison = ALWAYS;
	backFace.fail = OP_KEEP;
	backFace.depthFail = OP_KEEP;
	backFace.pass = OP_KEEP;
	backFace.comparison = ALWAYS;
}


RasterizerState::RasterizerState()
	:
	DrawingState("", FGOT_RASTERIZER_STATE),
	fillMode(FILL_SOLID),
	cullMode(CULL_BACK),
	frontCCW(true),
	depthBias(0),
	depthBiasClamp(0.0f),
	slopeScaledDepthBias(0.0f),
	enableDepthClip(true),
	enableScissor(false),
	enableMultisample(false),
	enableAntialiasedLine(false)
{

}

void RasterizerStageParameters::AddViewport(ViewPort vp)
{
	assert(m_activeViewportsNum < FORWARD_RENDERER_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
	m_viewports[m_activeViewportsNum++] = vp;
}

void RasterizerStageParameters::AddScissorRect(forward::RECT rect)
{
	assert(m_activeScissorRectNum < FORWARD_RENDERER_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
	m_scissorRects[m_activeScissorRectNum++] = rect;
}

SamplerState::SamplerState(const std::string& name)
	: 
	filter(ANISOTROPIC)
	, mipLODBias(0.0f)
	, maxAnisotropy(1)
	, comparison(NEVER)
	, borderColor({1.0f, 1.0f, 1.0f, 1.0f})
	, minLOD(-1024)
	, maxLOD(1024)
{
	SetName(name);
	m_type = FGOT_SAMPLER_STATE;
	mode[0] = WRAP;
	mode[1] = WRAP;
	mode[2] = WRAP;
}

void ShaderStageParameters::SetConstantBufferData(Shader& shader, u32 index, std::unordered_map<String, String>& params)
{
	auto dst = m_constantBuffers[index]->GetData();

	for (auto& p : params)
	{
		if (shader.m_shaderParamsInfo.contains(p.first))
		{
			const auto& info = shader.m_shaderParamsInfo[p.first];
			if (index != info.bind)
				continue;
			const auto offset = info.offset;
			const auto size = info.size;
			void* dataPtr = nullptr;
			if (info.typeName == "float")
			{
				f32 f = std::stof(p.second);
				dataPtr = &f;
			}
			else if (info.typeName == "float2")
			{
				Vector<String> v = pystring::split(p.second, ",");
				assert(v.size() == 2);
				Vector2f data(std::stof(v[0]), std::stof(v[1]));
				dataPtr = &data;
			}
			else if (info.typeName == "float3")
			{
				Vector<String> v = pystring::split(p.second, ",");
				assert(v.size() == 3);
				Vector3f data(std::stof(v[0]), std::stof(v[1]), std::stof(v[2]));
				dataPtr = &data;
			}
			else if (info.typeName == "float4")
			{
				Vector<String> v = pystring::split(p.second, ",");
				assert(v.size() == 4);
				Vector4f data(std::stof(v[0]), std::stof(v[1]), std::stof(v[2]), std::stof(v[3]));
				dataPtr = &data;
			}
			else if (info.typeName == "int")
			{
				i32 data = std::stoi(p.second);
				dataPtr = &data;
			}
			else if (info.typeName == "bool")
			{
				i32 data = p.second == "false" ? 0 : 1;
				dataPtr = &data;
			}

			assert(dataPtr);
			memcpy(dst + offset, dataPtr, size);
		}
	}
	m_constantBuffers[index]->SetDirty();
}

bool BindingRanges::AddRange(BindingRange range)
{
	auto newStartIt = m_ranges.end();
	auto newEndIt = m_ranges.end();
	for (auto it = m_ranges.begin(); it != m_ranges.end(); ++it)
	{
		if (
			(range.bindStart >= it->bindStart && range.bindStart <= it->bindEnd)
			||
			(range.bindEnd >= it->bindStart && range.bindEnd <= it->bindEnd)
			)
			return false;
		else if ((range.bindStart - 1 == it->bindEnd) && (newStartIt == m_ranges.end()))
			newStartIt = it;
		else if ((range.bindEnd + 1 == it->bindStart) && (newEndIt == m_ranges.end()))
			newEndIt = it;
	}
	if ((newStartIt != m_ranges.end()) && (newEndIt != m_ranges.end()))
	{
		newStartIt->bindEnd = newEndIt->bindEnd;
		m_ranges.erase(newEndIt);
	}
	else if (newStartIt != m_ranges.end())
		newStartIt->bindEnd = range.bindEnd;
	else if (newEndIt != m_ranges.end())
		newEndIt->bindStart = range.bindStart;
	else
		m_ranges.emplace_back(range);

	return true;
}

RTPipelineStateObject::RTPipelineStateObject(const SceneData& scene)
{
	m_type = FGOT_RT_PSO;
	FeedWithSceneData(scene);
}

void RTPipelineStateObject::FeedWithSceneData(const SceneData& scene)
{
	m_meshes.reserve(scene.mMeshData.size());
	for (auto& mesh : scene.mMeshData)
		m_meshes.emplace_back(mesh.m_VB, mesh.m_IB);

	m_instances.reserve(scene.mInstances.size());
	for (auto& ins : scene.mInstances)
		m_instances.emplace_back(InstanceData{
			.meshId = ins.meshId,
			.materialId = ins.materialId,
			.object2WorldMat = ins.mat
			});
}