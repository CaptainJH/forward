//--------------------------------------------------------------------------------
#include "SwapChainConfig.h"
#include "render/Device.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
SwapChainConfig::SwapChainConfig(const Device* pRender)
{
	if (pRender->GetRendererAPI() == RendererAPI::DirectX11)
		SetDefaultsDX11();
	else if (pRender->GetRendererAPI() == RendererAPI::DirectX12)
		SetDefaultsDX12();
}
//--------------------------------------------------------------------------------
SwapChainConfig::~SwapChainConfig()
{
}
//--------------------------------------------------------------------------------
void SwapChainConfig::SetDefaultsDX11()
{
	// Set the state to the default configuration.  These are the D3D11 default
	// values as well.

	m_State.BufferDesc.Width = 1;
	m_State.BufferDesc.Height = 1;
    m_State.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	m_State.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    m_State.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	m_State.BufferDesc.RefreshRate.Numerator = 60;
	m_State.BufferDesc.RefreshRate.Denominator = 1;

	m_State.SampleDesc.Count = 1;
	m_State.SampleDesc.Quality = 0;

	m_State.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_State.BufferCount = 2;
	m_State.OutputWindow = 0;
	m_State.Windowed = true;
	m_State.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// If you are creating a full screen swap chain, you may want to include the 
	// DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH flag in the flags below, which will
	// override the default DXGI behavior.  See here for more details:
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ee417025%28v=vs.85%29.aspx

	m_State.Flags = 0; 
}
//--------------------------------------------------------------------------------
void SwapChainConfig::SetDefaultsDX12()
{
	// Set the state to the default configuration.  These are the D3D11 default
	// values as well.

	m_State.BufferDesc.Width = 1;
	m_State.BufferDesc.Height = 1;
	m_State.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_State.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_State.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	m_State.BufferDesc.RefreshRate.Numerator = 60;
	m_State.BufferDesc.RefreshRate.Denominator = 1;

	m_State.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		//DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	m_State.SampleDesc.Count = 1;
	m_State.SampleDesc.Quality = 0;

	m_State.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_State.BufferCount = 2;
	m_State.OutputWindow = 0;
	m_State.Windowed = true;
	m_State.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
}
//--------------------------------------------------------------------------------
void SwapChainConfig::SetWidth( u32 width )
{
	m_State.BufferDesc.Width = width;
}
//--------------------------------------------------------------------------------
void SwapChainConfig::SetHeight( u32 height )
{
	m_State.BufferDesc.Height = height;
}
//--------------------------------------------------------------------------------
void SwapChainConfig::SetRefreshRateNumerator( u32 numerator )
{
	m_State.BufferDesc.RefreshRate.Numerator = numerator;
}
//--------------------------------------------------------------------------------
void SwapChainConfig::SetRefreshRateDenominator( u32 denominator )
{
	m_State.BufferDesc.RefreshRate.Denominator = denominator;
}
//--------------------------------------------------------------------------------
void SwapChainConfig::SetFormat( DXGI_FORMAT Format )
{
	m_State.BufferDesc.Format = Format;
}
//--------------------------------------------------------------------------------
void SwapChainConfig::SetScanlineOrder( DXGI_MODE_SCANLINE_ORDER ScanlineOrdering )
{
	m_State.BufferDesc.ScanlineOrdering = ScanlineOrdering;
}
//--------------------------------------------------------------------------------
void SwapChainConfig::SetScaling( DXGI_MODE_SCALING Scaling )
{
	m_State.BufferDesc.Scaling = Scaling;
}
//--------------------------------------------------------------------------------
void SwapChainConfig::SetBufferDesc( DXGI_MODE_DESC BufferDesc )
{
	m_State.BufferDesc = BufferDesc;
}
//--------------------------------------------------------------------------------
void SwapChainConfig::SetSampleDesc( DXGI_SAMPLE_DESC SampleDesc )
{
	m_State.SampleDesc = SampleDesc;
}
//--------------------------------------------------------------------------------
void SwapChainConfig::SetBufferUsage( DXGI_USAGE BufferUsage )
{
	m_State.BufferUsage = BufferUsage;
}
//--------------------------------------------------------------------------------
void SwapChainConfig::SetBufferCount( u32 BufferCount )
{
	m_State.BufferCount = BufferCount;
}
//--------------------------------------------------------------------------------
void SwapChainConfig::SetOutputWindow( HWND OutputWindow )
{
	m_State.OutputWindow = OutputWindow;
}
//--------------------------------------------------------------------------------
void SwapChainConfig::SetWindowed( bool Windowed )
{
	m_State.Windowed = Windowed;
}
//--------------------------------------------------------------------------------
void SwapChainConfig::SetSwapEffect( DXGI_SWAP_EFFECT SwapEffect )
{
	m_State.SwapEffect = SwapEffect;
}
//--------------------------------------------------------------------------------
void SwapChainConfig::SetFlags( u32 Flags )
{
	m_State.Flags = Flags;
}
//--------------------------------------------------------------------------------
DXGI_SWAP_CHAIN_DESC& SwapChainConfig::GetSwapChainDesc()
{
	return( m_State );
}
//--------------------------------------------------------------------------------
u32 SwapChainConfig::GetWidth() const
{
	return m_State.BufferDesc.Width;
}
//--------------------------------------------------------------------------------
u32 SwapChainConfig::GetHeight() const
{
	return m_State.BufferDesc.Height;
}