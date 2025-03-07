#include "Application.h"
#include "RHI/FrameGraph/FrameGraph.h"
#include "RHI/FrameGraph/Geometry.h"

#include "ArcBall.h"

#include "D:/Documents/GitHub/tech_notes/extern/glm/glm/glm.hpp"
#include "D:/Documents/GitHub/tech_notes/extern/glm/glm/gtc/constants.hpp"

using namespace glm;
typedef f32 real;

using namespace forward;

constexpr real PI = glm::pi<real>();

vec3 CalculateNormal(const vec3& a, const vec3& b, const vec3& c)
{
    // Get edges
    vec3 e0 = b - a;
    vec3 e1 = c - a;

    // Get perpendicular
    return normalize(cross(e0, e1));
}

struct SampleSphere
{
    static const int	NB_BANDS = 4;
    static const int	NB_BASES = NB_BANDS * NB_BANDS;

    SampleSphere(void) {}

    SampleSphere(const real u, const real v) :
        theta(u),
        phi(v),
        index(-1)
    {
        x = sin(theta) * cos(phi);
        z = sin(theta) * sin(phi);
        y = cos(theta);
    }

    // Spherical co-ordinates
    real	theta, phi;

    // Cartesian co-ordinates
    real	x, y, z;

    // y(l,m,theta,phi) for each l,m
    real	coeffs[NB_BASES];

    // Sample index in a set
    int		index;
};

struct SphericalFunction
{
    virtual real ToReal(const SampleSphere& s) const = 0;
    virtual vec3 ToColour(const SampleSphere& s) const = 0;

    // Convert radius to vector
    // Map from spherical co-ordinates onto the sphere using the unsigned sample as the radius
    vec3 ToVector(const real u, const real v)
    {
        SampleSphere s(u, v);

        // Sample the function
        real r = fabs(ToReal(s));

        // Map from spherical co-ordinates onto the sphere using the unsigned sample as the radius
        vec3 vec;
        vec.x = (float)(r * s.x);
        vec.y = (float)(r * s.y);
        vec.z = (float)(r * s.z);

        return (vec);
    }
};

void DrawSphericalFunction(SphericalFunction& func, const int /*name*/, std::vector<Vertex_POS_COLOR>& vertex, std::vector<u32>& index)
{
    const int	NB_SAMPLES = 50;

    real du = PI / NB_SAMPLES;
    real dv = 2 * PI / NB_SAMPLES;


    for (real u = 0; u < PI; u += du)
    {
        for (real v = 0; v < 2 * PI; v += dv)
        {
            // Sample 4 points in the neighbourhood
            vec3 p[4] =
            {
                func.ToVector(u, v),
                func.ToVector(u + du, v),
                func.ToVector(u + du, v + dv),
                func.ToVector(u, v + dv)
            };

            real ddu = du / 10;
            real ddv = dv / 10;

            // Sample the normals as even smaller patches in the neighbourhood
            vec3 n[4] =
            {
                CalculateNormal(p[0], func.ToVector(u + ddu, v), func.ToVector(u, v + ddv)),
                CalculateNormal(p[1], func.ToVector(u + du + ddu, v), func.ToVector(u + du, v + ddv)),
                CalculateNormal(p[2], func.ToVector(u + du + ddu, v + dv), func.ToVector(u + du, v + dv + ddv)),
                CalculateNormal(p[3], func.ToVector(u + ddu, v + dv), func.ToVector(u, v + dv + ddv))
            };

            // Positive SH samples are green, negative are red
            float c[3] = { 0, 0, 0 };
            if (func.ToReal(SampleSphere(u, v)) < 0)
                c[0] = 1;
            else
                c[1] = 1;

			const u32 base_idx = static_cast<u32>(vertex.size());
			for (int i = 0; i < 4; ++i) {
				Vertex_POS_COLOR newVertex;
				newVertex.Pos = { p[i].x, p[i].y, p[i].z };
				//v.Color = { c[0], c[1], c[2], 1.0f };
				auto l = std::min(1.0f, length(p[i]));
				auto pixelValue = std::abs(l * 0.6f);
				newVertex.Color = { pixelValue, pixelValue, pixelValue, 1.0f };
				vertex.push_back(newVertex);
			}

			index.push_back(base_idx + 0);
			index.push_back(base_idx + 1);
			index.push_back(base_idx + 2);
			index.push_back(base_idx + 2);
			index.push_back(base_idx + 3);
			index.push_back(base_idx + 0);

        }
    }

}

struct ExampleLight : public SphericalFunction
{
    real ToReal(const SampleSphere& s) const
    {
        // Example lighting function provided in documentation
        return (max(0.0f, 5 * cos(s.theta) - 4) + max(0.0f, -4 * sin(s.theta - PI) * cos(s.phi - 2.5f) - 3));
        //return (max(0.0f, 8 * cos(s.theta) - 4) + max(0.0f, -4 * sin(s.theta - PI) * cos(s.phi - 2.5f) - 3));
    }

    vec3 ToColour(const SampleSphere& s) const
    {
        return (vec3((float)ToReal(s)));
    }
};

struct CBufferTypeVS
{
	Matrix4f mat;
	Matrix4f dummy;
};

class SH_Demo : public Application
{
public:
	SH_Demo(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"SH_Demo";

		m_arcCam.SetWindow(width, height);
		m_arcCam.SetRadius(2.0f);
	}

	virtual bool Init();
	virtual void OnResize();

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;
	void OnSpace() override;

