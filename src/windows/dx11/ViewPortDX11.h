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
		ViewPortDX11( D3D11_VIEWPORT viewport );
		~ViewPortDX11();

		float GetWidth() const;
		float GetHeight() const;
		Vector2f GetClipSpacePosition( const Vector2f& screen ) const;
		Vector2f GetScreenSpacePosition( const Vector2f& clip ) const;

	protected:
		D3D11_VIEWPORT			m_ViewPort;

		//friend RasterizerStageDX11;
		//friend RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // ViewPortDX11_h
//--------------------------------------------------------------------------------

