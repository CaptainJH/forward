//***************************************************************************************
// d3dUtil.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#ifndef D3DUTIL_H
#define D3DUTIL_H

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif


#include <cassert>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <locale>
#include <codecvt>
#include <d3dcommon.h>
#include "RHI/DataFormat.h"
#include "RHI/PrimitiveTopology.h"

namespace forward
{

	//---------------------------------------------------------------------------------------
	// Simple d3d error checker for book demos.
	//---------------------------------------------------------------------------------------

#ifndef HR
#define HR(x)												\
	{														\
		HRESULT hr = (x);									\
		if(FAILED(hr))										\
		{													\
			LPWSTR output;                                  \
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |		\
				FORMAT_MESSAGE_IGNORE_INSERTS 	 |			\
				FORMAT_MESSAGE_ALLOCATE_BUFFER,				\
				NULL,										\
				hr,											\
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	\
				(LPTSTR) &output,							\
				0,											\
				NULL);										\
			MessageBox(NULL, output, L"Error", MB_OK);		\
		}													\
	}
#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 

	D3D_PRIMITIVE_TOPOLOGY Convert2D3DTopology(PrimitiveTopologyType topo);

	//--------------------------------------------------------------------------------------
	// Get surface information for a particular format
	//--------------------------------------------------------------------------------------
	void GetSurfaceInfo(u32 width, u32 height, DataFormatType df, u32* outNumBytes=nullptr, u32* outRowBytes=nullptr, u32* outNumRows=nullptr);

}

#endif // D3DUTIL_H