#pragma once
#include "RenderPass.h"

namespace forward
{
	class RenderPass;
	class RenderPassBuilder;

	class IRenderPassGenerator
	{
	private:
		virtual void OnRenderPassBuilding(RenderPass&) = 0;

		friend class RenderPassBuilder;
	};

	class RenderPassBuilder
	{
	public:
		RenderPassBuilder(RenderPass*);
		virtual ~RenderPassBuilder() = default;

		virtual RenderPassBuilder & operator<<(IRenderPassGenerator&);
		virtual RenderPass* GetRenderPass();

	protected:
		RenderPass * m_renderPass;
	};
}