//--------------------------------------------------------------------------------
// PipelineManagerDX11
// 
//--------------------------------------------------------------------------------
#ifndef PipelineManagerDX11_h
#define PipelineManagerDX11_h
//--------------------------------------------------------------------------------
#include "PCH.h"
#include "dx11/Pipeline/Stages/ProgrammableStages/ShaderStage/VertexShaderStageDX11.h"
#include "dx11/Pipeline/Stages/ProgrammableStages/ShaderStage/HullShaderStageDX11.h"
#include "dx11/Pipeline/Stages/ProgrammableStages/ShaderStage/DomainShaderStageDX11.h"
#include "dx11/Pipeline/Stages/ProgrammableStages/ShaderStage/GeometryShaderStageDX11.h"
#include "dx11/Pipeline/Stages/FixedStages/StreamOutput/StreamOutputStageDX11.h"
#include "dx11/Pipeline/Stages/FixedStages/Rasterizer/RasterizerStageDX11.h"
#include "dx11/Pipeline/Stages/ProgrammableStages/ShaderStage/PixelShaderStageDX11.h"
#include "dx11/Pipeline/Stages/ProgrammableStages/ShaderStage/ComputeShaderStageDX11.h"
#include "dx11/Pipeline/Stages/FixedStages/InputAssembler/InputAssemblerStageDX11.h"
#include "dx11/Pipeline/Stages/FixedStages/OutputMerger/OutputMergerStageDX11.h"
#include "dx11/Pipeline/Executors/GeometryDX11.h"
//#include "RenderEffectDX11.h"
#include "dx11/ResourceSystem/ResourceProxyDX11.h"
#include "dx11/ResourceSystem/ResourceDX11.h"
#include "dx11/dx11Util.h"
//--------------------------------------------------------------------------------
namespace forward
{

	class CommandListDX11;

	typedef Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> UserDefinedAnnotationComPtr;

	class PipelineManagerDX11
	{
	public:
		PipelineManagerDX11();
		~PipelineManagerDX11();

		// Allow the creator to set the context and feature level

		void SetDeviceContext( DeviceContextComPtr pContext, D3D_FEATURE_LEVEL level );

		// All of the 'Bind/Unbind' functions below are used to bind various resources to the
		// pipeline.  Currently only the CS can accept unordered access views.  A method is
		// provided to apply the resource changes as an optimization, which allows for the
		// renderer to cache resource changes between rendering calls if appropriate.

		//void BindConstantBufferParameter( ShaderType type, RenderParameterDX11* pParam, u32 slot, IParameterManager* pParamManager );
		//void BindShaderResourceParameter( ShaderType type, RenderParameterDX11* pParam, u32 slot, IParameterManager* pParamManager );
		//void BindUnorderedAccessParameter( ShaderType type, RenderParameterDX11* pParam, u32 slot, IParameterManager* pParamManager );
		//void BindSamplerStateParameter( ShaderType type, RenderParameterDX11* pParam, u32 slot, IParameterManager* pParamManager );

		//void BindShader( ShaderType type, i32 ID, IParameterManager* pParamManager );


		// The pipeline state is managed through the following methods.  The user must 
		// configure each pipeline stage as they want it, and then utilize these Apply
		// and Clear methods to manage when the state is actually sent to the API.
		//
		// The groups of state are split roughly into the following groups:
		// 1. Input Resources: These determine the input configuration of the pipeline.
		// 2. Pipeline Resources: The state for all processing done inside the pipeline.
		// 3. Render Targets: The state for receiving the output of the pipeline.
		//
		// The changes made to the pipeline resources with the Bind calls are cached and 
		// applied or cleared all at once with these calls.  This reduces the number 
		// of API function calls needed to configure the pipeline.

		void ApplyInputResources();
		void ClearInputResources();
		
		void ApplyPipelineResources();
		void ClearPipelineResources();

		void ClearRenderTargets();	
		void ApplyRenderTargets();
		
		void ClearPipelineState();
		void ClearPipelineSRVs();


