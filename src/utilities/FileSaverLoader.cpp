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

	struct handle_closer { void operator()(HANDLE h) { if (h) CloseHandle(h); } };

	typedef public std::unique_ptr<void, handle_closer> ScopedHandle;

	inline HANDLE safe_handle(HANDLE h) { return (h == INVALID_HANDLE_VALUE) ? 0 : h; }

	struct DDS_HEADER_DXT10
	{
		DataFormatType     dxgiFormat;
		uint32_t        resourceDimension;
		uint32_t        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
		uint32_t        arraySize;
		uint32_t        miscFlags2;
	};

#pragma pack(pop)
}

//--------------------------------------------------------------------------------------
#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a )

static DataFormatType GetDXGIFormat(const DDS_PIXELFORMAT& ddpf)
{
	if (ddpf.flags & DDS_RGB)
	{
		// Note that sRGB formats are written using the "DX10" extended header

		switch (ddpf.RGBBitCount)
		{
		case 32:
			if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
			{
				return DF_R8G8B8A8_UNORM;
			}

			if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
			{
				return DF_B8G8R8A8_UNORM;
			}

			if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
			{
				return DF_B8G8R8X8_UNORM;
			}

			// No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000) aka D3DFMT_X8B8G8R8

			// Note that many common DDS reader/writers (including D3DX) swap the
			// the RED/BLUE masks for 10:10:10:2 formats. We assume
			// below that the 'backwards' header mask is being used since it is most
			// likely written by D3DX. The more robust solution is to use the 'DX10'
			// header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

			// For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
			if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
			{
				return DF_R10G10B10A2_UNORM;
			}

			// No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10

			if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
			{
				return DF_R16G16_UNORM;
			}

			if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
			{
				// Only 32-bit color channel format in D3D9 was R32F
				return DF_R32_FLOAT; // D3DX writes this out as a FourCC of 114
			}
			break;

		case 24:
			// No 24bpp DXGI formats aka D3DFMT_R8G8B8
			break;

		case 16:
			if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
			{
				return DF_B5G5R5A1_UNORM;
			}
			if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
			{
				return DF_B5G6R5_UNORM;
			}

			// No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0x0000) aka D3DFMT_X1R5G5B5

			if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
			{
				return DF_B4G4R4A4_UNORM;
			}

			// No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0x0000) aka D3DFMT_X4R4G4B4

			// No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
			break;
		}
	}
	else if (ddpf.flags & DDS_LUMINANCE)
	{
		if (8 == ddpf.RGBBitCount)
		{
			if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
			{
				return DF_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
			}

			// No DXGI format maps to ISBITMASK(0x0f,0x00,0x00,0xf0) aka D3DFMT_A4L4
		}

		if (16 == ddpf.RGBBitCount)
		{
			if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
			{
				return DF_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
			}
			if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
			{
				return DF_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
			}
		}
	}
	else if (ddpf.flags & DDS_ALPHA)
	{
		if (8 == ddpf.RGBBitCount)
		{
			return DF_A8_UNORM;
		}
	}
	else if (ddpf.flags & DDS_FOURCC)
	{
		if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.fourCC)
		{
			return DF_BC1_UNORM;
		}
		if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.fourCC)
		{
			return DF_BC2_UNORM;
		}
		if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.fourCC)
		{
			return DF_BC3_UNORM;
		}

		// While pre-multiplied alpha isn't directly supported by the DXGI formats,
		// they are basically the same as these BC formats so they can be mapped
		if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.fourCC)
		{
			return DF_BC2_UNORM;
		}
		if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.fourCC)
		{
			return DF_BC3_UNORM;
		}

		if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.fourCC)
		{
			return DF_BC4_UNORM;
		}
		if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.fourCC)
		{
			return DF_BC4_UNORM;
		}
		if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.fourCC)
		{
			return DF_BC4_SNORM;
		}

		if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.fourCC)
		{
			return DF_BC5_UNORM;
		}
		if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.fourCC)
		{
			return DF_BC5_UNORM;
		}
		if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.fourCC)
		{
			return DF_BC5_SNORM;
		}

		// BC6H and BC7 are written using the "DX10" extended header

		if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.fourCC)
		{
			return DF_R8G8_B8G8_UNORM;
		}
		if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.fourCC)
		{
			return DF_G8R8_G8B8_UNORM;
		}

		if (MAKEFOURCC('Y', 'U', 'Y', '2') == ddpf.fourCC)
		{
			return DF_YUY2;
		}

		// Check for D3DFORMAT enums being set here
		switch (ddpf.fourCC)
		{
		case 36: // D3DFMT_A16B16G16R16
			return DF_R16G16B16A16_UNORM;

		case 110: // D3DFMT_Q16W16V16U16
			return DF_R16G16B16A16_SNORM;

		case 111: // D3DFMT_R16F
			return DF_R16_FLOAT;

		case 112: // D3DFMT_G16R16F
			return DF_R16G16_FLOAT;

		case 113: // D3DFMT_A16B16G16R16F
			return DF_R16G16B16A16_FLOAT;

		case 114: // D3DFMT_R32F
			return DF_R32_FLOAT;

		case 115: // D3DFMT_G32R32F
			return DF_R32G32_FLOAT;

		case 116: // D3DFMT_A32B32G32R32F
			return DF_R32G32B32A32_FLOAT;
		}
	}

	return DF_UNKNOWN;
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
	: m_contentDataPtr(nullptr)
	, m_header(nullptr)
	, m_contentSize(0)
{
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

	//ScopedHandle hFile(safe_handle(CreateFileW(filename.c_str(),
	//	GENERIC_READ,
	//	FILE_SHARE_READ,
	//	nullptr,
	//	OPEN_EXISTING,
	//	FILE_ATTRIBUTE_NORMAL,
	//	nullptr)));

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
	if (!ReadFile(hFile.get(), m_pData, FileSize.LowPart, &BytesRead, nullptr))
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
	m_header = hdr;
	ptrdiff_t offset = sizeof(uint32_t) + sizeof(DDS_HEADER)
		+ (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0);
	m_contentDataPtr = m_pData + offset;
	m_contentSize = static_cast<u32>(FileSize.LowPart - offset);

	return EResult::E_RESULT_NO_ERROR;
}

