#pragma once
#include "RHI/ResourceSystem/Buffer.h"
#include "RHI/FrameGraph/FrameGraph.h"
#include "RHI/Device.h"
#include "math/Vector2f.h"
#include "math/Vector3f.h"
#include "math/Matrix4f.h"
#include "utilities/Utils.h"

namespace forward
{
	class FrameGraph;
	class RendererBase : public intrusive_ref_counter
	{
	public:
		RendererBase() = default;
		virtual ~RendererBase() = default;

		RendererBase& SetPass(u8 passIndex);
		void DrawEffect(FrameGraph*);
		void Update(f32 dt) { if (mUpdateFunc)mUpdateFunc(dt); }

		std::function<void(f32)> mUpdateFunc = nullptr;

	protected:
		u8 m_currentPassIndex = 0;

	public:
		Vector<RenderPass> m_renderPassVec;
	};
}