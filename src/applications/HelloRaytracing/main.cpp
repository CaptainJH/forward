

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
		Log::Get().Close();
	}

	bool Init() override
	{
		Log::Get().Open();
		if (!InitMainWindow())
			return false;


		return true;
	}

protected:
	void UpdateScene(f32) override
	{

	}
	void DrawScene() override
	{

	}

};

FORWARD_APPLICATION_MAIN(HelloRaytracing, 1920, 1080);