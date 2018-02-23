//***************************************************************************************
// PipelineStateObject.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once
#include "Vector4f.h"
#include "FrameGraphResource.h"
#include "PrimitiveTopology.h"
#include "VertexFormat.h"

namespace forward
{
	class BlendState
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



	class DepthStencilState
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



	class RasterizerState
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

	struct PipelineStageState
	{
		PipelineStageState()
		{}
	};

	struct InputAssemblerStageState : public PipelineStageState
	{
		PrimitiveTopologyType	m_topologyType;
		FrameGraphResource		m_indexBuffer;
		std::vector<FrameGraphResource> m_vertexBuffers;
		VertexFormat			m_vertexLayout;
	};

	struct OutputMergerStageState : public PipelineStageState
	{
		BlendState			m_blendState;
		DepthStencilState	m_dsState;

		FrameGraphResource	m_depthStencilResource;
		std::vector<FrameGraphResource> m_renderTargetResources;

	};

	struct RECT
	{
		i32 left;
		i32 top;
		i32	width;
		i32 height;
	};

	struct RasterizerStageState : public PipelineStageState
	{
		RasterizerState	m_rsState;
		std::vector<RECT> m_viewports;
		std::vector<RECT> m_scissorRects;
	};

	struct ShaderStageState : public PipelineStageState
	{
		FrameGraphResource m_shader;
		std::vector<FrameGraphResource> m_constantBuffers;
		std::vector<FrameGraphResource> m_shaderResources;
	};


	struct PipelineStateObject
	{
		InputAssemblerStageState	m_IAState;
		RasterizerStageState		m_RSState;
		OutputMergerStageState		m_OMState;

		ShaderStageState			m_VSState;
		ShaderStageState			m_PSState;
	};
}