#pragma warning( push )
#pragma warning( disable : 4127 )
#pragma warning( disable : 4251 )
#pragma warning( disable : 4275 )
#include <OpenVDB/openvdb.h>
#pragma warning( pop )

#include "Application.h"
#include "RHI/FrameGraph/FrameGraph.h"
#include "dxCommon/DirectXTexEXR.h"

#include <execution>
#include <ImathBoxAlgo.h>


using namespace forward;

float4x4 cameraToWorld{ 0.844328f, 0, -0.535827f, 0, -0.170907f, 0.947768f, -0.269306f, 0, 0.50784f, 0.318959f, 0.800227f, 0, 83.292171f, 45.137326f, 126.430772f, 1 };
struct Grid
{
    const u32 baseResolution = 128U;
    std::unique_ptr<float[]> densityData;
    Box bb = { float3(-30.0f), float3(30.0f) };
    float operator () (const u32& xi, const u32& yi, const u32& zi) const 
    {
        if (xi < 0 || xi > baseResolution - 1 ||
            yi < 0 || yi > baseResolution - 1 ||
            zi < 0 || zi > baseResolution - 1)
            return 0;

        return densityData[(zi * baseResolution + yi) * baseResolution + xi];
    }
};

bool raybox(const Ray& ray, const Box& bound, float& tmin, float& tmax)
{
    float3 entry, exit;
    auto intersect = Imath::findEntryAndExitPoints(ray, bound, entry, exit);
    if (!intersect) return false;

    tmin = (entry - ray.pos).length();
    tmax = (exit - ray.pos).length();

    return true;
}

float phaseHG(const float3& viewDir, const float3& lightDir, const float& g)
{
    float costheta = viewDir.dot(lightDir);
    return 1 / (4 * f_PI) * (1 - g * g) / powf(1 + g * g - 2 * g * costheta, 1.5f);
}

//[comment]
// Function where the coordinates of the sample points are converted from world space
// to voxel space. We can then use these coordinates to read the density stored
// in the grid. We can either use a nearest neighbor search or a trilinear
// interpolation.
//[/comment]
float lookup(const Grid& grid, const float3& p)
{
    const float3 gridSize = grid.bb.max - grid.bb.min;
    const float3 pLocal = (p - grid.bb.min) / gridSize;
    const float3 pVoxel = pLocal * static_cast<f32>(grid.baseResolution);

    float3 pLattice(pVoxel.x - 0.5f, pVoxel.y - 0.5f, pVoxel.z - 0.5f);
    int xi = static_cast<int>(std::floor(pLattice.x));
    int yi = static_cast<int>(std::floor(pLattice.y));
    int zi = static_cast<int>(std::floor(pLattice.z));
#if 0
    // nearest neighbor seach
    return grid(xi, yi, zi);
#else
    // trilinear filtering
    float weight[3];
    float value = 0;

    for (int i = 0; i < 2; ++i) 
    {
        weight[0] = 1 - std::abs(pLattice.x - (xi + i));
        for (int j = 0; j < 2; ++j) 
        {
            weight[1] = 1 - std::abs(pLattice.y - (yi + j));
            for (int k = 0; k < 2; ++k) 
            {
                weight[2] = 1 - std::abs(pLattice.z - (zi + k));
                value += weight[0] * weight[1] * weight[2] * grid(xi + i, yi + j, zi + k);
            }
        }
    }

    return value;
#endif
}

void integrate(
    const Ray& ray,                         // camera ray 
    const float& tMin, const float& tMax,   // range of integration
    Color3& L,                               // radiance (out)
    float& T,                               // transmission (out)
    const Grid& grid)                       // cached data
{
    const float stepSize = 0.05f;
    float sigma_a = 0.5f;
    float sigma_s = 0.5f;
    float sigma_t = sigma_a + sigma_s;
    float g = 0; // henyey-greenstein asymetry factor 
    size_t d = 2; // russian roulette "probability" 
    float shadowOpacity = 1;

    size_t numSteps = static_cast<size_t>(std::ceil((tMax - tMin) / stepSize));
    float stride = (tMax - tMin) / numSteps;

    float3 lightDir(-0.315798f, 0.719361f, 0.618702f);
    Color3 lightColor(20);

    Color3 Lvol(0.0f);
    float Tvol = 1;

    for (size_t n = 0; n < numSteps; ++n) 
    {
        float t = tMin + stride * (n + 0.5f);
        float3 samplePos = ray(t);

        //[comment]
        // Read density from the 3D grid
        //[/comment]
        float density = lookup(grid, samplePos);

        float Tsample = exp(-stride * density * sigma_t);
        Tvol *= Tsample;

        float tlMin, tlMax;
        Ray lightRay(samplePos, samplePos + lightDir);
        if (density > 0 && raybox(lightRay, grid.bb, tlMin, tlMax) && tlMax > 0) 
        {
            size_t numStepsLight = static_cast<size_t>(std::ceil(tlMax / stepSize));
            float strideLight = tlMax / numStepsLight;
            float densityLight = 0;
            for (size_t nl = 0; nl < numStepsLight; ++nl) 
            {
                float tLight = strideLight * (nl + 0.5f);
                //[comment]
                // Read density from the 3D grid
                //[/comment]
                densityLight += lookup(grid, lightRay(tLight));
            }
            float lightRayAtt = exp(-densityLight * strideLight * sigma_t * shadowOpacity);
            Lvol += lightColor * lightRayAtt * phaseHG(-ray.dir, lightDir, g) * sigma_s * Tvol * stride * density;
        }

        if (Tvol < 1e-3) 
        {
            if (rand() / (float)RAND_MAX > 1.f / d)
                break;
            else
                Tvol *= d;
        }
    }

    L = Lvol;
    T = Tvol;
}

