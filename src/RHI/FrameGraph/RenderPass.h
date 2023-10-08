//***************************************************************************************
// RenderPass.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once
#include <functional>
#include "PipelineStateObjects.h"

namespace forward
{
	class CommandList;
	class RenderPassBuilder;

	class RenderPass
	{
	public:

		enum OperationFlags
		{
			OF_PRESENT			= 0x1,
			OF_CLEAN_RT		= 0x2,
			OF_CLEAN_DS		= 0x4,
			OF_NO_CLEAN		= 0x8,
			OF_DEFAULT		= OF_CLEAN_RT | OF_CLEAN_DS,
		};

		typedef std::function<void(RenderPassBuilder&, RasterPipelineStateObject&)> RasterSetupFuncType;
		typedef std::function<void(RenderPassBuilder&, ComputePipelineStateObject&)> ComputeSetupFuncType;
		typedef std::function<void(RenderPassBuilder&, RTPipelineStateObject&)> RTSetupFuncType;
		typedef std::function<void(CommandList&)> ExecuteFuncType;

	public:
		RenderPass(RasterSetupFuncType setup, ExecuteFuncType execute, OperationFlags operationType=OF_DEFAULT);
		RenderPass(ComputeSetupFuncType setup, ExecuteFuncType execute, OperationFlags operationType=OF_DEFAULT);
		RenderPass(RTSetupFuncType setup, ExecuteFuncType execute, OperationFlags operationType=OF_DEFAULT);

		RenderPass() = delete;
		~RenderPass() = default;

		OperationFlags GetRenderPassFlags() const;
		void Execute(CommandList&);
		void AttachRenderPass(RenderPass* ptr);
		RenderPass* GetNextRenderPass();

		template<class T> T& GetPSO()
		{
			return std::get<T>(m_pso);
		}

		template<class T> bool IsPSO()
		{
			return std::holds_alternative<T>(m_pso);
		}

	protected:
		PSOUnion				m_pso;

		const ExecuteFuncType			m_executeCallback;
		const OperationFlags			m_opFlags;

		RenderPass*						m_nextPass = nullptr;
	};
}