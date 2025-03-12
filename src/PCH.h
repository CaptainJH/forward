#pragma once


#include <cfloat>
#include <string>
#include <sstream>
#include <fstream>
#include <array>
#include <vector>
#include <map>
#include <algorithm>
#include <execution>
#include <memory>

#include <thread>
#include <chrono>

#include "Types.h"

#ifdef _WINDOWS
#include <wrl.h>
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE( x ) {if(x){(x)->Release();(x)=NULL;}}
#endif // !SAFE_RELEASE
#ifndef SAFE_DELETE
#define SAFE_DELETE( x ) {if(x){delete (x);(x)=NULL;}}
#endif // !SAFE_DELETE
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY( x ) {if(x){delete[] (x);(x)=NULL;}}
#endif // !SAFE_DELETE_ARRAY

#ifdef EXPORT_API
#define FORWARD_API __declspec(dllexport)
#else 
#define FORWARD_API
#endif

namespace forward
{
	const i32 NUM_THREADS = 1;
	const f32 Infinity = FLT_MAX;
}
