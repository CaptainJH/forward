#pragma once
#include "RenderPassHelper.h"
#include "render/ResourceSystem/Buffer.h"
#include "Vector2f.h"
#include "Vector3f.h"
#include "utilities/Utils.h"

namespace forward
{
	class Effect : public IRenderPassSource
	{
	public:
		Effect();
		virtual ~Effect();

		Effect& SetPass(u8 passIndex);

	protected:
		u8	m_currentPassIndex = 0;

	private:
		//void OnRenderPassBuilding(RenderPass&) override;
	};
}