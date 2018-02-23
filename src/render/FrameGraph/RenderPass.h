//***************************************************************************************
// RenderPass.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once
#include "PipelineStateObjects.h"
#include "FrameGraphResource.h"

namespace forward
{
	class RenderPass
	{
	public:

		virtual void Execute() = 0;

		PipelineStateObject& GetPSO();

		virtual void Reset();

	protected:
		PipelineStateObject				m_pso;
		std::vector<RenderPass*>		m_nextPasses;
	};
}