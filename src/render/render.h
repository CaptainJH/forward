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
}