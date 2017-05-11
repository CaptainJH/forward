//--------------------------------------------------------------------------------
// ViewPortDX11
//
//--------------------------------------------------------------------------------
#ifndef ViewPortDX11_h
#define ViewPortDX11_h
//--------------------------------------------------------------------------------
#include <d3d11_2.h>
#include "Vector2f.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class ViewPortDX11
	{
	public:
		ViewPortDX11();
		ViewPortDX11( const u32 width, const u32 height );
		~ViewPortDX11();

		f32 GetWidth() const;
		f32 GetHeight() const;
		Vector2f GetClipSpacePosition( const Vector2f& screen ) const;
		Vector2f GetScreenSpacePosition( const Vector2f& clip ) const;

	protected:
		D3D11_VIEWPORT			m_ViewPort;

		friend class RasterizerStageDX11;
		friend class RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // ViewPortDX11_h
//--------------------------------------------------------------------------------

