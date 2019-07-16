//--------------------------------------------------------------------------------
#include "PCH.h"
#include "FileSystem.h"
#include <chrono>
#ifdef WINDOWS
#include <filesystem>
using namespace std::experimental;
#else
#include <unistd.h>
#include <os/log.h>
#include "Utils.h"
#endif

//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
template<>
FileSystem* forward::Singleton<FileSystem>::msSingleton = 0;
//--------------------------------------------------------------------------------
FileSystem::FileSystem()
{
	mDataFolder = L"Data/";
	mModelsSubFolder = L"Models/";
	mScriptsSubFolder = L"Scripts/";
	mShaderSubFolder = L"Shaders/";
	mTextureSubFolder = L"Textures/";
	mSavedFolder = L"Saved/";
	mFontFolder = L"src/render/Text/";

	mLogFolder = L"Log/";

#ifdef WINDOWS
    auto path = filesystem::current_path();
    auto pathStr = path.generic_wstring();
#else
    char cwdBuffer[128] = {0};
    getcwd(cwdBuffer, 128);
    os_log(OS_LOG_DEFAULT, "from os_log : %s", cwdBuffer);
    auto pathStr = TextHelper::ToUnicode(std::string(cwdBuffer));
#endif
	auto index = pathStr.find(L"forward");
	assert(index < pathStr.length());
	auto indexEnd = pathStr.find_first_of(L'/', index);
	mCWD = pathStr;
	if(indexEnd < pathStr.length())
		mCWD = pathStr.substr(0, indexEnd + 1);
	else
		mCWD += L'/';
	mDataFolder = mCWD + mDataFolder;
	mLogFolder = mCWD + mLogFolder;
	mSavedFolder = mCWD + mSavedFolder;
	mFontFolder = mCWD + mFontFolder;

#ifdef WINDOWS
	auto logPath = filesystem::path(mLogFolder);
	if (!filesystem::exists(logPath))
	{
		filesystem::create_directory(logPath);
	}
	auto savedPath = filesystem::path(mSavedFolder);
	if (!filesystem::exists(savedPath))
	{
		filesystem::create_directory(savedPath);
	}
#endif
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
	return mLogFolder;
}
//--------------------------------------------------------------------------------
std::wstring FileSystem::GetSavedFolder() const
{
	return mSavedFolder;
}
//--------------------------------------------------------------------------------
std::wstring FileSystem::GetFontFolder() const
{
	return mFontFolder;
}
//--------------------------------------------------------------------------------
std::wstring FileSystem::GetDataFolder() const
{
	return( mDataFolder );
}
//--------------------------------------------------------------------------------
std::wstring FileSystem::GetModelsFolder() const
{
	return( mDataFolder + mModelsSubFolder );
}
//--------------------------------------------------------------------------------
std::wstring FileSystem::GetScriptsFolder() const
{
	return( mDataFolder + mScriptsSubFolder );
}
//--------------------------------------------------------------------------------
std::wstring FileSystem::GetShaderFolder() const
{
	return( mDataFolder + mShaderSubFolder );
}
//--------------------------------------------------------------------------------
std::wstring FileSystem::GetTextureFolder() const
{
	return( mDataFolder + mTextureSubFolder );
}
//--------------------------------------------------------------------------------
void FileSystem::SetDataFolder( const std::wstring& folder )
{
	mDataFolder = folder;
}
//--------------------------------------------------------------------------------
void FileSystem::SetModelsFolder( const std::wstring& folder )
{
	mModelsSubFolder = folder;
}
//--------------------------------------------------------------------------------
void FileSystem::SetScriptsFolder( const std::wstring& folder )
{
	mScriptsSubFolder = folder;
}
//--------------------------------------------------------------------------------
void FileSystem::SetShaderFolder( const std::wstring& folder )
{
	mShaderSubFolder = folder;
}
//--------------------------------------------------------------------------------
void FileSystem::SetTextureFolder( const std::wstring& folder )
{
	mTextureSubFolder = folder;
}
//--------------------------------------------------------------------------------
bool FileSystem::FileExists( const std::wstring& file )
{
	// Check if the file exists, and that it is not a directory
#ifdef WINDOWS
	filesystem::path path(file);
	return filesystem::exists(path) && !filesystem::is_directory(path);
#else
    return true;
#endif
}
//--------------------------------------------------------------------------------
/// return true if file1 is newer, otherwise return false
bool FileSystem::FileIsNewer( const std::wstring& file1, const std::wstring& file2 )
{
	// This method assumes that the existance of the files has already been verified!
#ifdef WINDOWS
	filesystem::path file1path(file1);
	filesystem::path file2path(file2);

	std::chrono::system_clock::time_point stamp1 = filesystem::last_write_time(file1path);
	std::chrono::system_clock::time_point stamp2 = filesystem::last_write_time(file2path);

	return stamp1 > stamp2;
#else
    assert(false && "Not implemented yet!");
    return true;
#endif
}
//--------------------------------------------------------------------------------
