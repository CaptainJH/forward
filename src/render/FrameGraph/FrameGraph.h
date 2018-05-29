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

		FrameGraphObjectInfo(FrameGraphObject* obj);

		std::vector<RenderPassInfo*> m_readerPassList;
	};

	struct FrameGraphResourceInfo : public FrameGraphObjectInfo
	{
		FrameGraphResource*	GetFrameGraphResource();

		FrameGraphResourceInfo(FrameGraphResource* res);

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

		FrameGraphResourceInfo* registerReadFrameGraphResource(FrameGraphResource* res, RenderPass* pass);
		FrameGraphResourceInfo* registerWriteFrameGraphResource(FrameGraphResource* res, RenderPass* pass);
		FrameGraphObjectInfo* registerDrawingState(FrameGraphDrawingState* state, RenderPass* pass);
		FrameGraphObjectInfo* registerVertexFormat(VertexFormat* vformat, RenderPass* pass);
		FrameGraphObjectInfo* registerShader(FrameGraphShader* shader, RenderPass* pass);
		void registerRenderPass(RenderPass* pass);
	};
}