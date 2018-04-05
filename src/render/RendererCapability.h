//***************************************************************************************
// RendererCapability.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once


namespace forward
{

	/// DirectX 11 capabilities

	#define FORWARD_RENDERER_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT		32			// D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT
	#define FORWARD_RENDERER_SIMULTANEOUS_RENDER_TARGET_COUNT			8			// D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT
	#define FORWARD_RENDERER_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE	16	// D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE
	#define FORWARD_RENDERER_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT		14	// D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT
	#define FORWARD_RENDERER_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT				128	// D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT
	#define FORWARD_RENDERER_COMMONSHADER_SAMPLER_SLOT_COUNT					16	//D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT
}