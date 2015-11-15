//--------------------------------------------------------------------------------
#include "FileLoader.h"
#include "FileSystem.h"
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
bool FileLoader::Open( const std::wstring& filename )
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
	
	return( true );
}
//--------------------------------------------------------------------------------
bool FileLoader::Close( )
{
	SAFE_DELETE_ARRAY(m_pData);
	m_uiSize = 0;

	return( true );
}
//--------------------------------------------------------------------------------
char* FileLoader::GetDataPtr()
{
	return( m_pData );
}
//--------------------------------------------------------------------------------
u32 FileLoader::GetDataSize()
{
	return( m_uiSize );
}
//--------------------------------------------------------------------------------