#pragma once
#include "RenderPass.h"

namespace forward
{
	class RenderPass;
	class RenderPassBuilder;

	class IRenderPassSource
	{
	private:
		virtual void OnRenderPassBuilding(RenderPass&) = 0;

		friend class RenderPassBuilder;
		friend class RenderPassBuilderDecorator;
	};

	class RenderPassBuilder
	{
	public:
		RenderPassBuilder(RenderPass*);
		virtual ~RenderPassBuilder();

		virtual RenderPassBuilder & operator<<(IRenderPassSource&);

	protected:
		RenderPass * m_renderPass;
		virtual RenderPass* GetRenderPass();

		friend class RenderPassBuilderDecorator;
	};

	RenderPassBuilder& operator<<(RenderPassBuilder& lhs, RenderPassBuilder& rhs);

	class RenderPassBuilderDecorator : public RenderPassBuilder
	{
	public:
		RenderPassBuilderDecorator();

		RenderPassBuilder& operator<<(IRenderPassSource&) override;
		void SetRenderPassBuilder(RenderPassBuilder* builder);

	protected:
		RenderPassBuilder * m_builder = nullptr;

		RenderPass* GetRenderPass() override;

	private:
		virtual void PostDummyRenderPassBuilding(RenderPass& dummy) = 0;
	};

	class VertexBufferOnly : public RenderPassBuilderDecorator
	{
	private:
		void PostDummyRenderPassBuilding(RenderPass& dummy) override;
	};

	extern VertexBufferOnly VBOnly;
}