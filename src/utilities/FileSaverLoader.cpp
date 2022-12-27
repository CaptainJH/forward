//***************************************************************************************
// FileSaverLoader.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "FileSaver.h"
#include "FileLoader.h"
#include "FileSystem.h"
#ifdef _WINDOWS
#include <windows.h>
#endif
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------


FileLoader::FileLoader()
	: m_pData(nullptr)
	, m_uiSize(0)
{
}
//--------------------------------------------------------------------------------
FileLoader::~FileLoader()
{
	Close();
}
//--------------------------------------------------------------------------------
EResult FileLoader::Open(const std::wstring& filename)
{
	if (!FileSystem::getSingleton().FileExists(filename))
		return EResult::E_RESULT_FILE_DOES_NOT_EXIST;

	// Close the current file if one is open.
	Close();

#ifdef MACOS
    auto filepathAnsi = TextHelper::ToAscii(filename);
    std::ifstream shaderFile(filepathAnsi);
#else
	std::ifstream shaderFile(filename);
#endif
	std::string hlslCode((std::istreambuf_iterator<char>(shaderFile)),
		std::istreambuf_iterator<char>());

	m_uiSize = static_cast<u32>(hlslCode.length() + 1);
	m_pData = new char[m_uiSize];
	memset(m_pData, 0, m_uiSize);
	memcpy(m_pData, hlslCode.c_str(), m_uiSize);

	return EResult::E_RESULT_NO_ERROR;
}
//--------------------------------------------------------------------------------
bool FileLoader::Close()
{
	SAFE_DELETE_ARRAY(m_pData);
	m_uiSize = 0;

	return(true);
}
//--------------------------------------------------------------------------------
i8* FileLoader::GetDataPtr()
{
	return(m_pData);
}
//--------------------------------------------------------------------------------
u32 FileLoader::GetDataSize()
{
	return(m_uiSize);
}
//--------------------------------------------------------------------------------


FileSaver::FileSaver()
{

}

FileSaver::~FileSaver()
{

}

bool FileSaver::SaveAsBMP(const std::wstring& filename, const u8* pData, u32 width, u32 height) const
{
	std::wstring filepath = FileSystem::getSingleton().GetSavedFolder() + filename;
	auto paddedsize = width * height * 4;

	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER info;
	memset(&bmfh, 0, sizeof(BITMAPFILEHEADER));
	memset(&info, 0, sizeof(BITMAPINFOHEADER));

	bmfh.bfType = 0x4d42;       // 0x4d42 = 'BM'
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + paddedsize;
	bmfh.bfOffBits = 0x36;

	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biWidth = width;
	info.biHeight = static_cast<i32>(height) * -1; // reverse image
	info.biPlanes = 1;
	info.biBitCount = 32;
    info.biCompression = 0;//BI_RGB;
	info.biSizeImage = 0;
	info.biXPelsPerMeter = 0x0ec4;
	info.biYPelsPerMeter = 0x0ec4;
	info.biClrUsed = 0;
	info.biClrImportant = 0;

#ifdef MACOS
    auto filepathAnsi = TextHelper::ToAscii(filepath);
    std::ofstream file(filepathAnsi, std::ofstream::binary);
#else
	std::ofstream file(filepath, std::ofstream::binary);
#endif
	file.write((const i8*)&bmfh, sizeof(BITMAPFILEHEADER));
	file.write((const i8*)&info, sizeof(BITMAPINFOHEADER));
	file.write((const i8*)pData, paddedsize);
	file.close();

	return true;
}
