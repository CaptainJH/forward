#pragma once
#include <math.h>
#include "FrameGraph/RenderPassHelper.h"
#include "FrameGraph/Geometry.h"
#include "Vector2f.h"
#include "Vector3f.h"
#include "utilities/Utils.h"

namespace forward
{
	class SceneData : public IRenderPassGenerator
	{
	public:
		static SceneData LoadFromFile(const std::wstring fileName);

		void OnRenderPassBuilding(RenderPass&) override;

		std::vector<SimpleGeometry> mMeshData;
	};
}