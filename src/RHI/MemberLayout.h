#pragma once
#include "PCH.h"

namespace forward
{
	struct MemberLayout
	{
		std::string name;
		u32			offset;
		u32			numElements;
	};

	typedef std::vector<MemberLayout> BufferLayout;
}