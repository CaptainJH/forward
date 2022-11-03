#include "ApplicationWin.h"
#include "RHI/FrameGraph/FrameGraph.h"
#include "RHI/FrameGraph/Geometry.h"
#include <iostream>
#include <filesystem>
#include "dx12/ResourceSystem/Textures/DeviceTexture2DDX12.h"
#include "dx12/DeviceDX12.h"

using namespace forward;

class TexLoadingTest : public Application
{
public:
	TexLoadingTest(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"TexLoadingTest";
		DeviceType = DeviceType::Device_Forward_DX12;
		mAppType = AT_OffScreen;
	}

	~TexLoadingTest() override
	{
	}

	bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;

public:
	std::filesystem::path m_folder;
};

i32 main(int argc, char* argv[])
{
	if (argc < 2)
		return -1;

	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	TexLoadingTest theApp(1200, 800);

	if (!theApp.Init())
		return 0;

	theApp.m_folder = argv[1];
	assert(std::filesystem::exists(theApp.m_folder));
	theApp.Run();
}

void TexLoadingTest::UpdateScene([[maybe_unused]]f32 dt) {}

void TexLoadingTest::DrawScene()
{
	auto device = static_cast<DeviceDX12*>(m_pDevice);
	device->ResetCommandList();
	std::vector<forward::shared_ptr<Texture2D>> texList;
	for (auto it : std::filesystem::directory_iterator(m_folder))
	{
		if (std::filesystem::is_regular_file(it.path())
			&&
			(it.path().extension() == ".tga" || it.path().extension() == ".DDS")
			)
		{
			auto fileToLoad = it.path().c_str();
			auto tex = make_shared<Texture2D>("Tex", fileToLoad);
			if (!tex->DeviceObject())
			{
				auto deviceTex = forward::make_shared<DeviceTexture2DDX12>(device->GetDevice(), tex.get());
				tex->SetDeviceObject(deviceTex);
			}

			texList.push_back(tex);
		}
	}

	// Execute the initialization commands
	HR(device->CommandList()->Close());
	ID3D12CommandList* cmdLists[] = { device->CommandList()};
	device->CommandQueue()->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	// Wait until initialization is complete.
	device->FlushCommandQueue();
	std::cout << texList.size() << " textures loaded" << std::endl;
}

bool TexLoadingTest::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	return true;
}