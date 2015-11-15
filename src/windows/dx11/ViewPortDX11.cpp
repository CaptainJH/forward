//--------------------------------------------------------------------------------
#include "ViewPortDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
ViewPortDX11::ViewPortDX11()
{
	m_ViewPort.Width = 1.0f;
	m_ViewPort.Height = 1.0f;
	m_ViewPort.TopLeftX = 0.0f;
	m_ViewPort.TopLeftY = 0.0f;
	m_ViewPort.MinDepth = 0.0f;
	m_ViewPort.MaxDepth = 1.0f;
}
//--------------------------------------------------------------------------------
ViewPortDX11::ViewPortDX11( D3D11_VIEWPORT viewport )
{
	m_ViewPort = viewport;
}
//--------------------------------------------------------------------------------
ViewPortDX11::~ViewPortDX11()
{
}
//--------------------------------------------------------------------------------
f32 ViewPortDX11::GetWidth() const
{
	return( m_ViewPort.Width );
}
//--------------------------------------------------------------------------------
f32 ViewPortDX11::GetHeight() const
{
	return( m_ViewPort.Height );
}
//--------------------------------------------------------------------------------
Vector2f ViewPortDX11::GetClipSpacePosition( const Vector2f& screen ) const
{
	return(	Vector2f( ( ( screen.x / m_ViewPort.Width ) - 0.5f ) * 2.0f,
					 -( ( screen.y / m_ViewPort.Height ) - 0.5f ) * 2.0f ) );
}
//--------------------------------------------------------------------------------
Vector2f ViewPortDX11::GetScreenSpacePosition( const Vector2f& clip ) const
{
	return( Vector2f( (  clip.x / 2.0f + 0.5f ) * m_ViewPort.Width,
					  ( -clip.y / 2.0f + 0.5f ) * m_ViewPort.Height ) );
}
//--------------------------------------------------------------------------------
