//***************************************************************************************
// RenderPass.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once
#include <functional>
#include "PipelineStateObjects.h"

namespace forward
{
	class Renderer;
	class RenderPassBuilder;

	class RenderPass
	{
	public:
		enum CleanType
		{
			CT_Default,
			CT_Nothing,
		};

		typedef std::function<void(RenderPassBuilder&, PipelineStateObject&)> SetupFuncType;
		typedef std::function<void(Renderer&)> ExecuteFuncType;

	public:
		RenderPass(CleanType cleanType, SetupFuncType setup, ExecuteFuncType execute);
		RenderPass();
		~RenderPass();

		PipelineStateObject& GetPSO();
		CleanType GetCleanType() const;
		void Execute(Renderer&);
		void AttachRenderPass(RenderPass* ptr);
		RenderPass* GetNextRenderPass();

	protected:
		PipelineStateObject				m_pso;

		const SetupFuncType				m_setupCallback;
		const ExecuteFuncType			m_executeCallback;
		const CleanType					m_cleanType;

		RenderPass*						m_nextPass = nullptr;
	};
}