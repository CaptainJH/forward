//--------------------------------------------------------------------------------
#include "PCH.h"
#include "FileSystem.h"
#include <chrono>
#include <boost\filesystem.hpp>
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
std::wstring FileSystem::sDataFolder = L"Data/";
std::wstring FileSystem::sModelsSubFolder = L"Models/";
std::wstring FileSystem::sScriptsSubFolder = L"Scripts/";
std::wstring FileSystem::sShaderSubFolder = L"Shaders/";
std::wstring FileSystem::sTextureSubFolder = L"Textures/";

std::wstring FileSystem::sLogFolder = L"Log/";

FileSystem* forward::Singleton<FileSystem>::msSingleton = 0;
//--------------------------------------------------------------------------------
FileSystem::FileSystem()
{
	auto path = boost::filesystem::current_path();
	auto pathStr = path.generic_wstring();
	auto index = pathStr.find(L"forward");
	assert(index < pathStr.length());
	auto indexEnd = pathStr.find_first_of(L'/', index);
	assert(indexEnd < pathStr.length());
	mCWD = pathStr.substr(0, indexEnd + 1);
	sDataFolder = mCWD + sDataFolder;
	sLogFolder = mCWD + sLogFolder;

	auto logPath = boost::filesystem::path(sLogFolder);
	if (!boost::filesystem::exists(logPath))
	{
		boost::filesystem::create_directory(logPath);
	}
}
//--------------------------------------------------------------------------------
FileSystem::~FileSystem()
{
}

std::wstring FileSystem::GetCWD() const
{
	return mCWD;
}

void FileSystem::SetCWD(const std::wstring& cwd)
{
	mCWD = cwd;
}

//--------------------------------------------------------------------------------
std::wstring FileSystem::GetLogFolder() const
{
	return sLogFolder;
}
//--------------------------------------------------------------------------------
std::wstring FileSystem::GetDataFolder() const
{
	return( sDataFolder );
}
//--------------------------------------------------------------------------------
std::wstring FileSystem::GetModelsFolder() const
{
	return( sDataFolder + sModelsSubFolder );
}
//--------------------------------------------------------------------------------
std::wstring FileSystem::GetScriptsFolder() const
{
	return( sDataFolder + sScriptsSubFolder );
}
//--------------------------------------------------------------------------------
std::wstring FileSystem::GetShaderFolder() const
{
	return( sDataFolder + sShaderSubFolder );
}
//--------------------------------------------------------------------------------
std::wstring FileSystem::GetTextureFolder() const
{
	return( sDataFolder + sTextureSubFolder );
}
//--------------------------------------------------------------------------------
void FileSystem::SetDataFolder( const std::wstring& folder )
{
	sDataFolder = folder;
}
//--------------------------------------------------------------------------------
void FileSystem::SetModelsFolder( const std::wstring& folder )
{
	sModelsSubFolder = folder;
}
//--------------------------------------------------------------------------------
void FileSystem::SetScriptsFolder( const std::wstring& folder )
{
	sScriptsSubFolder = folder;
}
//--------------------------------------------------------------------------------
void FileSystem::SetShaderFolder( const std::wstring& folder )
{
	sShaderSubFolder = folder;
}
//--------------------------------------------------------------------------------
void FileSystem::SetTextureFolder( const std::wstring& folder )
{
	sTextureSubFolder = folder;
}
//--------------------------------------------------------------------------------
bool FileSystem::FileExists( const std::wstring& file )
{
	// Check if the file exists, and that it is not a directory

	boost::filesystem::path path(file);
	return boost::filesystem::exists(path) && !boost::filesystem::is_directory(path);
}
//--------------------------------------------------------------------------------
/// return true if file1 is newer, otherwise return false
bool FileSystem::FileIsNewer( const std::wstring& file1, const std::wstring& file2 )
{
	// This method assumes that the existance of the files has already been verified!

	boost::filesystem::path file1path(file1);
	boost::filesystem::path file2path(file2);

	std::time_t stamp1 = boost::filesystem::last_write_time(file1path);
	std::time_t stamp2 = boost::filesystem::last_write_time(file2path);

	auto tp1 = std::chrono::system_clock::from_time_t(stamp1);
	auto tp2 = std::chrono::system_clock::from_time_t(stamp2);

	return tp1 > tp2;
}
//--------------------------------------------------------------------------------