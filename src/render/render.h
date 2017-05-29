//***************************************************************************************
// render.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once
#include "Types.h"
#include "DataFormat.h"


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


	class Renderer
	{
	public:
		virtual RendererAPI GetRendererAPI() const = 0;

	protected:
		Renderer();
		virtual ~Renderer();

		// Static renderer access - used for accessing the renderer when no reference
		// is already available.

		static Renderer*					m_spRenderer;
	};
}