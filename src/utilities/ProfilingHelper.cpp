#include "ProfilingHelper.h"
#ifdef USE_SUPERLUMINAL
#include "Superluminal/PerformanceAPI.h"
#endif
#ifdef USE_PIX
#include "pix3.h"
#endif

using namespace forward;

#ifdef USE_SUPERLUMINAL
static PerformanceAPI_Functions s_PerformanceAPI;
#endif


void ProfilingHelper::BeginSuperluminalEvent([[maybe_unused]] const i8* label, [[maybe_unused]] const u8 r, [[maybe_unused]] const u8 g, [[maybe_unused]] const u8 b)
{
#ifdef USE_SUPERLUMINAL
	s_PerformanceAPI.BeginEvent(label, NULL, PERFORMANCEAPI_MAKE_COLOR(r, g, b));
#endif
}

void ProfilingHelper::EndSuperluminalEvent()
{
#ifdef USE_SUPERLUMINAL
	s_PerformanceAPI.EndEvent();
#endif
}

void ProfilingHelper::BeginPixEvent([[maybe_unused]] const i8* label, [[maybe_unused]] const u8 r, [[maybe_unused]] const u8 g, [[maybe_unused]] const u8 b)
{
#ifdef USE_PIX
	PIXBeginEvent(PIX_COLOR(r, g, b), label);
#endif
}

void ProfilingHelper::BeginPixEvent([[maybe_unused]] const i8* label)
{
#ifdef USE_PIX
	PIXBeginEvent(PIX_COLOR_DEFAULT, label);
#endif
}

void ProfilingHelper::EndPixEvent()
{
#ifdef USE_PIX
	PIXEndEvent();
#endif
}

#ifdef USE_SUPERLUMINAL
int PerformanceAPI_Load(const wchar_t* inPathToDLL, PerformanceAPI_Functions* outFunctions)
{
	HMODULE module = LoadLibraryW(inPathToDLL);
	if (module == NULL)
		return 0;

	PerformanceAPI_GetAPI_Func getAPI = (PerformanceAPI_GetAPI_Func)((void*)GetProcAddress(module, "PerformanceAPI_GetAPI"));
	if (getAPI == NULL)
	{
		FreeLibrary(module);
		return 0;
	}

	if (getAPI(PERFORMANCEAPI_VERSION, outFunctions) == 0)
	{
		FreeLibrary(module);
		return 0;
	}

	return 1;
}
#endif

ProfilingHelper::ProfilingHelper()
{
#ifdef USE_SUPERLUMINAL
	memset(&s_PerformanceAPI, 0, sizeof(PerformanceAPI_Functions));
	PerformanceAPI_Load(L"C:\\Program Files\\Superluminal\\Performance\\API\\dll\\x64\\PerformanceAPI.dll", &s_PerformanceAPI);
#endif
}

static ProfilingHelper s_ProfilingHelperInstance;