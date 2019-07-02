//--------------------------------------------------------------------------------
// FileSystem
//
// This class uses the Monostate pattern to provide data paths for the 
// application.  The paths are all specified as static variables, but are accessed
// with non-static members.  Just make sure you set these paths before doing 
// anything else in the application, including any of the initialization functions
// if you need specialized locations.
//
// Please note that the Monostate pattern requires that the state be set *outside*
// of any code that is executed pre-main.  This includes constructors of 
// statically declared objects, such as the application object.  So if you are
// going to modify the paths, do so outside of the application constructor!
//--------------------------------------------------------------------------------
#ifndef FileSystem_h
#define FileSystem_h
//--------------------------------------------------------------------------------
#include <string>
#include "Singleton.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class FileSystem : public Singleton<FileSystem>
	{
	public:
		FileSystem();
		FileSystem(const char* forwardPath);
		~FileSystem();

		std::wstring GetLogFolder() const;
		
		std::wstring GetDataFolder() const;
		std::wstring GetModelsFolder() const;
		std::wstring GetScriptsFolder() const;
		std::wstring GetShaderFolder() const;
		std::wstring GetTextureFolder() const;
		std::wstring GetSavedFolder() const;
		std::wstring GetFontFolder() const;

		std::wstring GetCWD() const;
		void SetCWD(const std::wstring& cwd);

		void SetDataFolder( const std::wstring& folder );
		void SetModelsFolder( const std::wstring& folder );
		void SetScriptsFolder( const std::wstring& folder );
		void SetShaderFolder( const std::wstring& folder );
		void SetTextureFolder( const std::wstring& folder );

		bool FileExists( const std::wstring& file );
		bool FileIsNewer( const std::wstring& file1, const std::wstring& file2 );

	private:
		std::wstring mCWD;

		std::wstring mDataFolder;
		std::wstring mModelsSubFolder;
		std::wstring mScriptsSubFolder;
		std::wstring mShaderSubFolder;
		std::wstring mTextureSubFolder;
		std::wstring mSavedFolder;
		std::wstring mFontFolder;

		std::wstring mLogFolder;
	};
};
//--------------------------------------------------------------------------------
#endif // FileSystem_h