	void OnMouseDown(u8 /*btnState*/, f32 /*x*/, f32 /*y*/) override;
	void OnMouseUp(u8 /*btnState*/, f32 /*x*/, f32 /*y*/) override;
	void OnMouseMove(u8 /*btnState*/, f32 /*x*/, f32 /*y*/) override;
	Vector3f rotateVector(Quaternion<f32> q, Vector3f v);

private:

	Matrix4f m_viewMat;
	Matrix4f m_projMat;

	shared_ptr<ConstantBuffer<CBufferTypeVS>> m_constantBufferVS;

	shared_ptr<VertexBuffer>		m_VB;
	shared_ptr<IndexBuffer>		m_IB;

	std::unique_ptr<RenderPass> m_renderPass;

	ArcBall m_arcCam;
};

void SH_Demo::UpdateScene(f32 /*dt*/)
{
	auto camRot = m_arcCam.GetQuat();
	Vector3f target = Vector3f(0.0f, 0.0f, 0.0f);
	Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);
	Vector3f dir = rotateVector(camRot, Vector3f(0.0f, 0.0f, -1.0f));
	auto camPos = target + dir * 2.0f;
	Vector3f newUp = rotateVector(camRot, up);

	auto viewMat = Matrix4f::LookAtLHMatrix(camPos, target, newUp);
	auto viewProjMat = viewMat * m_projMat;
	auto worldMat = Matrix4f::Identity();
	auto worldInvT = worldMat.Inverse().Transpose();

	(*m_constantBufferVS).GetTypedData()->mat = viewProjMat;
	(*m_constantBufferVS).GetTypedData()->dummy = worldMat.Inverse().Transpose();
}

void SH_Demo::DrawScene()
{
	FrameGraph fg;
	m_pDevice->BeginDrawFrameGraph(&fg);
	fg.DrawRenderPass(m_renderPass.get());
	m_pDevice->EndDrawFrameGraph();
}

bool SH_Demo::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	// Build the view matrix.
	Vector3f pos = Vector3f(0.0f, 0.0f, 2.0f);
	Vector3f target; target.MakeZero();
	Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);
	m_viewMat = Matrix4f::LookAtLHMatrix(pos, target, up);
	// Build the projection matrix
	m_projMat = Matrix4f::PerspectiveFovLHMatrix(0.5f * f_PI, AspectRatio(), 0.01f, 100.0f);

	m_renderPass = std::make_unique<RenderPass>(
		[&](RenderPassBuilder&, RasterPipelineStateObject& pso) {
			// setup shaders
			pso.m_VSState.m_shader = forward::make_shared<VertexShader>("PassVS", L"BasicShader", "VSMain");
			pso.m_PSState.m_shader = forward::make_shared<PixelShader>("PassPS", L"BasicShader", "PSMainQuad");

			std::vector<Vertex_POS_COLOR> vertex;
			std::vector<u32> index;
			ExampleLight exampleLight;
			DrawSphericalFunction(exampleLight, 0, vertex, index);

			// setup geometry
			m_VB = forward::make_shared<VertexBuffer>("SH_VB", Vertex_POS_COLOR::GetVertexFormat(), 
				static_cast<u32>(vertex.size()));
			m_IB = forward::make_shared<IndexBuffer>("SH_IB", PrimitiveTopologyType::PT_TRIANGLELIST, 
				static_cast<u32>(index.size()));
			for (auto& v : vertex)
				m_VB->AddVertex(v);
			for (auto idx : index)
				m_IB->AddIndex(idx);

			pso.m_IAState.m_indexBuffer = m_IB;
			pso.m_IAState.m_topologyType = m_IB->GetPrimitiveType();

			pso.m_IAState.m_vertexBuffers[0] = m_VB;
			pso.m_IAState.m_vertexLayout = m_VB->GetVertexFormat();

			// setup constant buffer
			m_constantBufferVS = make_shared<ConstantBuffer<CBufferTypeVS>>("CB0");
			pso.m_VSState.m_constantBuffers[0] = m_constantBufferVS;

			// setup render targets
			auto dsPtr = m_pDevice->GetDefaultDS();
			pso.m_OMState.m_depthStencilResource = dsPtr;

			auto rsPtr = m_pDevice->GetDefaultRT();
			pso.m_OMState.m_renderTargetResources[0] = rsPtr;

			// setup rasterizer
			pso.m_RSState.m_rsState.enableScissor = false;
		},
		[&](CommandList& cmdList) {
			cmdList.DrawIndexed(m_IB->GetNumElements());
		});

	return true;
}

void SH_Demo::OnResize()
{
	Application::OnResize();
}

void SH_Demo::OnSpace()
{
	mAppPaused = !mAppPaused;
	//m_pRender2->SaveRenderTarget(L"FirstRenderTargetOut.bmp");
}

void SH_Demo::OnMouseDown(u8 btnState, f32 x, f32 y)
{
	if (btnState == 1)
	{
		m_arcCam.OnBegin(x, y);
	}
}

void SH_Demo::OnMouseUp(u8 /*btnState*/, f32 /*x*/, f32 /*y*/)
{
	m_arcCam.OnEnd();
}

void SH_Demo::OnMouseMove(u8 /*btnState*/, f32 x, f32 y)
{
	if (m_arcCam.IsDragging())
	{
		// Rotate camera
		m_arcCam.OnMove(x, y);
	}
}

Vector3f SH_Demo::rotateVector(Quaternion<f32> q, Vector3f v)
{
	Quaternion<f32> vec(0.0f, v.x, v.y, v.z);
	Quaternion<f32> inv(q.w, -q.x, -q.y, -q.z);
	auto result = q * vec * inv;
	return Vector3f(result.x, result.y, result.z);
}

FORWARD_APPLICATION_MAIN(SH_Demo, 1920, 1080);