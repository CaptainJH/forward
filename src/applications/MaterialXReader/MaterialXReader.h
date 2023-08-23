#pragma once
#ifdef FORWARD_EXPORTS
#define FORWARD_API __declspec(dllexport) 
#else
#define FORWARD_API __declspec(dllimport) 
#endif
#include <string>
#include <unordered_map>

extern "C" FORWARD_API int Forward_Read_MaterialX(const char* file, std::string& outVS, std::string& outPS, 
	std::unordered_map<std::string, std::string>& paramsPS);