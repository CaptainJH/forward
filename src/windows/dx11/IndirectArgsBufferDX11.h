//--------------------------------------------------------------------------------
// IndirectArgsBufferDX11 
//
//--------------------------------------------------------------------------------
#ifndef IndirectArgsBufferDX11_h
#define IndirectArgsBufferDX11_h
//--------------------------------------------------------------------------------
#include "BufferDX11.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class IndirectArgsBufferDX11 : public BufferDX11
	{
	public:
		IndirectArgsBufferDX11( Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer );
		virtual ~IndirectArgsBufferDX11();

		virtual ResourceType				GetType();

	protected:

		//friend RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // IndirectArgsBufferDX11_h
