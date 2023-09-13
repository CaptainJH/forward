#pragma once
#pragma warning(disable : 4996)

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
	const f32 f_PI = 3.141592654f;
	const f32 f_2PI = 6.283185307f;
	const f32 f_1DIVPI = 0.318309886f;
	const f32 f_1DIV2PI = 0.159154943f;
	const f32 f_PIDIV2 = 1.570796327f;
	const f32 f_PIDIV4 = 0.785398163f;

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

	// Description:
	// The EResult enumeration defines acceptable return values from functions that can fail to complete successfully.
	enum EResult
	{
		E_RESULT_NO_ERROR = 0,				// The function completed successfully.
		E_RESULT_NULL_POINTER_ARGUMENT = 1,	// An argument to the function was NULL, but should be non-NULL to behave correctly.
		E_RESULT_NEGATIVE_ARGUMENT = 2,		// An argument to the function was negative, but should be positive to behave correctly.
		E_RESULT_ARGUMENT_OUT_OF_RANGE = 3,	// An argument to the function was outside of the accepted range.
		E_RESULT_EMPTY_ARGUMENT = 4,		// A string passed to the function was empty.
		E_RESULT_INVALID_ARGUMENT = 5,		// A passed object was of the wrong type.
		E_RESULT_CORRUPT_DATA_SOURCE = 6,	// An error was generated while parsing.
		E_RESULT_INSUFFICIENT_INFORMATION = 7,	// A matching type could not be found, or the required handler is not registered.
		E_RESULT_FUNCTION_OBSOLETE = 8,		// The function is now obsolete.
		E_RESULT_UNKNOWN_ERROR = 9,			// An unknown error occurred.
		E_RESULT_FILE_DOES_NOT_EXIST = 10,	// The requested file does not exist.
		E_RESULT_FILE_ACCESS_DENIED = 11,	// Access to the requested file was denied.
		E_RESULT_FILE_WRITE_PROTECTED = 12,	// The file cannot be written to because it is read only.
		E_RESULT_OUT_OF_MEMORY = 13,		// The function could not complete successfully due to a lack of memory.
		E_RESULT_UNABLE_TO_ALLOCATE = 14,	// The function could not complete successfully because it was unable to allocate or lock a required resource.
		E_RESULT_DLL_NOT_LOADED = 15,		// The DLL required is not loaded.
		E_RESULT_DLL_INCORRECT_VERSION = 16,	// The loaded DLL is an incorrect version.
		E_RESULT_INSUFFICIENT_DATA = 17,	// The file was truncated, or the array was not long enough.
		E_RESULT_OBJECT_OF_SAME_NAME_EXISTS = 18,	// An object with the same name already exists.
		E_RESULT_OBJECT_NOT_FOUND = 19,		// The requested object was not found.
		E_RESULT_OBJECT_OF_WRONG_TYPE = 20,	// The object was of the wrong type.
		E_RESULT_UNINITIALIZED_DATA = 21,	// Some data required has not been correctly initialized before use.
		E_RESULT_VALUE_ALREADY_SET = 22,	// The function is trying to update a value that can only be set once.
		E_RESULT_FEATURE_NOT_AVAILABLE = 23,	// The function could not complete because it tries to use a feature (or set of features) not currently supported.
		E_RESULT_INVALID_VALUE = 24,		// The value is abnormal.
		E_RESULT_COUNT						// The number of Phyre results.
	};


	template<typename Func1, typename Func2>
	struct ScopeGuard
	{
		ScopeGuard(Func1&& func1, Func2&& func2)
			: m_func1(func1), m_func2(func2)
		{
			m_func1();
		}

		~ScopeGuard()
		{
			m_func2();
		}

	private:
		Func1 m_func1;
		Func2 m_func2;
	};

	inline u32 Align(u32 size, u32 alignment)
	{
		return (size + (alignment - 1)) & ~(alignment - 1);
	}
}