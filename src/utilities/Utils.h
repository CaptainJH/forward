#pragma once

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