struct RenderContext
{
    float fov{ 45 };
    size_t width{ 640 }, height{ 480 };
    float frameAspectRatio;
    float focal;
    float pixelWidth;
    Color3 backgroundColor{ 0.572f, 0.772f, 0.921f };
};

void initRenderContext(RenderContext& rc)
{
    rc.frameAspectRatio = rc.width / float(rc.height);
    rc.focal = tanf(f_PI / 180 * rc.fov * 0.5f);
    rc.pixelWidth = rc.focal / rc.width;
}

void trace(Ray& ray, Color3& L, float& transmittance, const RenderContext& /*rc*/, const Grid& grid)
{
    float tmin, tmax;
    if (raybox(ray, grid.bb, tmin, tmax))
        integrate(ray, tmin, tmax, L, transmittance, grid);
}

void render(const size_t& frame)
{
    std::cout << "Rendering frame: " << frame << std::endl;

    //[comment]
    // Load the density data from file into memory for this current frame
    //[/comment]
    std::ifstream ifs;
    std::stringstream ss;
    ss << "D:/Documents/GitHub/scratchapixel/volume-rendering-for-developers/cachefiles/grid." << frame << ".bin";
    ifs.open(ss.str().c_str(), std::ios::binary);
    Grid grid;
    grid.densityData = std::make_unique<float[]>(grid.baseResolution * grid.baseResolution * grid.baseResolution);
    ifs.read((char*)grid.densityData.get(), sizeof(float) * grid.baseResolution * grid.baseResolution * grid.baseResolution);
    ifs.close();

    const u32 width = 640, height = 480;

    RenderContext rc;
    initRenderContext(rc);

    const size_t nsamples = 1;

    Vector<Color3> imgbuf(width * height);

    const float3 rayOrig = float3(0) * cameraToWorld;

    std::for_each(std::execution::par, imgbuf.begin(), imgbuf.end(), [&](auto& data) {
        auto offset = &data - imgbuf.data();
        auto i = offset % width;
        auto j = offset / width;

        Color3 pixelColor(0.0f);
        //float  opacity = 0;
        for (unsigned jj = 0; jj < nsamples; ++jj) {
            for (unsigned ii = 0; ii < nsamples; ++ii) {
                float3 rayDir;
                rayDir.x = (2 * (i + 1.f / nsamples * (ii + 0.5f)) / width - 1) * rc.focal;
                rayDir.y = (1 - 2 * (j + 1.f / nsamples * (jj + 0.5f)) / height) * rc.focal * 1 / rc.frameAspectRatio; // Maya style
                rayDir.z = -1;

                cameraToWorld.multDirMatrix(rayDir, rayDir);
                rayDir.normalize();

                Ray ray(rayOrig, rayOrig + rayDir);

                Color3 L(0.0f); // radiance for that ray (light collected)
                float transmittance = 1;
                trace(ray, L, transmittance, rc, grid);
                pixelColor += rc.backgroundColor * transmittance + L;
            }
        }
        imgbuf[offset] = {
            std::clamp(pixelColor.x, 0.f, 1.f),
            std::clamp(pixelColor.y, 0.f, 1.f),
            std::clamp(pixelColor.z, 0.f, 1.f)
        };
        });

    // writing file
    std::wstringstream wss;
    wss << L"smoke." << frame << L".exr";

    std::wstring exrFilePath = FileSystem::getSingleton().GetSavedFolder() + wss.str();
    DirectX::Image resultImage = {
        .width = width,
        .height = height,
        .format = DXGI_FORMAT_R32G32B32_FLOAT,
        .rowPitch = width * sizeof(float3),
        .slicePitch = width * height * sizeof(float3),
        .pixels = reinterpret_cast<uint8_t*>(imgbuf.data())
    };
    DirectX::SaveToEXRFile(resultImage, exrFilePath.c_str());
}

class VolumeViewer : public Application
{
public:
	VolumeViewer(i32 width, i32 height)
		: Application(width, height)
	{
		mMainWndCaption = L"VolumeViewer";
		mAppType = AT_OffScreen;
	}

	bool Init() override;

protected:
	void UpdateScene(f32 dt) override;
	void DrawScene() override;



private:

};

bool VolumeViewer::Init()
{
	Log::Get().Open();
	if (!Application::Init())
		return false;

	return true;
}

void VolumeViewer::UpdateScene(f32)
{

}

void VolumeViewer::DrawScene()
{
    //for (auto frame = 1; frame <= 90; ++frame)
    //    render(frame);
    render(0);
    render(50);
}

int main()
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	VolumeViewer theApp(1200, 800);

	if (!theApp.Init())
		return 0;

	theApp.Run();
}