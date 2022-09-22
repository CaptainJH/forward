//***************************************************************************************
// RenderPass.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once
#include <functional>
#include "PipelineStateObjects.h"

namespace forward
{
	class Device;
	class RenderPassBuilder;

	class RenderPass
	{
	public:

		enum OperationFlags
		{
			OF_PRESENT		= 0x1,
			OF_CLEAN_RT		= 0x2,
			OF_CLEAN_DS		= 0x4,
			OF_NO_CLEAN		= 0x8,
			OF_DEFAULT		= OF_CLEAN_RT | OF_CLEAN_DS,
		};

		typedef std::function<void(RenderPassBuilder&, PipelineStateObject&)> SetupFuncType;
		typedef std::function<void(Device&)> ExecuteFuncType;

	public:
		RenderPass(OperationFlags operationType, SetupFuncType setup, ExecuteFuncType execute);
		RenderPass(SetupFuncType setup, ExecuteFuncType execute);
		RenderPass();
		~RenderPass();

		PipelineStateObject& GetPSO();
		OperationFlags GetRenderPassFlags() const;
		void Execute(Device&);
		void AttachRenderPass(RenderPass* ptr);
		RenderPass* GetNextRenderPass();

	protected:
		PipelineStateObject				m_pso;

		const SetupFuncType				m_setupCallback;
		const ExecuteFuncType			m_executeCallback;
		const OperationFlags			m_opFlags;

		RenderPass*						m_nextPass = nullptr;
	};
}