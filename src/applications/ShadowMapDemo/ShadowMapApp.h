#include "ApplicationDX11.h"
#include "BufferConfigDX11.h"
#include "RasterizerStateConfigDX11.h"
#include "Texture2dConfigDX11.h"
#include "ShaderResourceViewConfigDX11.h"
#include "RenderTargetViewConfigDX11.h"
#include "DepthStencilViewConfigDX11.h"
#include "ShaderResourceViewConfigDX11.h"
#include "SamplerStateConfigDX11.h"
#include "ShaderResourceViewDX11.h"
#include "TriangleIndices.h"
#include "Camera.h"

using namespace forward;

//--------------------------------------------------------------------------------
// Structure for Vertex Buffer
struct Vertex
{
	Vector3f Pos;
	Vector4f Color;
};

// structure for constant buffer
struct CBufferType
{
	Matrix4f mat;
	Matrix4f matLight;
	Vector4f flags;
};

class ShadowMapApp : public Application
{
public:
	ShadowMapApp(HINSTANCE hInstance, i32 width, i32 height)
		: Application(hInstance, width, height)
		, m_vsID(-1)
		, m_psID(-1)
		, m_drawShadowTarget(false)
		, m_usePCF(false)
	{
		mMainWndCaption = L"ShadowMapApp";
	}

	~ShadowMapApp()
	{
		Log::Get().Close();
	}

	virtual bool Init();
	virtual void OnResize();

protected:
	virtual void UpdateScene(f32 dt);
	virtual void DrawScene();
	virtual void OnSpace();
	virtual void OnEnter();
	virtual void OnChar(i8 key);
	virtual void OnMouseDown(WPARAM btnState, i32 x, i32 y);
	virtual void OnMouseMove(WPARAM btnState, i32 x, i32 y);

	void renderShadowTarget(const Matrix4f& mat);

private:
	void BuildShaders();
	void BuildGeometry();
	void BuildRenderTarget();
	void SetupPipeline();

	i32 m_vsID;
	i32 m_psID;

	i32 m_vsShadowTargetID;
	i32 m_psShadowTargetID;

	GeometryPtr m_pGeometry;
	GeometryPtr m_pFloor;
	Matrix4f m_worldMat;
	ResourcePtr m_constantBuffer;

	ResourcePtr m_renderTargetTex;
	ResourcePtr m_depthTargetTex;
	i32 m_samplerID;
	i32 m_pcfSamplerID;

	bool m_drawShadowTarget;
	bool m_usePCF;

	Camera m_camMain;
	Camera m_camLight;
	i32 m_preMouseX;
	i32 m_preMouseY;

	i32 m_shadowMapViewportID;
};