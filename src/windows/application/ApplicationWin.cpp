#include "ApplicationWin.h"


//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
// Initialize the ApplicationWin pointer to NULL
ApplicationWin* ApplicationWin::ms_pApplication = NULL;
//--------------------------------------------------------------------------------
ApplicationWin::ApplicationWin() :
	m_bSaveScreenshot( false ),
	m_bLoop( true )
{
	// Record the this pointer to provide access to the WinMain function.

	ms_pApplication = this;

}
//--------------------------------------------------------------------------------
ApplicationWin::~ApplicationWin( )
{

}
//--------------------------------------------------------------------------------
ApplicationWin* ApplicationWin::GetApplication( )
{
	return( ms_pApplication );
}
//--------------------------------------------------------------------------------
bool ApplicationWin::ConfigureCommandLine( LPSTR lpcmdline )
{
	// Default to returning true, but allow sub-classes to override this behavior.
	return( true );
}
//--------------------------------------------------------------------------------
void ApplicationWin::RequestTermination( )
{
	// This triggers the termination of the ApplicationWin
	PostQuitMessage( 0 );
}
//--------------------------------------------------------------------------------
//bool ApplicationWin::HandleEvent( EventPtr pEvent )
//{
//
//
//	return( false );
//}
//--------------------------------------------------------------------------------
void ApplicationWin::MessageLoop()
{
	MSG msg;
	
	while( true )
	{
		while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{ 
			if ( msg.message == WM_QUIT )
			{
				return;
			}

			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		// Call the overloaded ApplicationWin update function.
		Update();
		TakeScreenShot();
	}
}
//--------------------------------------------------------------------------------
void ApplicationWin::BeforeRegisterWindowClass( WNDCLASSEX &wc )
{
	// This function is intended to be overriden in dervived classes
}
//--------------------------------------------------------------------------------
LRESULT ApplicationWin::WindowProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{	

		case WM_CREATE: 
			{
				// Automatically return 0 to allow the window to proceed in the
				// creation process.

				return( 0 );
			} break;

		case WM_PAINT:
			{
				// This message is handled by the default handler to avoid a 
				// repeated sending of the message.  This results in the ability
				// to process all pending messages at once without getting stuck
				// in an eternal loop.
			} break;

		case WM_CLOSE:
			{
				// This message is sent when a window or an ApplicationWin should
				// terminate.
			} break;

		case WM_DESTROY: 
			{
				// This message is sent when a window has been destroyed.
				PostQuitMessage(0);
				return( 0 );
			} break;
    }
    return( DefWindowProc( hwnd, msg, wparam, lparam ) );
}
//--------------------------------------------------------------------------------