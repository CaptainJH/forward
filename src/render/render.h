//***************************************************************************************
// render.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once
#include "Types.h"
#include "DataFormat.h"
#include "ResourceSystem/DeviceResource.h"


namespace forward
{
	enum RendererType
	{
		Renderer_Hieroglyph,
		Renderer_Forward_DX11,
		Renderer_Forward_DX12,
	};

	enum RendererAPI
	{
		DirectX11,
		DirectX12
	};

	// Subresource information.
	struct Subresource
	{
		void* data;
		u32 rowPitch;
		u32 slicePitch;
	};

	class RenderPass;
	class SwapChainConfig;


	class Renderer
	{
	public:
		virtual ~Renderer();
		virtual RendererAPI GetRendererAPI() const = 0;

		virtual void DeleteResource(ResourcePtr ptr) = 0;

		virtual void DrawRenderPass(RenderPass& pass) = 0;

		virtual void OnResize(u32 width, u32 height) = 0;

		virtual bool Initialize(SwapChainConfig&) = 0;
		virtual void Shutdown() = 0;

	protected:
		Renderer();

		// Static renderer access - used for accessing the renderer when no reference
		// is already available.

		static Renderer*					m_spRenderer;
	};
}