//--------------------------------------------------------------------------------
#include "IndirectArgsBufferDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
IndirectArgsBufferDX11::IndirectArgsBufferDX11( Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer )
{
	m_pBuffer = pBuffer;
}
//--------------------------------------------------------------------------------
IndirectArgsBufferDX11::~IndirectArgsBufferDX11()
{
	// m_pBuffer is released in the BufferDX11 destructor
}
//--------------------------------------------------------------------------------
ResourceType IndirectArgsBufferDX11::GetType()
{
	return( RT_INDIRECTARGSBUFFER );
}
//--------------------------------------------------------------------------------
