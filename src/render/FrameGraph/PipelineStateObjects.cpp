//***************************************************************************************
// PipelineStateObject.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "PipelineStateObjects.h"

using namespace forward;

FrameGraphDrawingState::FrameGraphDrawingState(const std::string& name, FrameGraphObjectType type)
{
	m_name = name;
	m_type = type;
}

BlendState::BlendState()
	:
	FrameGraphDrawingState("", FGOT_BLEND_STATE),
	enableAlphaToCoverage(false),
	enableIndependentBlend(false),
	blendColor({ 0.0f, 0.0f, 0.0f, 0.0f }),
	sampleMask(0xFFFFFFFFu)
{
	for (int i = 0; i < NUM_TARGETS; ++i)
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
	FrameGraphDrawingState("", FGOT_DEPTH_STENCIL_STATE),
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
	FrameGraphDrawingState("", FGOT_RASTERIZER_STATE),
	fillMode(FILL_SOLID),
	cullMode(CULL_BACK),
	//frontCCW(true),
	frontCCW(false),
	depthBias(0),
	depthBiasClamp(0.0f),
	slopeScaledDepthBias(0.0f),
	enableDepthClip(true),
	enableScissor(false),
	enableMultisample(false),
	enableAntialiasedLine(false)
{

}

void RasterizerStageState::AddViewport(ViewPort vp)
{
	assert(m_activeViewportsNum < FORWARD_RENDERER_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
	m_viewports[m_activeViewportsNum++] = vp;
}