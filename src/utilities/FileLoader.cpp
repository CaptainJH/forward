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

	auto Size = hlslCode.size();
	m_pData = new char[Size];
	memset(m_pData, 0, Size);
	memcpy(m_pData, hlslCode.c_str(), Size);
	
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
unsigned int FileLoader::GetDataSize()
{
	return( m_uiSize );
}
//--------------------------------------------------------------------------------