#pragma once

#ifdef _WINDOWS

#include <d3d11_2.h>

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <memory>


#include <wrl.h>

#define SAFE_RELEASE( x ) {if(x){(x)->Release();(x)=NULL;}}
#define SAFE_DELETE( x ) {if(x){delete (x);(x)=NULL;}}
#define SAFE_DELETE_ARRAY( x ) {if(x){delete[] (x);(x)=NULL;}}

#include "Types.h"

#endif