#include "dx11/ApplicationDX11.h"
#include "dx11/ResourceSystem/Buffer/BufferConfigDX11.h"
#include "dx11/ResourceSystem/Texture/Texture2dConfigDX11.h"
#include "dx11/ResourceSystem/StateObject/SamplerStateConfigDX11.h"
#include "dx11/ResourceSystem/ResourceView/ShaderResourceViewDX11.h"
#include "TriangleIndices.h"

#include "Camera.h"
#include "CascadedShadowMapMatrixSetCaculator.h"
#include "LightSpacePerspectiveShadowMapMatrixCaculator.h"

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
	Matrix4f matCSM[CascadedShadowMapMatrixSetCaculator::m_iTotalCascades];
	Vector4f toCascadeOffsetX;
	Vector4f toCascadeOffsetY;
	Vector4f toCascadeScale;
	f32 shadowMapPixelSize;
	f32 lightSize;
	f32 blurFactorX;
	f32 blurFactorY;
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
		, m_useCSM(false)
		, m_useLiSP(false)
		, m_usePCSS(false)
		, m_useVSM(false)
		, m_useBlur(false)
		, m_blurCoef(0.25f)
		, m_CSM(m_camMain)
		, m_LiSP(m_camLight, m_camMain)
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

private:
	void BuildShaders();
	void BuildGeometry();
	void BuildRenderTarget();
	void SetupPipeline();

	void renderShadowTarget(const Matrix4f& mat);
	void renderSpotLightShadowMap(const Matrix4f& mat);
	void renderCSMShadowMap();

	void blurShadowRenderTarget(ResourceProxyPtr ptr);

	i32 m_vsID;
	i32 m_psID;

	i32 m_vsShadowTargetID;
	i32 m_psShadowTargetID;

	i32 m_vsCSMID;
	i32 m_gsCSMID;

	i32 m_psVSMDepthGenID;

	i32 m_vsQuadID;
	i32 m_psQuadBlurID;

	GeometryPtr m_pGeometry;
	GeometryPtr m_pFloor;
	GeometryPtr m_pQuad;
	Matrix4f m_worldMat;
	ResourceProxyPtr m_constantBuffer;

	ResourceProxyPtr m_shadowMapRenderTargetTex;
	ResourceProxyPtr m_shadowMapDepthTargetTex;
	ResourceProxyPtr m_CSMRenderTargetTex;
	ResourceProxyPtr m_CSMDepthTargetTex;
	ResourceProxyPtr m_VSMRenderTargetTex;
	ResourceProxyPtr m_VSMBlurRenderTargetTex;
	i32 m_samplerID;
	i32 m_pcfSamplerID;

	bool m_drawShadowTarget;
	bool m_usePCF;
	bool m_useCSM;
	bool m_useLiSP;
	bool m_usePCSS;
	bool m_useVSM;
	bool m_useBlur;

	f32 m_blurCoef;
	f32 m_blurCoefW;
	f32 m_blurCoefH;

	Camera m_camMain;
	Camera m_camLight;
	i32 m_preMouseX;
	i32 m_preMouseY;

	i32 m_shadowMapViewportID;
	i32 m_CSMViewportID;
	i32 m_blurViewportID;

	CascadedShadowMapMatrixSetCaculator m_CSM;
	LightSpacePerspectiveShadowMapMatrixCaculator m_LiSP;
};