u32 DDSFileLoader::GetImageContentSize() const
{
	return m_contentSize;
}

i8* DDSFileLoader::GetImageContentDataPtr() const
{
	return m_contentDataPtr;
}

u32 DDSFileLoader::GetImageHeight() const
{
	assert(m_header);
	return m_header->height;
}

u32 DDSFileLoader::GetImageWidth() const
{
	assert(m_header);
	return m_header->width;
}

DataFormatType DDSFileLoader::GetImageFormat() const
{
	DataFormatType format = DataFormatType::DF_UNKNOWN;

	assert(m_header);
	auto mipCount = m_header->mipMapCount;
	if (0 == mipCount)
	{
		mipCount = 1;
	}

	if ((m_header->ddspf.flags & DDS_FOURCC) &&
		(MAKEFOURCC('D', 'X', '1', '0') == m_header->ddspf.fourCC))
	{
		auto d3d10ext = reinterpret_cast<const DDS_HEADER_DXT10*>((const i8*)(m_pData + sizeof(DDS_HEADER)));

		auto arraySize = d3d10ext->arraySize;
		assert(arraySize != 0);

		switch (d3d10ext->dxgiFormat)
		{
		case DF_AI44:
		case DF_IA44:
		case DF_P8:
		case DF_A8P8:
			return format;

		default:
			if (DataFormat::GetNumBytesPerStruct(d3d10ext->dxgiFormat) == 0)
			{
				return format;
			}
		}

		format = d3d10ext->dxgiFormat;
	}
	else
	{
		format = GetDXGIFormat(m_header->ddspf);

		if (format != DataFormatType::DF_UNKNOWN)
		{
			assert(DataFormat::GetNumBytesPerStruct(format) != 0);
			return format;
		}
	}
	return DataFormatType::DF_UNKNOWN;
}

u32 DDSFileLoader::GetMipCount() const
{
	assert(m_header);
	if (m_header->mipMapCount == 0)
	{
		return 1;
	}

	return m_header->mipMapCount;
}

EResult DDSFileLoader::GetTextureDimension(u32& dimension, bool& isCube) const
{
	assert(m_header);
	u32 arraySize = 1;
	isCube = false;
	auto height = GetImageHeight();

	if ((m_header->ddspf.flags & DDS_FOURCC) &&
		(MAKEFOURCC('D', 'X', '1', '0') == m_header->ddspf.fourCC))
	{
		auto d3d10ext = reinterpret_cast<const DDS_HEADER_DXT10*>((const i8*)m_header + sizeof(DDS_HEADER));

		arraySize = d3d10ext->arraySize;
		if (arraySize == 0)
		{
			return E_RESULT_INVALID_VALUE;
		}

		switch (d3d10ext->resourceDimension)
		{
		case 2://D3D11_RESOURCE_DIMENSION_TEXTURE1D:
			// D3DX writes 1D textures with a fixed Height of 1
			if ((m_header->flags & DDS_HEIGHT) && height != 1)
			{
				return E_RESULT_INVALID_VALUE;
			}
			break;

		case 3://D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			if (d3d10ext->miscFlag & 4/*D3D11_RESOURCE_MISC_TEXTURECUBE*/)
			{
				arraySize *= 6;
				isCube = true;
			}
			break;

		case 4://D3D11_RESOURCE_DIMENSION_TEXTURE3D:
			if (!(m_header->flags & DDS_HEADER_FLAGS_VOLUME))
			{
				return E_RESULT_INVALID_VALUE;
			}

			if (arraySize > 1)
			{
				return E_RESULT_INVALID_VALUE;
			}
			break;

		default:
			return E_RESULT_UNKNOWN_ERROR;
		}

		dimension = d3d10ext->resourceDimension - 1;
	}
	else
	{
		if (m_header->flags & DDS_HEADER_FLAGS_VOLUME)
		{
			dimension = 3;
		}
		else
		{
			if (m_header->caps2 & DDS_CUBEMAP)
			{
				// We require all six faces to be defined
				if ((m_header->caps2 & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
				{
					return E_RESULT_INVALID_VALUE;
				}

				arraySize = 6;
				isCube = true;
			}

			dimension = 2;

			// Note there's no way for a legacy Direct3D 9 DDS to express a '1D' texture
		}
	}

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