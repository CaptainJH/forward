//***************************************************************************************
// FileSaver.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "pCH.h"

namespace forward
{
	class FileSaver
	{
	public:
		FileSaver();
		~FileSaver();

		bool SaveAsBMP(const std::wstring& filename, const u8* pData, u32 width, u32 height) const;
	};
}