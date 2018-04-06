//--------------------------------------------------------------------------------
#include "PCH.h"
#include "FileSystem.h"
#include <chrono>
#include <filesystem>
//--------------------------------------------------------------------------------
using namespace forward;
using namespace std::experimental;
//--------------------------------------------------------------------------------
std::wstring FileSystem::sDataFolder = L"Data/";
std::wstring FileSystem::sModelsSubFolder = L"Models/";
std::wstring FileSystem::sScriptsSubFolder = L"Scripts/";
std::wstring FileSystem::sShaderSubFolder = L"Shaders/";
std::wstring FileSystem::sTextureSubFolder = L"Textures/";
std::wstring FileSystem::sSaved = L"Saved/";

std::wstring FileSystem::sLogFolder = L"Log/";

FileSystem* forward::Singleton<FileSystem>::msSingleton = 0;
//--------------------------------------------------------------------------------
FileSystem::FileSystem()
{
	auto path = filesystem::current_path();
	auto pathStr = path.generic_wstring();
	auto index = pathStr.find(L"forward");
	assert(index < pathStr.length());
	auto indexEnd = pathStr.find_first_of(L'/', index);
	mCWD = pathStr;
	if(indexEnd < pathStr.length())
		mCWD = pathStr.substr(0, indexEnd + 1);
	else
		mCWD += L'/';
	sDataFolder = mCWD + sDataFolder;
	sLogFolder = mCWD + sLogFolder;

	auto logPath = filesystem::path(sLogFolder);
	if (!filesystem::exists(logPath))
	{
		filesystem::create_directory(logPath);
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
std::wstring FileSystem::GetSavedFolder() const
{
	return sSaved;
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

	filesystem::path path(file);
	return filesystem::exists(path) && !filesystem::is_directory(path);
}
//--------------------------------------------------------------------------------
/// return true if file1 is newer, otherwise return false
bool FileSystem::FileIsNewer( const std::wstring& file1, const std::wstring& file2 )
{
	// This method assumes that the existance of the files has already been verified!

	filesystem::path file1path(file1);
	filesystem::path file2path(file2);

	std::chrono::system_clock::time_point stamp1 = filesystem::last_write_time(file1path);
	std::chrono::system_clock::time_point stamp2 = filesystem::last_write_time(file2path);

	return stamp1 > stamp2;
}
//--------------------------------------------------------------------------------