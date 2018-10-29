#pragma once
#include "RenderPassHelper.h"
#include "render/ResourceSystem/Buffers/FrameGraphBuffer.h"
#include "Vector2f.h"
#include "Vector3f.h"
#include "utilities/Utils.h"

namespace forward
{
	class VisualEffect : public IRenderPassSource
	{
	public:
		VisualEffect();
		virtual ~VisualEffect();

		VisualEffect& SetPass(u8 passIndex);

	protected:
		u8	m_currentPassIndex = 0;

	private:
		//void OnRenderPassBuilding(RenderPass&) override;
	};
}