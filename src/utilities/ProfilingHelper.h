#pragma once
#include "PCH.h"

namespace forward
{

	struct ProfilingHelper
	{
		static void BeginSuperluminalEvent(const i8* label, const u8 r, const u8 g, const u8 b);
		static void EndSuperluminalEvent();

		static void BeginPixEvent(const i8* label, const u8 r, const u8 g, const u8 b);
		static void BeginPixEvent(const i8* label);
		static void EndPixEvent();

		ProfilingHelper();
	};
}
