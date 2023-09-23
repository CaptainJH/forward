//***************************************************************************************
// PipelineStateObject.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once
#include <array>
#include "Vector4f.h"
#include "RHI/ResourceSystem/Buffer.h"
#include "RHI/ResourceSystem/Texture.h"
#include "RHI/ResourceSystem/ShaderTable.h"
#include "RHI/ShaderSystem/Shader.h"
#include "RHI/RendererCapability.h"
#include "PrimitiveTopology.h"
#include "VertexFormat.h"

namespace forward
{
	struct BindingRange
	{
		u32 bindStart = std::numeric_limits<u32>::max();
		u32 bindEnd = 0;
		
		bool IsEmpty() const { return bindEnd < bindStart; }
		u32 Count() const { return bindEnd - bindStart + 1; }
	};

	struct BindingRanges
	{
		Vector<BindingRange> m_ranges;

		bool AddRange(BindingRange range);
	};

	class DrawingState : public GraphicsObject
	{
	public:
		DrawingState(const std::string& name, GraphicsObjectType type);
	};

	class BlendState : public DrawingState
	{
	public:
		enum Mode
		{
			BM_ZERO,
			BM_ONE,
			BM_SRC_COLOR,
			BM_INV_SRC_COLOR,
			BM_SRC_ALPHA,
			BM_INV_SRC_ALPHA,
			BM_DEST_ALPHA,
			BM_INV_DEST_ALPHA,
			BM_DEST_COLOR,
			BM_INV_DEST_COLOR,
			BM_SRC_ALPHA_SAT,
			BM_FACTOR,
			BM_INV_FACTOR,
			BM_SRC1_COLOR,
			BM_INV_SRC1_COLOR,
			BM_SRC1_ALPHA,
			BM_INV_SRC1_ALPHA
		};

		enum Operation
		{
			OP_ADD,
			OP_SUBTRACT,
			OP_REV_SUBTRACT,
			OP_MIN,
			OP_MAX
		};

		enum ColorWrite
		{
			CW_ENABLE_RED = 1,
			CW_ENABLE_GREEN = 2,
			CW_ENABLE_BLUE = 4,
			CW_ENABLE_ALPHA = 8,
			CW_ENABLE_ALL = 15
		};

		static const u32 NUM_TARGETS = 8;

		struct Target
		{
			bool enable;        // default: false
			Mode srcColor;      // default: BM_ONE
			Mode dstColor;      // default: BM_ZERO
			Operation opColor;  // default: OP_ADD
			Mode srcAlpha;      // default: BM_ONE
			Mode dstAlpha;      // default: BM_ZERO
			Operation opAlpha;  // default: OP_ADD
			u8 mask;			// default: CW_ENABLE_ALL
		};

		// Construction.
		BlendState();

		// Member access.  The members are intended to be write-once before
		// you create an associated graphics state.
		bool enableAlphaToCoverage;     // default: false
		bool enableIndependentBlend;    // default: false
		Target target[NUM_TARGETS];
		Vector4f blendColor;      // default: (0,0,0,0)
		u32 sampleMask;        // default: 0xFFFFFFFF
	};



	class DepthStencilState : public DrawingState
	{
	public:
		enum WriteMask
		{
			MASK_ZERO,
			MASK_ALL
		};

		enum Comparison
		{
			NEVER,
			LESS,
			EQUAL,
			LESS_EQUAL,
			GREATER,
			NOT_EQUAL,
			GREATER_EQUAL,
			ALWAYS
		};

		enum Operation
		{
			OP_KEEP,
			OP_ZERO,
			OP_REPLACE,
			OP_INCR_SAT,
			OP_DECR_SAT,
			OP_INVERT,
			OP_INCR,
			OP_DECR
		};

		struct Face
		{
			Operation fail;
			Operation depthFail;
			Operation pass;
			Comparison comparison;
		};

		// Construction.
		DepthStencilState();

		// Member access.  The members are intended to be write-once before
		// you create an associated graphics state.
		bool depthEnable;        // default: true
		WriteMask writeMask;     // default: MASK_ALL
		Comparison comparison;   // default: LESS_EQUAL
		bool stencilEnable;      // default: false
		u8 stencilReadMask;      // default: 0xFF
		u8 stencilWriteMask;     // default: 0xFF
		Face frontFace;          // default: (KEEP,KEEP,KEEP,ALWAYS)
		Face backFace;           // default: (KEEP,KEEP,KEEP,ALWAYS)
		u32 reference;           // default: 0
	};



