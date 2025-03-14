//--------------------------------------------------------------------------------
// This file is a portion of the Hieroglyph 3 Rendering Engine.  It is distributed
// under the MIT License, available in the root of this distribution and
// at the following URL:
//
// http://www.opensource.org/licenses/mit-license.php
//
// Copyright (c) Jason Zink
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
#include "pCH.h"
#include "Log.h"
#include "FileSystem.h"
#include "Utils.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
Log::Log()
{
}
//--------------------------------------------------------------------------------
Log& Log::Get()
{
	static Log log;
	return( log );
}
//--------------------------------------------------------------------------------
bool Log::Open()
{
	std::wstring filename = FileSystem::getSingleton().GetLogFolder() + L"\\Log.txt";
#ifdef MACOS
    auto filepathAnsi = TextHelper::ToAscii(filename);
    AppLog.open(filepathAnsi.c_str());
#else
	AppLog.open( filename.c_str() );
#endif

	Write( L"Log file opened." );

	return( true );
}
//--------------------------------------------------------------------------------
bool Log::Write( const wchar_t *cTextString )
{
	AppLog << cTextString << "\n";
#if _DEBUG
	::OutputDebugStringW( cTextString );
	::OutputDebugStringW( L"\n" );
#endif
	return( true );
}
//--------------------------------------------------------------------------------
bool Log::Write( std::wstring& TextString )
{
	Log::Write( TextString.c_str() );
	AppLog.flush();
	return( true );
}
//--------------------------------------------------------------------------------
bool Log::Close( )
{
	Write( L"Log file closed." );
	AppLog.close();
	return( true );
}
//--------------------------------------------------------------------------------
bool Log::WriteSeparater( )
{
	Write( L"------------------------------------------------------------" );

	return( true );
}
//--------------------------------------------------------------------------------
