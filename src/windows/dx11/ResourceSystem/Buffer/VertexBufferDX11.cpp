//--------------------------------------------------------------------------------
#include "VertexBufferDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
VertexBufferDX11::VertexBufferDX11( Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer, VertexBufferConfig* pConfig )
{
	m_pBuffer = pBuffer;

	if (pConfig)
	{
		m_pResourceConfig = new VertexBufferConfig(*pConfig);
	}
}
//--------------------------------------------------------------------------------
VertexBufferDX11::~VertexBufferDX11()
{
	// Buffer is released in the BufferDX11 destructor
}
//--------------------------------------------------------------------------------
ResourceType VertexBufferDX11::GetType()
{
	return( RT_VERTEXBUFFER );
}
//--------------------------------------------------------------------------------
void VertexBufferDX11::SetVertexSize( i32 size )
{
	m_iVertexSize = size;
}
//--------------------------------------------------------------------------------
void VertexBufferDX11::SetVertexCount( i32 count )
{
	m_iVertexCount = count;
}
//--------------------------------------------------------------------------------