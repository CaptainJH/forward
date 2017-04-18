//--------------------------------------------------------------------------------
#include "IndexBufferDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
IndexBufferDX11::IndexBufferDX11( Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer )
{
	m_pBuffer = pBuffer;
}
//--------------------------------------------------------------------------------
IndexBufferDX11::~IndexBufferDX11()
{
	// Buffer is released in the BufferDX11 destructor
}
//--------------------------------------------------------------------------------
ResourceType IndexBufferDX11::GetType()
{
	return( RT_INDEXBUFFER );
}
//--------------------------------------------------------------------------------
void IndexBufferDX11::SetIndexSize( i32 size )
{
	m_iIndexSize = size;
}
//--------------------------------------------------------------------------------
void IndexBufferDX11::SetIndexCount( i32 count )
{
	m_iIndexCount = count;
}
//--------------------------------------------------------------------------------