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

	typedef enum DXGI_FORMAT
	{
		DXGI_FORMAT_UNKNOWN = 0,
		DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
		DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
		DXGI_FORMAT_R32G32B32A32_UINT = 3,
		DXGI_FORMAT_R32G32B32A32_SINT = 4,
		DXGI_FORMAT_R32G32B32_TYPELESS = 5,
		DXGI_FORMAT_R32G32B32_FLOAT = 6,
		DXGI_FORMAT_R32G32B32_UINT = 7,
		DXGI_FORMAT_R32G32B32_SINT = 8,
		DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
		DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
		DXGI_FORMAT_R16G16B16A16_UNORM = 11,
		DXGI_FORMAT_R16G16B16A16_UINT = 12,
		DXGI_FORMAT_R16G16B16A16_SNORM = 13,
		DXGI_FORMAT_R16G16B16A16_SINT = 14,
		DXGI_FORMAT_R32G32_TYPELESS = 15,
		DXGI_FORMAT_R32G32_FLOAT = 16,
		DXGI_FORMAT_R32G32_UINT = 17,
		DXGI_FORMAT_R32G32_SINT = 18,
		DXGI_FORMAT_R32G8X24_TYPELESS = 19,
		DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
		DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
		DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
		DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
		DXGI_FORMAT_R10G10B10A2_UNORM = 24,
		DXGI_FORMAT_R10G10B10A2_UINT = 25,
		DXGI_FORMAT_R11G11B10_FLOAT = 26,
		DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
		DXGI_FORMAT_R8G8B8A8_UNORM = 28,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
		DXGI_FORMAT_R8G8B8A8_UINT = 30,
		DXGI_FORMAT_R8G8B8A8_SNORM = 31,
		DXGI_FORMAT_R8G8B8A8_SINT = 32,
		DXGI_FORMAT_R16G16_TYPELESS = 33,
		DXGI_FORMAT_R16G16_FLOAT = 34,
		DXGI_FORMAT_R16G16_UNORM = 35,
		DXGI_FORMAT_R16G16_UINT = 36,
		DXGI_FORMAT_R16G16_SNORM = 37,
		DXGI_FORMAT_R16G16_SINT = 38,
		DXGI_FORMAT_R32_TYPELESS = 39,
		DXGI_FORMAT_D32_FLOAT = 40,
		DXGI_FORMAT_R32_FLOAT = 41,
		DXGI_FORMAT_R32_UINT = 42,
		DXGI_FORMAT_R32_SINT = 43,
		DXGI_FORMAT_R24G8_TYPELESS = 44,
		DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
		DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
		DXGI_FORMAT_R8G8_TYPELESS = 48,
		DXGI_FORMAT_R8G8_UNORM = 49,
		DXGI_FORMAT_R8G8_UINT = 50,
		DXGI_FORMAT_R8G8_SNORM = 51,
		DXGI_FORMAT_R8G8_SINT = 52,
		DXGI_FORMAT_R16_TYPELESS = 53,
		DXGI_FORMAT_R16_FLOAT = 54,
		DXGI_FORMAT_D16_UNORM = 55,
		DXGI_FORMAT_R16_UNORM = 56,
		DXGI_FORMAT_R16_UINT = 57,
		DXGI_FORMAT_R16_SNORM = 58,
		DXGI_FORMAT_R16_SINT = 59,
		DXGI_FORMAT_R8_TYPELESS = 60,
		DXGI_FORMAT_R8_UNORM = 61,
		DXGI_FORMAT_R8_UINT = 62,
		DXGI_FORMAT_R8_SNORM = 63,
		DXGI_FORMAT_R8_SINT = 64,
		DXGI_FORMAT_A8_UNORM = 65,
		DXGI_FORMAT_R1_UNORM = 66,
		DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
		DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
		DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
		DXGI_FORMAT_BC1_TYPELESS = 70,
		DXGI_FORMAT_BC1_UNORM = 71,
		DXGI_FORMAT_BC1_UNORM_SRGB = 72,
		DXGI_FORMAT_BC2_TYPELESS = 73,
		DXGI_FORMAT_BC2_UNORM = 74,
		DXGI_FORMAT_BC2_UNORM_SRGB = 75,
		DXGI_FORMAT_BC3_TYPELESS = 76,
		DXGI_FORMAT_BC3_UNORM = 77,
		DXGI_FORMAT_BC3_UNORM_SRGB = 78,
		DXGI_FORMAT_BC4_TYPELESS = 79,
		DXGI_FORMAT_BC4_UNORM = 80,
		DXGI_FORMAT_BC4_SNORM = 81,
		DXGI_FORMAT_BC5_TYPELESS = 82,
		DXGI_FORMAT_BC5_UNORM = 83,
		DXGI_FORMAT_BC5_SNORM = 84,
		DXGI_FORMAT_B5G6R5_UNORM = 85,
		DXGI_FORMAT_B5G5R5A1_UNORM = 86,
		DXGI_FORMAT_B8G8R8A8_UNORM = 87,
		DXGI_FORMAT_B8G8R8X8_UNORM = 88,
		DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
		DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
		DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
		DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
		DXGI_FORMAT_BC6H_TYPELESS = 94,
		DXGI_FORMAT_BC6H_UF16 = 95,
		DXGI_FORMAT_BC6H_SF16 = 96,
		DXGI_FORMAT_BC7_TYPELESS = 97,
		DXGI_FORMAT_BC7_UNORM = 98,
		DXGI_FORMAT_BC7_UNORM_SRGB = 99,
		DXGI_FORMAT_AYUV = 100,
		DXGI_FORMAT_Y410 = 101,
		DXGI_FORMAT_Y416 = 102,
		DXGI_FORMAT_NV12 = 103,
		DXGI_FORMAT_P010 = 104,
		DXGI_FORMAT_P016 = 105,
		DXGI_FORMAT_420_OPAQUE = 106,
		DXGI_FORMAT_YUY2 = 107,
		DXGI_FORMAT_Y210 = 108,
		DXGI_FORMAT_Y216 = 109,
		DXGI_FORMAT_NV11 = 110,
		DXGI_FORMAT_AI44 = 111,
		DXGI_FORMAT_IA44 = 112,
		DXGI_FORMAT_P8 = 113,
		DXGI_FORMAT_A8P8 = 114,
		DXGI_FORMAT_B4G4R4A4_UNORM = 115,

		DXGI_FORMAT_P208 = 130,
		DXGI_FORMAT_V208 = 131,
		DXGI_FORMAT_V408 = 132,


		DXGI_FORMAT_FORCE_UINT = 0xffffffff
	} DXGI_FORMAT;


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

	struct DDS_HEADER_DXT10
	{
		DXGI_FORMAT     dxgiFormat;
		uint32_t        resourceDimension;
		uint32_t        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
		uint32_t        arraySize;
		uint32_t        miscFlags2;
	};

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
	ScopedHandle hFile(safe_handle(CreateFile2(filename.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		OPEN_EXISTING, nullptr)));

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
		if (FileSize.LowPart < (sizeof(DDS_HEADER) + sizeof(uint32_t) + sizeof(DDS_HEADER_DXT10)))
		{
			return E_RESULT_UNKNOWN_ERROR;
		}

		bDXT10Header = true;
	}

	// setup the pointers in the process request
	memcpy(&m_header, hdr, sizeof(m_header));
	ptrdiff_t offset = sizeof(uint32_t) + sizeof(DDS_HEADER)
		+ (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0);
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