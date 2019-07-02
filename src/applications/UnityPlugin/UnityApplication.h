#pragma once

#include "Application.h"
#include "render/FrameGraph/FrameGraph.h"
#include "render/FrameGraph/Geometry.h"


class UnityApplication : public forward::Application
{
public:
	UnityApplication(void* device, forward::RendererType renderType, const char* forwardPath)
		: forward::Application(device, renderType, forwardPath)
	{}

	~UnityApplication()
	{
		SAFE_DELETE(m_renderPass);
	}

	bool Init() override;
	void OnResize() override;

protected:
	void UpdateScene(forward::f32 dt) override;
	void DrawScene() override;

private:

	forward::Matrix4f m_worldMat;
	forward::Matrix4f m_viewMat;
	forward::Matrix4f m_projMat;

	forward::RenderPass* m_renderPass;
};
