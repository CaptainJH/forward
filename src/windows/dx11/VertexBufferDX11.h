//--------------------------------------------------------------------------------
// VertexBufferDX11 
//
//--------------------------------------------------------------------------------
#ifndef VertexBufferDX11_h
#define VertexBufferDX11_h
//--------------------------------------------------------------------------------
#include "BufferDX11.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class VertexBufferDX11 : public BufferDX11
	{
	public:
		VertexBufferDX11( Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer );
		virtual ~VertexBufferDX11();

		virtual ResourceType				GetType();

		void						SetVertexSize( int size );
		void						SetVertexCount( int count );

	protected:
		int							m_iVertexSize;
		int							m_iVertexCount;

		//friend RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // VertexBufferDX11_h