		// Pipeline execution calls - these are the methods for executing the 
		// pipeline with the given configuration (supplied by the render effect).
		// With the dispatch call, the same configuration is used except that you
		// specify the dimensions of the thread groups to instantiate.
		
		//void Draw( RenderEffectDX11& effect, GeometryPtr chunk, IParameterManager* pParamManager );
		//void Draw( RenderEffectDX11& effect, ResourcePtr vb, ResourcePtr ib,
		//			i32 inputLayout, D3D11_PRIMITIVE_TOPOLOGY primType,
		//			u32 vertexStride, u32 numIndices, IParameterManager* pParamManager);
  //      void DrawNonIndexed( RenderEffectDX11& effect, ResourcePtr vb, i32 inputLayout, 
  //                  D3D11_PRIMITIVE_TOPOLOGY primType, u32 vertexStride, u32 vertexCount, 
  //                  u32 startVertexLocation, IParameterManager* pParamManager);
		//void DrawInstanced( RenderEffectDX11& effect, GeometryPtr chunk,
		//					ResourcePtr instanceData, u32 instanceDataStride,
		//					u32 numInstances, IParameterManager* pParamManager );
		//void DrawInstanced( RenderEffectDX11& effect, ResourcePtr vb,
		//					D3D11_PRIMITIVE_TOPOLOGY primType, ResourcePtr ib,
		//					i32 inputLayout, u32 vertexStride, u32 numIndices,
		//					ResourcePtr instanceData, u32 instanceDataStride,
		//					u32 numInstances, IParameterManager* pParamManager );

		//void Dispatch( RenderEffectDX11& effect, u32 x, u32 y, u32 z, IParameterManager* pParamManager );

		//void DispatchIndirect( RenderEffectDX11& effect, ResourcePtr args, u32 offset, IParameterManager* pParamManager );
		//void DrawIndirect( RenderEffectDX11& effect, ResourcePtr args, u32 offset, i32 inputLayout,
		//	D3D11_PRIMITIVE_TOPOLOGY primType, u32 vertexStride, IParameterManager* pParamManager);


		// These draw calls are thin wrappers around the basic API draw calls.  They are intended
		// to be used by the PipelineExecutorDX11 subclasses to invoke the pipeline.

		void Draw( u32 VertexCount, u32 StartVertex );
		void DrawIndexed( u32 IndexCount, u32 StartIndex, i32 VertexOffset ); 
		void DrawIndexedInstanced( u32 IndexCountPerInstance, u32 InstanceCount, u32 StartIndexLocation, INT BaseVertexLocation, u32 StartInstanceLocation );
		void DrawInstancedIndirect( ID3D11Buffer* argsBuffer, u32 offset );



		// The pipeline state can be modified with command lists.  This allows
		// the renderer to generate a command list on a deferred context, and
		// then utilize the list to perform rendering on an immediate context.

		//void GenerateCommandList( CommandListDX11* pList );
		//void ExecuteCommandList( CommandListDX11* pList );


		//--------------------------------------------------------------------------------------
		// Each of the following methods are used to modify resources in some way.  The user is
		// required to understand the implications of using these methods on both immediate
		// contexts and deferred contexts - this means for immediate that the command is carried
		// out more or less right away (placed in the instruction queue of the driver anyways)
		// while the deferred context command goes into a command list to be executed later on.
		// Please check to make sure what you are trying to do will work in both contexts!!!
		//--------------------------------------------------------------------------------------

		// Clear buffers does what it says - the currently bound render target views and depth
		// stencil view are cleared to the provided values.

		void ClearBuffers( Vector4f color, f32 depth = 1.0f, u32 stencil = 0 );

		bool UpdateBufferResource(ResourcePtr pResource, void* pdata, u32 size);


		// This is an alternative method to mapping for updating resources.  In certain 
		// situations one method may or may not be more efficient than the other, so it is
		// worth trying both to see which is more performant in a given situation.

		void UpdateSubresource( i32 rid, u32 DstSubresource, const D3D11_BOX *pDstBox, const void *pSrcData,
			u32 SrcRowPitch, u32 SrcDepthPitch );

