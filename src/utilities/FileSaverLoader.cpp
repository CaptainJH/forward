//***************************************************************************************
// FileSaverLoader.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "FileSaver.h"
#include "FileLoader.h"
#include "FileSystem.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
#ifndef _WINDOWS
typedef struct tagBITMAPINFOHEADER
{
	DWORD  biSize;            // size of the structure
	LONG   biWidth;           // image width
	LONG   biHeight;          // image height
	WORD   biPlanes;          // bitplanes
	WORD   biBitCount;        // resolution 
	DWORD  biCompression;     // compression
	DWORD  biSizeImage;       // size of the image
	LONG   biXPelsPerMeter;   // pixels per meter X
	LONG   biYPelsPerMeter;   // pixels per meter Y
	DWORD  biClrUsed;         // colors used
	DWORD  biClrImportant;    // important colors
} BITMAPINFOHEADER;
#endif


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
bool FileLoader::Open(const std::wstring& filename)
{
	if (!FileSystem::getSingleton().FileExists(filename))
		return false;

	// Close the current file if one is open.
	Close();

	std::ifstream shaderFile(filename);
	std::string hlslCode((std::istreambuf_iterator<char>(shaderFile)),
		std::istreambuf_iterator<char>());

	m_uiSize = static_cast<u32>(hlslCode.length());
	m_pData = new char[m_uiSize];
	memset(m_pData, 0, m_uiSize);
	memcpy(m_pData, hlslCode.c_str(), m_uiSize);

	return(true);
}
//--------------------------------------------------------------------------------
bool FileLoader::Close()
{
	SAFE_DELETE_ARRAY(m_pData);
	m_uiSize = 0;

	return(true);
}
//--------------------------------------------------------------------------------
char* FileLoader::GetDataPtr()
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
	//std::wstring filepath = FileSystem::getSingleton().GetSavedFolder() + filename;
	std::wstring filepath = L"C:\\Users\\heqij\\Documents\\GitHub\\forward\\Saved\\" + filename;
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
	info.biCompression = BI_RGB;
	info.biSizeImage = 0;
	info.biXPelsPerMeter = 0x0ec4;
	info.biYPelsPerMeter = 0x0ec4;
	info.biClrUsed = 0;
	info.biClrImportant = 0;

	HANDLE file = CreateFile(filepath.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (NULL == file)
	{
		CloseHandle(file);
		return false;
	}

	unsigned long bwritten;
	if (WriteFile(file, &bmfh, sizeof(BITMAPFILEHEADER),
		&bwritten, NULL) == false)
	{
		CloseHandle(file);
		return false;
	}

	if (WriteFile(file, &info, sizeof(BITMAPINFOHEADER),
		&bwritten, NULL) == false)
	{
		CloseHandle(file);
		return false;
	}

	if (WriteFile(file, pData, paddedsize, &bwritten, NULL) == false)
	{
		CloseHandle(file);
		return false;
	}

	CloseHandle(file);
	return true;
}