	class RasterizerState : public DrawingState
	{
	public:
		enum FillMode
		{
			FILL_SOLID,
			FILL_WIREFRAME
		};

		enum CullMode
		{
			CULL_NONE,
			CULL_FRONT,
			CULL_BACK
		};

		// Construction.
		RasterizerState();

		// Member access.  The members are intended to be write-once before
		// you create an associated graphics state.
		FillMode fillMode;              // default: FILL_SOLID
		CullMode cullMode;              // default: CULL_BACK
		bool frontCCW;                  // default: true
		i32 depthBias;                  // default: 0
		f32 depthBiasClamp;             // default: 0
		f32 slopeScaledDepthBias;       // default: 0
		bool enableDepthClip;           // default: true
		bool enableScissor;             // default: false
		bool enableMultisample;         // default: false
		bool enableAntialiasedLine;     // default: false
	};

	class SamplerState : public DrawingState
	{
	public:
		// The encoding involves minification (MIN), magnification (MAG), and
		// mip-level filtering (MIP).  After each is P (POINT) or L (LINEAR).
		enum Filter
		{
			MIN_P_MAG_P_MIP_P,
			MIN_P_MAG_P_MIP_L,
			MIN_P_MAG_L_MIP_P,
			MIN_P_MAG_L_MIP_L,
			MIN_L_MAG_P_MIP_P,
			MIN_L_MAG_P_MIP_L,
			MIN_L_MAG_L_MIP_P,
			MIN_L_MAG_L_MIP_L,
			ANISOTROPIC,
			COMPARISON_MIN_P_MAG_P_MIP_P,
			COMPARISON_MIN_P_MAG_P_MIP_L,
			COMPARISON_MIN_P_MAG_L_MIP_P,
			COMPARISON_MIN_P_MAG_L_MIP_L,
			COMPARISON_MIN_L_MAG_P_MIP_P,
			COMPARISON_MIN_L_MAG_P_MIP_L,
			COMPARISON_MIN_L_MAG_L_MIP_P,
			COMPARISON_MIN_L_MAG_L_MIP_L,
			COMPARISON_ANISOTROPIC
		};

		// Modes for handling texture coordinates at texture-image boundaries.
		enum Mode
		{
			WRAP,
			MIRROR,
			CLAMP,
			BORDER,
			MIRROR_ONCE
		};

		enum Comparison
		{
			NEVER,
			LESS,
			EQUAL,
			LESS_EQUAL,
			GREATER,
			NOT_EQUAL,
			GREATER_EQUAL,
			ALWAYS
		};

		// Construction.
		SamplerState(const std::string& name);

		// Member access.  The members are intended to be write-once before
		// you create an associated graphics state.
		Filter		filter;
		Mode		mode[3];
		f32			mipLODBias;
		u32			maxAnisotropy;
		Comparison	comparison;
		Vector4f	borderColor;
		f32 minLOD;
		f32 maxLOD;
	};

	struct PipelineStageState
	{
		PipelineStageState()
		{}
	};

	struct InputAssemblerStageState : public PipelineStageState
	{
		PrimitiveTopologyType	m_topologyType;
		shared_ptr<IndexBuffer>	m_indexBuffer = nullptr;
		std::array<shared_ptr<VertexBuffer>, FORWARD_RENDERER_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> m_vertexBuffers = { nullptr };
		VertexFormat			m_vertexLayout;
	};

	struct OutputMergerStageState : public PipelineStageState
	{
		BlendState			m_blendState;
		DepthStencilState	m_dsState;

		shared_ptr<Texture2D>	m_depthStencilResource;
		std::array<shared_ptr<Texture2D>, FORWARD_RENDERER_SIMULTANEOUS_RENDER_TARGET_COUNT> m_renderTargetResources = { nullptr };
	};

	struct RECT
	{
		i32 left;
		i32 top;
		i32	width;
		i32 height;
	};

