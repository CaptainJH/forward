//***************************************************************************************
// render.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once
#include "Types.h"
#include "DataFormat.h"


namespace forward
{
	// Subresource information.
	struct Subresource
	{
		void* data;
		u32 rowPitch;
		u32 slicePitch;
	};


	class Renderer
	{
	protected:
		Renderer();
		virtual ~Renderer();

		// Static renderer access - used for accessing the renderer when no reference
		// is already available.

		static Renderer*					m_spRenderer;
	};
}