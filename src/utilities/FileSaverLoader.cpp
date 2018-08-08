//***************************************************************************************
// FileSaverLoader.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "FileSaver.h"
#include "FileLoader.h"
#include "FileSystem.h"
#include <windows.h>
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
namespace
{
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

	//--------------------------------------------------------------------------------------
	// Macros
	//--------------------------------------------------------------------------------------
#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |       \
                ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))
#endif /* defined(MAKEFOURCC) */

//--------------------------------------------------------------------------------------
// DDS file structure definitions
//
// See DDS.h in the 'Texconv' sample and the 'DirectXTex' library
//--------------------------------------------------------------------------------------
#pragma pack(push,1)

	const uint32_t DDS_MAGIC = 0x20534444; // "DDS "

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA

#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

	enum DDS_MISC_FLAGS2
	{
		DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L,
	};

	const u32 DDS_HEADER_DXT10_SIZE = 20;

	struct handle_closer { void operator()(HANDLE h) { if (h) CloseHandle(h); } };

	typedef public std::unique_ptr<void, handle_closer> ScopedHandle;

	inline HANDLE safe_handle(HANDLE h) { return (h == INVALID_HANDLE_VALUE) ? 0 : h; }

#pragma pack(pop)
}


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

	std::ifstream shaderFile(filename);
	std::string hlslCode((std::istreambuf_iterator<char>(shaderFile)),
		std::istreambuf_iterator<char>());

	m_uiSize = static_cast<u32>(hlslCode.length());
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

DDSFileLoader::DDSFileLoader()
	: m_bitData(nullptr)
	, m_bitSize(0)
{
	memset(&m_header, 0, sizeof(m_header));
}

DDSFileLoader::~DDSFileLoader()
{

}

EResult DDSFileLoader::Open(const std::wstring& filename)
{
	// open the file
	//ScopedHandle hFile(safe_handle(CreateFile2(filename.c_str(),
	//	GENERIC_READ,
	//	FILE_SHARE_READ,
	//	OPEN_EXISTING, nullptr)));

	ScopedHandle hFile(safe_handle(CreateFileW(filename.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr)));

	if (!hFile)
	{
		E_RESULT_UNKNOWN_ERROR;
	}

	// Get the file size
	LARGE_INTEGER FileSize = { 0 };

	FILE_STANDARD_INFO fileInfo;
	if (!GetFileInformationByHandleEx(hFile.get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)))
	{
		return E_RESULT_UNKNOWN_ERROR;
	}
	FileSize = fileInfo.EndOfFile;

	// File is too big for 32-bit allocation, so reject read
	if (FileSize.HighPart > 0)
	{
		return E_RESULT_UNKNOWN_ERROR;
	}

	// Need at least enough data to fill the header and magic number to be a valid DDS
	if (FileSize.LowPart < (sizeof(DDS_HEADER) + sizeof(uint32_t)))
	{
		return E_RESULT_UNKNOWN_ERROR;
	}

	// create enough space for the file data
	m_pData = new (std::nothrow) i8[FileSize.LowPart];
	if (!m_pData)
	{
		return E_RESULT_OUT_OF_MEMORY;
	}

	// read the data in
	DWORD BytesRead = 0;
	if (!ReadFile(hFile.get(),
		m_pData,
		FileSize.LowPart,
		&BytesRead,
		nullptr))
	{
		return E_RESULT_UNKNOWN_ERROR;
	}

	if (BytesRead < FileSize.LowPart)
	{
		return E_RESULT_UNKNOWN_ERROR;
	}

	// DDS files always start with the same magic number ("DDS ")
	uint32_t dwMagicNumber = *(const uint32_t*)(m_pData);
	if (dwMagicNumber != DDS_MAGIC)
	{
		return E_RESULT_UNKNOWN_ERROR;
	}

	auto hdr = reinterpret_cast<DDS_HEADER*>(m_pData + sizeof(uint32_t));

	// Verify header to validate DDS file
	if (hdr->size != sizeof(DDS_HEADER) ||
		hdr->ddspf.size != sizeof(DDS_PIXELFORMAT))
	{
		return E_RESULT_UNKNOWN_ERROR;
	}

	// Check for DX10 extension
	bool bDXT10Header = false;
	if ((hdr->ddspf.flags & DDS_FOURCC) &&
		(MAKEFOURCC('D', 'X', '1', '0') == hdr->ddspf.fourCC))
	{
		// Must be long enough for both headers and magic value
		if (FileSize.LowPart < (sizeof(DDS_HEADER) + sizeof(uint32_t) + DDS_HEADER_DXT10_SIZE))
		{
			return E_RESULT_UNKNOWN_ERROR;
		}

		bDXT10Header = true;
	}

	// setup the pointers in the process request
	memcpy(&m_header, hdr, sizeof(m_header));
	ptrdiff_t offset = sizeof(uint32_t) + sizeof(DDS_HEADER)
		+ (bDXT10Header ? DDS_HEADER_DXT10_SIZE : 0);
	m_bitData = m_pData + offset;
	m_bitSize = FileSize.LowPart - offset;

	return EResult::E_RESULT_NO_ERROR;
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
	info.biCompression = BI_RGB;
	info.biSizeImage = 0;
	info.biXPelsPerMeter = 0x0ec4;
	info.biYPelsPerMeter = 0x0ec4;
	info.biClrUsed = 0;
	info.biClrImportant = 0;

	std::ofstream file(filepath, std::ofstream::binary);
	file.write((const i8*)&bmfh, sizeof(BITMAPFILEHEADER));
	file.write((const i8*)&info, sizeof(BITMAPINFOHEADER));
	file.write((const i8*)pData, paddedsize);
	file.close();

	return true;
}