//--------------------------------------------------------------------------------
// SwapChainConfigDX11
//
//--------------------------------------------------------------------------------
#ifndef SwapChainConfigDX11_h
#define SwapChainConfigDX11_h
//--------------------------------------------------------------------------------
#include "Types.h"
#include <dxgi.h>
//--------------------------------------------------------------------------------
namespace forward
{
	class SwapChainConfig
	{
	public:
		SwapChainConfig();
		virtual ~SwapChainConfig();

		void SetDefaults();

		void SetWidth( u32 width );
		void SetHeight( u32 height );
		void SetRefreshRateNumerator( u32 numerator );
		void SetRefreshRateDenominator( u32 denominator );
		void SetFormat( DXGI_FORMAT Format );
		void SetScanlineOrder( DXGI_MODE_SCANLINE_ORDER ScanlineOrdering );
		void SetScaling( DXGI_MODE_SCALING Scaling );


		void SetBufferDesc( DXGI_MODE_DESC BufferDesc );
		void SetSampleDesc( DXGI_SAMPLE_DESC SampleDesc );
		void SetBufferUsage( DXGI_USAGE BufferUsage );
		void SetBufferCount( u32 BufferCount );
		void SetOutputWindow( HWND OutputWindow );
		void SetWindowed( bool Windowed );
		void SetSwapEffect( DXGI_SWAP_EFFECT SwapEffect );
		void SetFlags( u32 Flags );

		DXGI_SWAP_CHAIN_DESC& GetSwapChainDesc();

	protected:
		DXGI_SWAP_CHAIN_DESC 		m_State;
	};
};
//--------------------------------------------------------------------------------
#endif // SwapChainConfigDX11_h
//--------------------------------------------------------------------------------