	struct ViewPort
	{
		f32 Width = 0.0f;
		f32 Height = 0.0f;
		f32 TopLeftX = 0.0f;
		f32 TopLeftY = 0.0f;
		f32 MinDepth = 0.0f;
		f32 MaxDepth = 1.0f;
	};

	struct RasterizerStageState : public PipelineStageState
	{
		RasterizerState	m_rsState;

		void AddViewport(ViewPort vp);
		void AddScissorRect(RECT rect);
		u32 m_activeViewportsNum = 0;
		u32 m_activeScissorRectNum = 0;
		std::array<ViewPort, FORWARD_RENDERER_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE> m_viewports;
		std::array<forward::RECT, FORWARD_RENDERER_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE> m_scissorRects;
	};

	struct ShaderStageState : public PipelineStageState
	{
		std::array<shared_ptr<ConstantBufferBase>, FORWARD_RENDERER_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT> m_constantBuffers = { nullptr };
		std::array<shared_ptr<Resource>, FORWARD_RENDERER_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> m_shaderResources = { nullptr };
		std::array<shared_ptr<SamplerState>, FORWARD_RENDERER_COMMONSHADER_SAMPLER_SLOT_COUNT> m_samplers = { nullptr };

		void SetConstantBufferData(Shader& shader, u32 index, std::unordered_map<String, String>& params);
		void SetConstantBufferDataFromPtr(u32 index, void* data)
		{
			memcpy(m_constantBuffers[index]->GetData(), data, m_constantBuffers[index]->GetNumBytes());
		}
	};

	struct VertexShaderStageState : public ShaderStageState
	{
		shared_ptr<VertexShader> m_shader;
	};

	struct PixelShaderStageState : public ShaderStageState
	{
		shared_ptr<PixelShader> m_shader;

		void SetConstantBufferDataFromStr(u32 index, std::unordered_map<String, String>& params)
		{
			ShaderStageState::SetConstantBufferData(*m_shader, index, params);
		}
	};

	struct GeometryShaderStageState : public ShaderStageState
	{
		shared_ptr<GeometryShader> m_shader;
	};

	struct ComputeShaderStageState : public ShaderStageState
	{
		shared_ptr<ComputeShader> m_shader;
		std::array<shared_ptr<Texture>, 8> m_uavShaderRes = { nullptr };
	};

	struct BindlessShaderStage
	{
		u32 m_space;
		Vector<shared_ptr<ConstantBufferBase>> m_constantBuffers;
		Vector<shared_ptr<Resource>> m_shaderResources;
		Vector<shared_ptr<Texture>> m_uavShaderRes;
	};

	struct RaytracingShaderStageState : public ShaderStageState
	{
		shared_ptr<RaytracingShaders> m_shader;
		std::array<shared_ptr<Texture>, 8> m_uavShaderRes = { nullptr };

		Vector<BindlessShaderStage> m_bindlessShaderStageStates;

		shared_ptr<ShaderTable> m_rayGenShaderTable;
		shared_ptr<ShaderTable> m_hitShaderTable;
		shared_ptr<ShaderTable> m_missShaderTable;
	};

	struct PipelineStateObject
	{
		InputAssemblerStageState	m_IAState;
		RasterizerStageState			m_RSState;
		OutputMergerStageState		m_OMState;

		VertexShaderStageState		m_VSState;
		GeometryShaderStageState	m_GSState;
		PixelShaderStageState			m_PSState;
		ComputeShaderStageState		m_CSState;

		u32 m_usedCBV_SRV_UAV_Count = 0;
		u32 m_usedSampler_Count = 0;

		shared_ptr<DeviceObject>	m_devicePSO;
	};
	
	struct SceneData;
	struct RTPipelineStateObject
	{
		struct InstanceData
		{
			u32 meshId;
			u32 materialId;
			float4x4 object2WorldMat;
		};

		Vector<std::pair<shared_ptr<VertexBuffer>, shared_ptr<IndexBuffer>>> m_meshes;
		Vector<InstanceData> m_instances;
		RaytracingShaderStageState m_rtState;
		u32 m_usedCBV_SRV_UAV_Count = 0;

		shared_ptr<DeviceObject> m_deviceRTPSO;

		RTPipelineStateObject() = default;
		RTPipelineStateObject(const SceneData& scene);
	};
}