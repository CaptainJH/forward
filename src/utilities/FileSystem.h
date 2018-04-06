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
		~FileSystem();

		std::wstring GetLogFolder() const;
		
		std::wstring GetDataFolder() const;
		std::wstring GetModelsFolder() const;
		std::wstring GetScriptsFolder() const;
		std::wstring GetShaderFolder() const;
		std::wstring GetTextureFolder() const;
		std::wstring GetSavedFolder() const;

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

		static std::wstring sDataFolder;
		static std::wstring sModelsSubFolder;
		static std::wstring sScriptsSubFolder;
		static std::wstring sShaderSubFolder;
		static std::wstring sTextureSubFolder;
		static std::wstring sSaved;

		static std::wstring sLogFolder;
	};
};
//--------------------------------------------------------------------------------
#endif // FileSystem_h