#include "ProfilingHelper.h"
#include "Superluminal/PerformanceAPI.h"
#ifdef USE_PIX
#include "pix3.h"
#endif

using namespace forward;

static PerformanceAPI_Functions s_PerformanceAPI;


void ProfilingHelper::BeginSuperluminalEvent(const i8* label, const u8 r, const u8 g, const u8 b)
{
	s_PerformanceAPI.BeginEvent(label, NULL, PERFORMANCEAPI_MAKE_COLOR(r, g, b));
}

void ProfilingHelper::EndSuperluminalEvent()
{
	s_PerformanceAPI.EndEvent();
}

#ifdef USE_PIX
void ProfilingHelper::BeginPixEvent(const i8* label, const u8 r, const u8 g, const u8 b)
{
	PIXBeginEvent(PIX_COLOR(r, g, b), label);
}

void ProfilingHelper::BeginPixEvent(const i8* label)
{
	PIXBeginEvent(PIX_COLOR_DEFAULT, label);
}

void ProfilingHelper::EndPixEvent()
{
	PIXEndEvent();
}
#endif

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

ProfilingHelper::ProfilingHelper()
{
	memset(&s_PerformanceAPI, 0, sizeof(PerformanceAPI_Functions));
	PerformanceAPI_Load(L"C:\\Program Files\\Superluminal\\Performance\\API\\dll\\x64\\PerformanceAPI.dll", &s_PerformanceAPI);
}

static ProfilingHelper s_ProfilingHelperInstance;