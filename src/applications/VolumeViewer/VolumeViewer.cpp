
#include "PCH.h"
#include "Application.h"
#include "RHI/FrameGraph/FrameGraph.h"


using namespace forward;

class VolumeViewer : public Application
{
public:
	VolumeViewer(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"VolumeViewer";
	}

	bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;



private:

};

bool VolumeViewer::Init()
{
	return true;
}

void VolumeViewer::UpdateScene(f32)
{

}

void VolumeViewer::DrawScene()
{

}

int main()
{

}