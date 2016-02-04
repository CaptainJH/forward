#pragma once
#include <vector>
#include <string>
#include <sstream>

namespace forward
{
	class CallStackTraceGenerator
	{
	private:
		// this is a pure utils class 
		// cannot be instantiated 
		//
		CallStackTraceGenerator() = delete;
		CallStackTraceGenerator(const CallStackTraceGenerator&) = delete;
		CallStackTraceGenerator& operator=(const CallStackTraceGenerator&) = delete;
		~CallStackTraceGenerator() = delete;

	public:
		static std::vector<std::string> GetTrace();
		static std::string str(const std::vector<std::string>& stack);
	};
}