		// Copy from one resource to another resource.  Check the documentation for restrictions
		// if you get errors when performing the copying.

		void CopySubresourceRegion( ResourcePtr DestResource, u32 DstSubresource, u32 DstX, u32 DstY, u32 DstZ,
			ResourcePtr SrcResource, u32 SrcSubresource, D3D11_BOX* pSrcBox );

		void CopyResource( ResourcePtr DestResource, ResourcePtr SrcResource );

        // Resolve a subresource from a MSAA texture to a non-MSAA texture.
        
        void ResolveSubresource ( ResourcePtr DestResource, u32 DstSubresource, 
                                    ResourcePtr SrcResource, u32 SrcSubresource,
                                    DXGI_FORMAT format );
		void ResolveSubresource(ResourceDX11* DestResource, u32 DstSubresource,
			ResourceDX11* SrcResource, u32 SrcSubresource,
			DXGI_FORMAT format);

		// Copy the internal counter variable from a UAV (either from Append/Consume 
		// functionality, or directly a StructuredBuffer internal counter) to the specified
		// output buffer.

		void CopyStructureCount( ResourcePtr dest, u32 offset, ResourcePtr uav );



		// Pipeline statistics are available through the query objects in D3D11.
		// Call start, do some rendering, call end, and then print the results to
		// a string and either log them or display them.

		void StartPipelineStatistics( );
		void EndPipelineStatistics( );
		std::wstring PrintPipelineStatistics( );


		// Provide the ability to set markers and event notifications for use in a debugging
		// tool such as the Graphics Debugger in VS2012.

		void BeginEvent( std::wstring& name );
		void EndEvent();
		void SetMarker( std::wstring& name );


		void SaveTextureScreenShot( i32 ID, std::wstring filename );

		ID3D11DeviceContext* GetDeviceContext();


		D3D_FEATURE_LEVEL						m_FeatureLevel;
		
		DeviceContextComPtr			            m_pContext;
		UserDefinedAnnotationComPtr				m_pAnnotation;
		
        static const i32                        NumQueries = 3;
        i32                                     m_iCurrentQuery;
        QueryComPtr					            m_Queries[NumQueries];
		D3D11_QUERY_DATA_PIPELINE_STATISTICS	m_PipelineStatsData;

		// The shader stage resources are managed by these classes.

		VertexStageDX11		VertexShaderStage;
		HullStageDX11		HullShaderStage;
		DomainStageDX11		DomainShaderStage;
		GeometryStageDX11	GeometryShaderStage;
		PixelStageDX11		PixelShaderStage;
		ComputeStageDX11	ComputeShaderStage;

		// Each of the shader stages are stored in array that is indexed by the shader
		// type enumeration for fast access.

		ShaderStageDX11*	ShaderStages[6];

		InputAssemblerStageDX11	InputAssemblerStage;
		StreamOutputStageDX11	StreamOutputStage;
		RasterizerStageDX11		RasterizerStage;
		OutputMergerStageDX11	OutputMergerStage;

	private:
		// Resources can be mapped in order to manually modify or read their contents.  The 
		// returned structure provides information about the resource including the pitch and
		// width to be used in accessing it.

		D3D11_MAPPED_SUBRESOURCE	MapResource(i32 index, u32 subresource, D3D11_MAP actions, u32 flags);
		void						UnMapResource(i32 index, u32 subresource);

		D3D11_MAPPED_SUBRESOURCE	MapResource(ResourcePtr pGlyphResource, u32 subresource, D3D11_MAP actions, u32 flags);
		void						UnMapResource(ResourcePtr pGlyphResource, u32 subresource);

		D3D11_MAPPED_SUBRESOURCE	MapResource(ResourceDX11* pGlyphResource, u32 subresource, D3D11_MAP actions, u32 flags);
		void						UnMapResource(ResourceDX11* pGlyphResource, u32 subresource);

	};
};

#endif // PipelineManagerDX11_h
