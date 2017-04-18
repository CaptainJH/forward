
//--------------------------------------------------------------------------------
#include "BufferDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
BufferDX11::BufferDX11()
{
	ZeroMemory( &m_DesiredDesc, sizeof( D3D11_BUFFER_DESC ) );
	ZeroMemory( &m_ActualDesc, sizeof( D3D11_BUFFER_DESC ) );
}
//--------------------------------------------------------------------------------
BufferDX11::~BufferDX11()
{
}
//--------------------------------------------------------------------------------
D3D11_BUFFER_DESC BufferDX11::GetActualDescription()
{
	ZeroMemory( &m_ActualDesc, sizeof( D3D11_BUFFER_DESC ) );

	if ( m_pBuffer )
		m_pBuffer->GetDesc( &m_ActualDesc );

	return( m_ActualDesc );
}
//--------------------------------------------------------------------------------
D3D11_BUFFER_DESC BufferDX11::GetDesiredDescription()
{
	return( m_DesiredDesc );
}
//--------------------------------------------------------------------------------
void BufferDX11::SetDesiredDescription( D3D11_BUFFER_DESC desc )
{
	m_DesiredDesc = desc;
}
//--------------------------------------------------------------------------------
u32 BufferDX11::GetByteWidth()
{
	D3D11_BUFFER_DESC description = GetActualDescription();

	return( description.ByteWidth );
}
//--------------------------------------------------------------------------------
D3D11_USAGE BufferDX11::GetUsage()
{
	D3D11_BUFFER_DESC description = GetActualDescription();

	return( description.Usage );
}
//--------------------------------------------------------------------------------
u32 BufferDX11::GetBindFlags()
{
	D3D11_BUFFER_DESC description = GetActualDescription();

	return( description.BindFlags );
}
//--------------------------------------------------------------------------------
u32 BufferDX11::GetCPUAccessFlags()
{
	D3D11_BUFFER_DESC description = GetActualDescription();

	return( description.CPUAccessFlags );
}
//--------------------------------------------------------------------------------
u32 BufferDX11::GetMiscFlags()
{
	D3D11_BUFFER_DESC description = GetActualDescription();

	return( description.MiscFlags );
}
//--------------------------------------------------------------------------------
u32 BufferDX11::GetStructureByteStride()
{
	D3D11_BUFFER_DESC description = GetActualDescription();

	return( description.StructureByteStride );
}
//--------------------------------------------------------------------------------
void BufferDX11::SetByteWidth( u32 width )
{
	m_DesiredDesc.ByteWidth = width;
}
//--------------------------------------------------------------------------------
void BufferDX11::SetUsage( D3D11_USAGE usage )
{
	m_DesiredDesc.Usage = usage;
}
//--------------------------------------------------------------------------------
void BufferDX11::SetBindFlags( u32 flags )
{
	m_DesiredDesc.BindFlags = flags;
}
//--------------------------------------------------------------------------------
void BufferDX11::SetCPUAccessFlags( u32 flags )
{
	m_DesiredDesc.CPUAccessFlags = flags;
}
//--------------------------------------------------------------------------------
void BufferDX11::SetMiscFlags( u32 flags )
{
	m_DesiredDesc.MiscFlags = flags;
}
//--------------------------------------------------------------------------------
void BufferDX11::SetStructureByteStride( u32 stride )
{
	m_DesiredDesc.StructureByteStride = stride;
}
//--------------------------------------------------------------------------------
void* BufferDX11::Map()
{
	return( 0 );
}
//--------------------------------------------------------------------------------
void BufferDX11::UnMap()
{
}
//--------------------------------------------------------------------------------
ID3D11Resource*	BufferDX11::GetResource()
{
	return( m_pBuffer.Get() );
}
//--------------------------------------------------------------------------------
u32 BufferDX11::GetEvictionPriority()
{
	u32 priority = 0;

	if ( m_pBuffer )
		priority = m_pBuffer->GetEvictionPriority();

	return( priority );
}
//--------------------------------------------------------------------------------
void BufferDX11::SetEvictionPriority( u32 EvictionPriority )
{
	if ( m_pBuffer )
		m_pBuffer->SetEvictionPriority( EvictionPriority );
}
//--------------------------------------------------------------------------------