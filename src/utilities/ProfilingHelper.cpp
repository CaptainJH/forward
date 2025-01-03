#include "ProfilingHelper.h"
#include "Utils.h"
#include "FileSystem.h"
#ifdef USE_SUPERLUMINAL
#include "Superluminal/PerformanceAPI.h"
#endif
#ifdef USE_PIX
#include <pix3.h>
#endif
#if USE_RENDERDOC
#include <renderdoc_app.h>
#endif

using namespace forward;

#ifdef USE_SUPERLUMINAL
static PerformanceAPI_Functions s_PerformanceAPI;
#endif

#if USE_RENDERDOC
static RENDERDOC_API_1_6_0* s_rdoc_api = nullptr;
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

void ProfilingHelper::BeginPixCapture([[maybe_unused]] const i8* filePath)
{
#ifdef USE_PIX
	PIXCaptureParameters pp = {};
	auto filePathW = TextHelper::ToUnicode(filePath);
	if (!FileSystem::getSingleton().FileExists(filePathW))
		filePathW = FileSystem::getSingleton().GetSavedFolder() + filePathW;
	pp.GpuCaptureParameters.FileName = filePathW.c_str();
	PIXBeginCapture(PIX_CAPTURE_GPU, &pp);
#endif
}

void ProfilingHelper::EndPixCapture()
{
#ifdef USE_PIX
	PIXEndCapture(TRUE);
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

#if USE_RENDERDOC
	HMODULE mod = LoadLibraryA(RENDERDOC_PATH"/renderdoc.dll");
	if (mod)
	{
		pRENDERDOC_GetAPI RENDERDOC_GetAPI =
			(pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
		auto ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_6_0, (void**)&s_rdoc_api);
		assert(ret == 1);
	}
#endif
}

void ProfilingHelper::BeginRenderDocCapture([[maybe_unused]] const i8* filePath)
{
#if USE_RENDERDOC
	if (s_rdoc_api)
	{
		s_rdoc_api->StartFrameCapture(NULL, NULL);
		s_rdoc_api->SetCaptureFilePathTemplate(filePath);
	}
#endif
}

void ProfilingHelper::EndRenderDocCapture()
{
#if USE_RENDERDOC
	if (s_rdoc_api)
		s_rdoc_api->EndFrameCapture(NULL, NULL);
#endif
}

static ProfilingHelper s_ProfilingHelperInstance;