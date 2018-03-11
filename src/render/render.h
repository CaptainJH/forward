//***************************************************************************************
// render.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once
#include "Types.h"
#include "DataFormat.h"
#include "ResourceSystem/DeviceResource.h"


namespace forward
{
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


	class Renderer
	{
	public:
		virtual RendererAPI GetRendererAPI() const = 0;

		virtual void DeleteResource(ResourcePtr ptr) = 0;

		virtual void DrawRenderPass(RenderPass& pass) = 0;

	protected:
		Renderer();
		virtual ~Renderer();

		// Static renderer access - used for accessing the renderer when no reference
		// is already available.

		static Renderer*					m_spRenderer;
	};
}