//--------------------------------------------------------------------------------
// RendererDX11
//
//--------------------------------------------------------------------------------
#ifndef RendererDX11_h
#define RendererDX11_h
//--------------------------------------------------------------------------------
#include "PCH.h"

#include "Vector2f.h"
#include "Vector3f.h"
#include "Vector4f.h"
#include "Matrix4f.h"

#include "ResourceSystem\ResourceProxyDX11.h"
#include "Pipeline\PipelineManagerDX11.h"
#include "Pipeline\Stages\ProgrammableStages\ShaderProgram\ShaderDX11.h"
#include "render.h"
#include "dx11Util.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class BufferDX11;
	class VertexBufferDX11;
	class IndexBufferDX11;
	class ConstantBufferDX11;
	class StructuredBufferDX11;
	class ByteAddressBufferDX11;
	class IndirectArgsBufferDX11;
	class Texture1dDX11;
	class Texture2dDX11;
	class Texture3dDX11;
	class ResourceDX11;
	class SwapChainDX11;
	class ViewPortDX11;

	class BufferConfigDX11;
	class Texture1dConfigDX11;
	class Texture2dConfigDX11;
	class Texture3dConfigDX11;
	class SwapChainConfigDX11;

	class GeometryDX11;

	class ShaderResourceViewDX11;
	class RenderTargetViewDX11;
	class DepthStencilViewDX11;
	class UnorderedAccessViewDX11;

	class BlendStateConfigDX11;
	class DepthStencilStateConfigDX11;
	class RasterizerStateConfigDX11;
	class SamplerStateConfigDX11;


	class RendererDX11 : public Renderer
	{
	public:
		RendererDX11();
		virtual ~RendererDX11();

		// Access to the renderer.  There should only be a single instance
		// of the renderer at any given time.

		static RendererDX11* Get();

		// Provide the feature level of the current machine.  This can be
		// called before or after the device has been created.

		D3D_FEATURE_LEVEL GetAvailableFeatureLevel( D3D_DRIVER_TYPE DriverType );
		D3D_FEATURE_LEVEL GetCurrentFeatureLevel();

		// Provide an estimate of the available video memory.

		u64 GetAvailableVideoMemory();

		// Renderer initialization and shutdown methods.  These methods
		// obtain and release all of the hardware specific resources that
		// are used during rendering.

		bool Initialize( D3D_DRIVER_TYPE DriverType, D3D_FEATURE_LEVEL FeatureLevel );
		void Shutdown();

		// These methods provide rendering frame control.  They are closely
		// related to the API for sequencing rendering batches.

		void Present( HWND hWnd = 0, i32 SwapChain = -1, u32 SyncInterval = 0, u32 PresentFlags = 0 );

		// Allow the application to create swap chains

		i32 CreateSwapChain( SwapChainConfigDX11* pConfig );// ResourceManagerDX11

		// The create buffer method provides a way to create any of the buffer
		// structures - vertex, index, and constant buffers.  These all utilize the
		// same ID3D11Buffer interface, and only vary with a flag in the description.

		ResourcePtr CreateVertexBuffer( BufferConfigDX11* pConfig, Subresource* pData );
		ResourcePtr CreateIndexBuffer( BufferConfigDX11* pConfig,  D3D11_SUBRESOURCE_DATA* pData );
		ResourcePtr CreateConstantBuffer( BufferConfigDX11* pConfig,  D3D11_SUBRESOURCE_DATA* pData, bool bAutoUpdate = true );
		ResourcePtr CreateStructuredBuffer( BufferConfigDX11* pConfig,  D3D11_SUBRESOURCE_DATA* pData );
		ResourcePtr CreateByteAddressBuffer( BufferConfigDX11* pConfig,  D3D11_SUBRESOURCE_DATA* pData );
		ResourcePtr CreateIndirectArgsBuffer( BufferConfigDX11* pConfig, D3D11_SUBRESOURCE_DATA* pData );

		ResourcePtr CreateTexture1D( Texture1dConfigDX11* pConfig, Subresource* pData );
		ResourcePtr CreateTexture2D( Texture2dConfigDX11* pConfig, Subresource* pData );
		ResourcePtr CreateTexture3D( Texture3dConfigDX11* pConfig, Subresource* pData );

		void DeleteResource( ResourcePtr ptr );
		void DeleteResource( i32 index );

		// The resources created in the above function calls can only be accessed by
		// the rendering pipeline when they are bound with resource views.  The following
		// functions provide the interface for creating the views, and returns an index
		// for the application to use during rendering setup.

		i32 CreateShaderResourceView( i32 ResourceID, D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc );
		i32 CreateRenderTargetView( i32 ResourceID, D3D11_RENDER_TARGET_VIEW_DESC* pDesc );
		i32 CreateDepthStencilView( i32 ResourceID, D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc );
		i32 CreateUnorderedAccessView( i32 ResourceID, D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc );

		// The input layout is required to exactly match the input signature of the first
		// programmable stage that will receive the input assembler output.  This method is
		// provided for creating an input layout and later indexing the value when setting
		// the appropriate vertex shader.

		i32 CreateInputLayout( std::vector<D3D11_INPUT_ELEMENT_DESC>& elements, i32 ShaderID  );

		// The individual states available in the D3D11 pipeline can be created here.  They
		// are referred to by index after being created.

		i32 CreateBlendState( BlendStateConfigDX11* pConfig );
		i32 CreateDepthStencilState( DepthStencilStateConfigDX11* pConfig );
		i32 CreateRasterizerState( RasterizerStateConfigDX11* pConfig );
		i32 CreateSamplerState( SamplerStateConfigDX11* pConfig );
		i32 CreateViewPort(const u32 width, const u32 height);


		// Each programmable shader stage can be loaded from file, and stored in a list for
		// later use.  Either an application can directly set these values or a render effect
		// can encapsulate the entire pipeline configuration.

		i32 LoadShader( ShaderType type, const std::wstring& filename, const std::wstring& function,
			const std::wstring& model, bool enablelogging = true );

        i32 LoadShader( ShaderType type, const std::wstring& filename, const std::wstring& function,
            const std::wstring& model, const D3D_SHADER_MACRO* pDefines, bool enablelogging = true );
		
		ResourcePtr GetSwapChainResource( i32 ID );

		// This is an interim solution to get the resolution of the current
		// adapter output resolution.

		Vector2f GetDesktopResolution();

		ResourcePtr LoadTexture( std::wstring filename, bool sRGB = false );
		
		// NOTE: This method is likely to be removed in the near future, since there should not
		//       be any situations where there is a raw texture pointer being given to the renderer!
		ResourcePtr LoadTexture( ID3D11Texture2D* pTexture );

		// These methods are used to take an existing texture resource and to resize it.  This
		// will recreate the texture and its associated views with the exact same configuration
		// except that the size will be updated.

		void ResizeTexture( ResourcePtr texture, u32 width, u32 height );
		void ResizeTextureSRV( i32 RID, i32 SRVID, u32 width, u32 height );
		void ResizeTextureRTV( i32 RID, i32 RTVID, u32 width, u32 height );
		void ResizeTextureDSV( i32 RID, i32 DSVID, u32 width, u32 height );
		void ResizeTextureUAV( i32 RID, i32 UAVID, u32 width, u32 height );
		void ResizeSwapChain( i32 ID, u32 width, u32 height );
		void ResizeViewport( i32 ID, u32 width, u32 height );

		// Provide access to the pipeline states.
		BlendStateComPtr								GetBlendState( i32 index );
		DepthStencilStateComPtr							GetDepthState( i32 index );
		RasterizerStateComPtr							GetRasterizerState( i32 index );
		const ViewPortDX11&								GetViewPort( i32 index );

		InputLayoutComPtr								GetInputLayout( i32 index );
		SamplerStateComPtr								GetSamplerState( i32 index );

		ShaderDX11*										GetShader( i32 index );

		// This method is here for allowing easy integration with other libraries
		// which require access to the device.  Do not use this interface to create 
		// objects unless those objects are then registered with this renderer class!!!
		// NOTE: This method is likely to be removed in the near future!

		ID3D11Device* GetDevice();

	protected:

		// The main API interfaces used in the renderer.
		Microsoft::WRL::ComPtr<ID3D11Device>	m_pDevice;
		Microsoft::WRL::ComPtr<ID3D11Debug>		m_pDebugger;
		D3D_DRIVER_TYPE							m_driverType;

		// In general, all resources and API objects are housed in expandable arrays
		// wrapper objects.  The position within the array is used to provide fast
		// random access to the renderer clients.

		std::vector<SwapChainDX11*>				m_vSwapChains;

		// Resource allocation containers are stored in an expandable array, which
		// provides fast random access with indices.

		std::vector<ResourceDX11*>				m_vResources;

		// Resource view containers.  These are indexed by the application for
		// the various pipeline binding operations.

		std::vector<ShaderResourceViewDX11>		m_vShaderResourceViews;
		std::vector<RenderTargetViewDX11>		m_vRenderTargetViews;
		std::vector<DepthStencilViewDX11>		m_vDepthStencilViews;
		std::vector<UnorderedAccessViewDX11>	m_vUnorderedAccessViews;

		// The shader programs are stored in an expandable array of their base classes.

		std::vector<ShaderDX11*>				m_vShaders;
		
		// These states are stored as shared pointers to the object.  This is the direction
		// that the renderer is heading in - eventually references to the objects will be
		// returned instead of indices.  This will eliminate issues with creating and
		// destroying many resources, and allow the renderer clients to have greater access
		// the objects without querying the renderer.

		std::vector<BlendStateComPtr>			m_vBlendStates;
		std::vector<DepthStencilStateComPtr>	m_vDepthStencilStates;
		std::vector<RasterizerStateComPtr>		m_vRasterizerStates;

		std::vector<InputLayoutComPtr>			m_vInputLayouts;
		std::vector<SamplerStateComPtr>			m_vSamplerStates;
		std::vector<ViewPortDX11>				m_vViewPorts;

	public:
		PipelineManagerDX11*					pImmPipeline;

		ResourceDX11*				GetResourceByIndex( i32 index );

		// Texture resource accessors
		
		Texture1dDX11*				GetTexture1DByIndex( i32 rid );
		Texture2dDX11*				GetTexture2DByIndex( i32 rid );
		Texture3dDX11*				GetTexture3DByIndex( i32 rid );
		SwapChainDX11*				GetSwapChainByIndex( i32 sid );

		// Buffer resource accessors

		BufferDX11*					GetGenericBufferByIndex( i32 rid );
		ConstantBufferDX11*			GetConstantBufferByIndex( i32 rid );
		VertexBufferDX11*			GetVertexBufferByIndex( i32 rid );
		IndexBufferDX11*			GetIndexBufferByIndex( i32 rid );
		ByteAddressBufferDX11*		GetByteAddressBufferByIndex( i32 rid );
		IndirectArgsBufferDX11*		GetIndirectArgsBufferByIndex( i32 rid );
		StructuredBufferDX11*		GetStructuredBufferByIndex( i32 rid );

		// Resource view accessors

		RenderTargetViewDX11&		GetRenderTargetViewByIndex( i32 rid );
		DepthStencilViewDX11&		GetDepthStencilViewByIndex( i32 rid );
		ShaderResourceViewDX11&		GetShaderResourceViewByIndex( i32 rid );
		UnorderedAccessViewDX11&	GetUnorderedAccessViewByIndex( i32 rid );

	protected:

		i32							GetUnusedResourceIndex();
		i32							StoreNewResource( ResourceDX11* pResource );

		D3D_FEATURE_LEVEL			m_FeatureLevel;

		friend GeometryDX11;
	};
};

//u32 WINAPI _TaskThreadProc( void* lpParameter );

// Multithreading support objects
//extern HANDLE						g_aThreadHandles[NUM_THREADS];
//extern Glyph3::ThreadPayLoad		g_aPayload[NUM_THREADS];
extern HANDLE						g_aBeginEventHandle[NUM_THREADS];
extern HANDLE						g_aEndEventHandle[NUM_THREADS];

template <class T>
void LogObjectVector( std::vector<T> objects );
template <class T>
void LogObjectPtrVector( std::vector<T> objects );

#endif // RendererDX11_h