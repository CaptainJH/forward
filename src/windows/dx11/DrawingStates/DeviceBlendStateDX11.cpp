//***************************************************************************************
// DeviceBlendStateDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "DeviceBlendStateDX11.h"

using namespace forward;

DeviceBlendStateDX11::DeviceBlendStateDX11(ID3D11Device* device, BlendState* blendState)
	: DeviceDrawingStateDX11(blendState)
{
	// Specify the blend state description.
	D3D11_BLEND_DESC desc;
	desc.AlphaToCoverageEnable = (blendState->enableAlphaToCoverage ? TRUE : FALSE);
	desc.IndependentBlendEnable = (blendState->enableIndependentBlend ? TRUE : FALSE);
	for (auto i = 0; i < BlendState::NUM_TARGETS; ++i)
	{
		D3D11_RENDER_TARGET_BLEND_DESC& out = desc.RenderTarget[i];
		const BlendState::Target& in = blendState->target[i];
		out.BlendEnable = (in.enable ? TRUE : FALSE);
		out.SrcBlend = msMode[in.srcColor];
		out.DestBlend = msMode[in.dstColor];
		out.BlendOp = msOperation[in.opColor];
		out.SrcBlendAlpha = msMode[in.srcAlpha];
		out.DestBlendAlpha = msMode[in.dstAlpha];
		out.BlendOpAlpha = msOperation[in.opAlpha];
		out.RenderTargetWriteMask = in.mask;
	}

	// Create the blend state.
	ID3D11BlendState* state = nullptr;
	HR(device->CreateBlendState(&desc, &state));
	m_deviceObjPtr = state;
}

DeviceBlendStateDX11::~DeviceBlendStateDX11()
{
}

void DeviceBlendStateDX11::Bind(ID3D11DeviceContext* deviceContext)
{
	f32 afBlendFactors[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	auto state = GetBlendState();
	/// TODO: should use values in BlendState
	deviceContext->OMSetBlendState(state, afBlendFactors, 0xFFFFFFFF);
}

ID3D11BlendState* DeviceBlendStateDX11::GetBlendState()
{
	return static_cast<ID3D11BlendState*>(m_deviceObjPtr.Get());
}

D3D11_BLEND const DeviceBlendStateDX11::msMode[] =
{
	D3D11_BLEND_ZERO,
	D3D11_BLEND_ONE,
	D3D11_BLEND_SRC_COLOR,
	D3D11_BLEND_INV_SRC_COLOR,
	D3D11_BLEND_SRC_ALPHA,
	D3D11_BLEND_INV_SRC_ALPHA,
	D3D11_BLEND_DEST_ALPHA,
	D3D11_BLEND_INV_DEST_ALPHA,
	D3D11_BLEND_DEST_COLOR,
	D3D11_BLEND_INV_DEST_COLOR,
	D3D11_BLEND_SRC_ALPHA_SAT,
	D3D11_BLEND_BLEND_FACTOR,
	D3D11_BLEND_INV_BLEND_FACTOR,
	D3D11_BLEND_SRC1_COLOR,
	D3D11_BLEND_INV_SRC1_COLOR,
	D3D11_BLEND_SRC1_ALPHA,
	D3D11_BLEND_INV_SRC1_ALPHA
};

D3D11_BLEND_OP const DeviceBlendStateDX11::msOperation[] =
{
	D3D11_BLEND_OP_ADD,
	D3D11_BLEND_OP_SUBTRACT,
	D3D11_BLEND_OP_REV_SUBTRACT,
	D3D11_BLEND_OP_MIN,
	D3D11_BLEND_OP_MAX
};