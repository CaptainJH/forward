#pragma once

#ifdef _WINDOWS

#include <string>
#include <sstream>
#include <fstream>
#include <array>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>

#include <thread>
#include <chrono>

#include "Types.h"
#include "d3dUtil.h"

#include <wrl.h>
#define NOMINMAX

#define SAFE_RELEASE( x ) {if(x){(x)->Release();(x)=NULL;}}
#define SAFE_DELETE( x ) {if(x){delete (x);(x)=NULL;}}
#define SAFE_DELETE_ARRAY( x ) {if(x){delete[] (x);(x)=NULL;}}

const forward::i32 NUM_THREADS = 1;
const forward::f32 Infinity = FLT_MAX;
const forward::f32 Pi = 3.1415926535f;

#endif