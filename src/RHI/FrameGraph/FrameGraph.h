//***************************************************************************************
// FrameGraph.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "RenderPass.h"

namespace forward
{
	struct RenderPassInfo;

	struct FrameGraphObjectInfo
	{
		GraphicsObject* m_object;

		DrawingState* GetFrameGraphDrawingState();
		Shader*		GetFrameGraphShader();
		VertexFormat*			GetVertexFormat();

		FrameGraphObjectInfo(GraphicsObject* obj);

		std::vector<RenderPassInfo*> m_readerPassList;
	};

	struct FrameGraphResourceInfo : public FrameGraphObjectInfo
	{
		Resource*	GetFrameGraphResource();

		FrameGraphResourceInfo(Resource* res);

		std::vector<RenderPassInfo*> m_writerPassList;
	};

	struct RenderPassInfo
	{
		RenderPassInfo(RenderPass* pass);

		RenderPass* m_renderPass = nullptr;

		std::vector<FrameGraphResourceInfo*>	m_inputResources;
		std::vector<FrameGraphResourceInfo*>	m_outputResources;

		std::vector<FrameGraphObjectInfo*>		m_drawingStates;
		std::vector<FrameGraphObjectInfo*>		m_shaders;
		FrameGraphObjectInfo*					m_vertexFormat;
	};

	class FrameGraph
	{
	public:

		void Reset();
		void DrawRenderPass(RenderPass* pass);
		std::vector<RenderPassInfo>& GetRenderPassDB();
		void LinkInfo();

	private:
		std::vector<RenderPassInfo> m_passDB;

		std::vector<FrameGraphResourceInfo> m_allUsedResources;
		std::vector<FrameGraphObjectInfo>	m_allUsedDrawingStates;
		std::vector<FrameGraphObjectInfo>	m_allUsedVertexFormats;
		std::vector<FrameGraphObjectInfo>	m_allUsedShaders;

		FrameGraphResourceInfo* registerReadFrameGraphResource(Resource* res, RenderPass* pass);
		FrameGraphResourceInfo* registerWriteFrameGraphResource(Resource* res, RenderPass* pass);
		FrameGraphObjectInfo* registerDrawingState(DrawingState* state, RenderPass* pass);
		FrameGraphObjectInfo* registerVertexFormat(VertexFormat* vformat, RenderPass* pass);
		FrameGraphObjectInfo* registerShader(Shader* shader, RenderPass* pass);
		void registerRenderPass(RenderPass* pass);
	};
}