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
		static void BeginPixCapture(const i8* filePath);
		static void EndPixCapture();

		ProfilingHelper();
	};
}

#define ScopedSuperluminalEvent(NAME, R, G, B) auto NAME = ScopeGuard([] {\
		forward::ProfilingHelper::BeginSuperluminalEvent(#NAME, R, G, B);\
		}, [] {\
			forward::ProfilingHelper::EndSuperluminalEvent();\
		});

#define ScopedPixEvent(NAME, R, G, B) auto NAME = ScopeGuard([] {\
		forward::ProfilingHelper::BeginPixEvent(#NAME, R, G, B);\
		}, [] {\
			forward::ProfilingHelper::EndPixEvent();\
		});
