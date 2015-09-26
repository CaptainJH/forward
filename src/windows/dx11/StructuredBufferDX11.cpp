//--------------------------------------------------------------------------------
#include "StructuredBufferDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
StructuredBufferDX11::StructuredBufferDX11( Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer )
{
	m_pBuffer = pBuffer;
}
//--------------------------------------------------------------------------------
StructuredBufferDX11::~StructuredBufferDX11()
{
	// Buffer is released in the BufferDX11 destructor
}
//--------------------------------------------------------------------------------
ResourceType StructuredBufferDX11::GetType()
{
	return( RT_STRUCTUREDBUFFER );
}
//--------------------------------------------------------------------------------
