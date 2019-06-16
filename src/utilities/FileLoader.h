//--------------------------------------------------------------------------------
// FileLoader
//
// The FileLoader class is a helper class for loading simple files.  The contents
// of the desired file are loaded with the Open() method, and an internally memory
// is allocated to hold the entire contents of the file.  This memory can then be
// accessed by the user, and it is freed when the class instance is destroyed or
// the Close() method is called.
//
// Because of this, it is convenient to declare an instance of this class on the 
// stack.  Then once it goes out of scope, the memory is automatically freed, and
// you don't have to worry about releasing it.
//
// This class should also be compatible with Win7 classic, Win8 desktop, and
// Win8 Metro style applications.
//--------------------------------------------------------------------------------
#ifndef FileLoader_h
#define FileLoader_h
//--------------------------------------------------------------------------------
#include "PCH.h"
#include "Utils.h"
#include "DataFormat.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class FileLoader 
	{
	public:
		FileLoader();
		virtual ~FileLoader();

		virtual EResult Open( const std::wstring& filename );
		bool Close( );

		i8* GetDataPtr();
		u32 GetDataSize();

	protected:
		i8*			m_pData;
		u32	m_uiSize;

	};

#ifdef _WINDOWS
	struct DDS_PIXELFORMAT
	{
		u32    size;
		u32    flags;
		u32    fourCC;
		u32    RGBBitCount;
		u32    RBitMask;
		u32    GBitMask;
		u32    BBitMask;
		u32    ABitMask;
	};

	struct DDS_HEADER
	{
		u32        size;
		u32        flags;
		u32        height;
		u32        width;
		u32        pitchOrLinearSize;
		u32        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
		u32        mipMapCount;
		u32        reserved1[11];
		DDS_PIXELFORMAT ddspf;
		u32        caps;
		u32        caps2;
		u32        caps3;
		u32        caps4;
		u32        reserved2;
	};

	class DDSFileLoader : public FileLoader
	{
	public:
		DDSFileLoader();
		virtual ~DDSFileLoader();

		EResult Open(const std::wstring& filename) override;
		u32 GetImageContentSize() const;
		i8* GetImageContentDataPtr() const;
		u32 GetImageWidth() const;
		u32 GetImageHeight() const;
		u32 GetMipCount() const;
		EResult GetTextureDimension(u32& dimension, bool& isCube) const;
		DataFormatType GetImageFormat() const;

	protected:
		DDS_HEADER*	m_header;
		u32			m_contentSize;
		i8*			m_contentDataPtr;
	};
#endif
};
//--------------------------------------------------------------------------------
#endif // FileLoader_h
//--------------------------------------------------------------------------------
