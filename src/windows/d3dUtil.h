//***************************************************************************************
// d3dUtil.h by Frank Luna (C) 2011 All Rights Reserved.
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

#include "Vector4f.h"

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


//---------------------------------------------------------------------------------------
// Utility classes.
//---------------------------------------------------------------------------------------

	class TextHelper
	{
	public:

		template<typename T>
		static std::wstring ToString(const T& s)
		{
			std::wostringstream oss;
			oss << s;

			return oss.str();
		}

		template<typename T>
		static T FromString(const std::wstring& s)
		{
			T x;
			std::wistringstream iss(s);
			iss >> x;

			return x;
		}

		static std::string ToAscii(const std::wstring& input)
		{
			return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(input);
		}

		static std::wstring ToUnicode(const std::string& input)
		{
			return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().from_bytes(input);
		}
	};


	// #define XMGLOBALCONST extern CONST __declspec(selectany)
	//   1. extern so there is only one copy of the variable, and not a separate
	//      private copy in each .obj.
	//   2. __declspec(selectany) so that the compiler does not complain about
	//      multiple definitions in a .cpp file (it can pick anyone and discard 
	//      the rest because they are constant--all the same).

	namespace Colors
	{
		extern Vector4f White;
		extern Vector4f Black;
		extern Vector4f Red;
		extern Vector4f Green;
		extern Vector4f Blue;
		extern Vector4f Yellow;
		extern Vector4f Cyan;
		extern Vector4f Magenta;
		extern Vector4f Silver;
		extern Vector4f LightSteelBlue;
	}

}

#endif // D3DUTIL_H