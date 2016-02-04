#include "PCH.h"
#include "CallStackTraceGenerator.h"

using namespace forward;

#ifdef _WINDOWS
#pragma warning(disable : 4091)
#include <DbgHelp.h>
#include <WinBase.h>
#pragma comment(lib, "Dbghelp.lib")

std::vector<std::string> CallStackTraceGenerator::GetTrace()
{
	std::vector<std::string> result;

    typedef USHORT (WINAPI *CaptureStackBackTraceType)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG);
    CaptureStackBackTraceType func = (CaptureStackBackTraceType)(GetProcAddress(LoadLibrary(L"kernel32.dll"), "RtlCaptureStackBackTrace"));

    if(func == NULL)
        return result; // WOE 29.SEP.2010

    // Quote from Microsoft Documentation:
    // ## Windows Server 2003 and Windows XP:  
    // ## The sum of the FramesToSkip and FramesToCapture parameters must be less than 63.
    const int kMaxCallers = 62; 

    void         * callers_stack[ kMaxCallers ];
    unsigned short frames;
    SYMBOL_INFO  * symbol;
    HANDLE         process;
    process = GetCurrentProcess();
    SymInitialize( process, NULL, TRUE );
    frames               = (func)( 0, kMaxCallers, callers_stack, NULL );
    symbol               = ( SYMBOL_INFO * )calloc( sizeof( SYMBOL_INFO ) + 256 * sizeof( char ), 1 );
    symbol->MaxNameLen   = 255;
    symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

    for( unsigned int i = 1;  i < frames;  i++ )
    {
		std::stringstream out;
        SymFromAddr( process, ( DWORD64 )( callers_stack[ i ] ), 0, symbol );
        out << "*** " << i << ": " << callers_stack[i] << " " << symbol->Name << " - 0x" << symbol->Address << std::endl;
		result.push_back(out.str());
    }

    free( symbol );

	return result;
}
#else

#endif

std::string CallStackTraceGenerator::str(const std::vector<std::string>& stack)
{
	std::string str;
	for (auto it = stack.rbegin(); it != stack.rend(); ++it)
	{
		str += *it;
	}

	return str;
}