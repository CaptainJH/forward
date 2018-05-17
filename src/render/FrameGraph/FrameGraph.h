//***************************************************************************************
// FrameGraph.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include <vector>
#include "RenderPass.h"
#include "RenderPassHelper.h"

namespace forward
{
	struct RenderPassInfo;

	struct FrameGraphObjectInfo
	{
		FrameGraphObject* m_object;

		FrameGraphDrawingState* GetFrameGraphDrawingState();
		FrameGraphShader*		GetFrameGraphShader();
		VertexFormat*			GetVertexFormat();

		std::vector<RenderPassInfo*> m_readerPassList;
	};

	struct FrameGraphResourceInfo : public FrameGraphObjectInfo
	{
		FrameGraphResource*	GetFrameGraphResource();

		std::vector<RenderPassInfo*> m_writerPassList;
	};

	struct RenderPassInfo
	{
		RenderPass* m_renderPass = nullptr;

		std::vector<FrameGraphResourceInfo*> m_inputResources;
		std::vector<FrameGraphResourceInfo*> m_outputResources;

		std::vector<FrameGraphDrawingState*>	m_drawingStates;
		std::vector<FrameGraphShader*>			m_shaders;
		VertexFormat*							m_vertexFormat;
	};

	class FrameGraph
	{
	public:

		void Reset();
		void DrawRenderPass(RenderPass* pass);
		std::vector<RenderPassInfo>& GetRenderPassDB();

	private:
		std::vector<RenderPassInfo> m_passDB;

		std::vector<FrameGraphResourceInfo> m_allUsedResources;
		std::vector<FrameGraphObjectInfo*> m_allUsedDrawingStates;
		std::vector<FrameGraphObjectInfo*> m_allUsedVertexFormats;
		std::vector<FrameGraphObjectInfo*> m_allUsedShaders;

		void registerFrameGraphResource(FrameGraphResource* res);
	};
}