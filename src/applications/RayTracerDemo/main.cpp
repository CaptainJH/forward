//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "D3D12RaytracingHelloWorld.h"

#include "Application.h"

using namespace forward;

class HelloRaytracing : public Application
{
public:
	HelloRaytracing(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"Hello Raytracing!";
	}

	~HelloRaytracing()
	{
		m_pDemo->OnDestroy();
		Log::Get().Close();
	}

	bool Init() override
	{
		Log::Get().Open();
		if (!InitMainWindow())
			return false;

		m_pDemo = std::make_unique<D3D12RaytracingHelloWorld>(mClientWidth, mClientHeight, mMainWndCaption, mhMainWnd);
		m_pDemo->OnInit();

		return true;
	}

protected:
	void UpdateScene(f32 dt) override
	{
		if (m_pDemo)
			m_pDemo->OnUpdate();
	}
	void DrawScene() override
	{
		if (m_pDemo)
			m_pDemo->OnRender();
	}

	std::unique_ptr<D3D12RaytracingHelloWorld> m_pDemo;
};

FORWARD_APPLICATION_MAIN(HelloRaytracing, 1920, 1080);