#include "forwardDX11Dll.h"

#include "utilities/FileSystem.h"
#include <iostream>

namespace
{
	forward::FileSystem sFileSystem;
}

using namespace forward;

void Forward_Constructor()
{
	std::cout << "===Forward_Constructor===" << std::endl;
}

void Forward_Destructor()
{
	std::cout << "===Forward_Destructor===" << std::endl;
}

const wchar_t* FileSystem_GetFontFolder()
{
	static std::wstring ret;

	ret = forward::FileSystem::getSingletonPtr()->GetFontFolder();
	return ret.c_str();
}