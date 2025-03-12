#pragma once
#include <string>
#include <unordered_map>

extern "C" __declspec(dllexport) int Forward_Read_MaterialX(const char* file, std::string& outVS, std::string& outPS,
	std::unordered_map<std::string, std::string>& paramsPS);