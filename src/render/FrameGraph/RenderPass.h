//***************************************************************************************
// RenderPass.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once
#include <functional>
#include "PipelineStateObjects.h"

namespace forward
{

	class Renderer;

	class RenderPass
	{
	public:
		enum CleanType
		{
			CT_Default,
		};

		typedef std::function<void(PipelineStateObject&)> SetupFuncType;
		typedef std::function<void(Renderer&)> ExecuteFuncType;

	public:
		RenderPass(CleanType cleanType, SetupFuncType setup, ExecuteFuncType execute);
		~RenderPass();

		PipelineStateObject& GetPSO();
		CleanType GetCleanType() const;
		void Execute(Renderer&);

	protected:
		PipelineStateObject				m_pso;
		std::vector<RenderPass*>		m_nextPasses;

		const SetupFuncType				m_setupCallback;
		const ExecuteFuncType			m_executeCallback;
		const CleanType					m_cleanType;
	};
}