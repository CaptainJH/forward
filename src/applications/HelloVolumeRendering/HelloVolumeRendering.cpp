//[header]
// Rendering volumetric object using ray-marching. A basic implementation (chapter 1 & 2)
//
// https://www.scratchapixel.com/lessons/advanced-rendering/volume-rendering-for-developers/ray-marching-algorithm
//[/header]
//[compile]
// Download the raymarch-chap2.cpp file to a folder.
// Open a shell/terminal, and run the following command where the file is saved:
//
// clang++ -O3 raymarch-chap2.cpp -o render -std=c++17 (optional: -DBACKWARD_RAYMARCHING)
//
// You can use c++ if you don't use clang++
//
// Run with: ./render. Open the resulting image (ppm) in Photoshop or any program
// reading PPM files.
//[/compile]
//[ignore]
// Copyright (C) 2022  www.scratchapixel.com
// Modified by Heqi Ju 2023
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//[/ignore]

#define _USE_MATH_DEFINES
//#define RUN_BENCHMARK
#include "PCH.h"
#include "utilities/Utils.h"
#include "utilities/FileSystem.h"
#include "windows/dxCommon/DirectXTexEXR.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <memory>
#include <algorithm>
#include <vector>
#include <random>
#include <execution>
#include <tbb/tbb.h>

#ifdef RUN_BENCHMARK
#include "benchmark/benchmark.h"
#endif

#include "windows/dx12/DeviceDX12.h"
#include "RHI/FrameGraph/FrameGraph.h"
#include "utilities/ProfilingHelper.h"

using namespace forward;

constexpr float3 background_color{ 0.572f, 0.772f, 0.921f };
constexpr float floatMax = std::numeric_limits<float>::max();

struct IsectData
{
    float t0{ floatMax }, t1{ floatMax };
    float3 pHit;
    float3 nHit;
    bool inside{ false };
};

struct Object
{
public:
    float3 color;
    int type{ 0 };
    virtual bool intersect(const float3&, const float3&, IsectData&) const = 0;
    virtual ~Object() {}
    Object() {}
};

bool solveQuadratic(float a, float b, float c, float& r0, float& r1)
{
    float d = b * b - 4 * a * c;
    if (d < 0) return false;
    else if (d == 0) r0 = r1 = -0.5f * b / a;
    else {
        float q = (b > 0) ? -0.5f * (b + sqrtf(d)) : -0.5f * (b - sqrtf(d));
        r0 = q / a;
        r1 = c / q;
    }

    if (r0 > r1) std::swap(r0, r1);

    return true;
}

struct Sphere : Object
{
public:
    Sphere() { color = float3{ 1, 0, 0 }; type = 1; }
    bool intersect(const float3& rayOrig, const float3& rayDir, IsectData& isect) const override
    {
        auto rayOrigc = rayOrig - center;
        float a = rayDir ^ rayDir;
        float b = 2 * (rayDir ^ rayOrigc);
        float c = (rayOrigc ^ rayOrigc) - radius * radius;

        if (!solveQuadratic(a, b, c, isect.t0, isect.t1)) return false;

        if (isect.t0 < 0) {
            if (isect.t1 < 0) return false;
            else {
                isect.inside = true;
                isect.t0 = 0;
            }
        }

        return true;
    }

    float radius{ 1 };
    float3 center{ 0, 0, -4 };
};

std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(0.0, 1.0);

float3 integrate(const float3& ray_orig, const float3& ray_dir, const std::vector<std::unique_ptr<Object>> &objects)
{
    const Object* hit_object = nullptr;
    IsectData isect;
    for (const auto& object : objects) {
        IsectData isect_object;
        if (object->intersect(ray_orig, ray_dir, isect_object)) {
            hit_object = object.get();
            isect = isect_object;
        }
    }
	
    if (!hit_object)
        return background_color;

    float step_size = 0.2f;
    const float absorption = 0.1f;
    const float scattering = 0.1f;
    const float density = 1.0f;
    const int ns = static_cast<int>(std::ceil((isect.t1 - isect.t0) / step_size));
    step_size = (isect.t1 - isect.t0) / ns;

    float3 light_dir{ 0, 1, 0 };
    float3 light_color{ 1.3f, 0.3f, 0.9f };
    IsectData isect_vol;

    float transparency = 1; // initialize transmission to 1 (fully transparent)
    float3 result(0.0f); // initialize volumetric sphere color to 0

#ifdef BACKWARD_RAYMARCHING
    // [comment]
    // The ray-marching loop (backward, march from t1 to t0)
    // [/comment]
    for (int n = 0; n < ns; ++n) {
        float t = isect.t1 - step_size * (n + 0.5);
        vec3 sample_pos = ray_orig + t * ray_dir;

        float sample_transparency = exp(-step_size * (scattering + absorption));
        transparency *= sample_transparency;

        if (hit_object->intersect(sample_pos, light_dir, isect_vol) && isect_vol.inside) {
            float light_attenuation = exp(-density * isect_vol.t1 * (scattering + absorption));
            result += light_color * light_attenuation * scattering * density * step_size;
        }
        else
            std::cerr << "oops\n";

        result *= sample_transparency;
    }

    return background_color * transparency + result;
#else
	// [comment]
	// The ray-marching loop (forward, march from t0 to t1)
	// [/comment]
    for (int n = 0; n < ns; ++n) {
        float t = isect.t0 + step_size * (n + 0.5f);
        auto sample_pos = ray_orig + t * ray_dir;

        // compute sample transmission
        float sample_attenuation = exp(-step_size * (scattering + absorption));
        transparency *= sample_attenuation;

        // In-scattering. Find distance light travels through volumetric sphere to the sample.
        // Then use Beer's law to attenuate the light contribution due to in-scattering.
        if (hit_object->intersect(sample_pos, light_dir, isect_vol) && isect_vol.inside) {
            float light_attenuation = exp(-density * isect_vol.t1 * (scattering + absorption));
            result += transparency * light_color * light_attenuation * scattering * density * step_size;
        }
        else
            std::cerr << "oops\n";
    }

    // combine background color and volumetric sphere color
    return background_color * transparency + result;
#endif
}

std::vector<std::unique_ptr<Object>> g_geo;

void Setup()
{
    g_geo.clear();
    std::unique_ptr<Sphere> sph = std::make_unique<Sphere>();
    sph->radius = 5;
    sph->center.x = 0;
    sph->center.y = 0;
    sph->center.z = -20;
    g_geo.push_back(std::move(sph));
}

void DrawVolume(u32 size, std::vector<float3>& buffer)
{
    buffer.resize(size * size, { 0.0f, 0.0f, 0.0f });

    const auto frameAspectRatio = size / float(size);
    const float fov = 45;
    const float focal = tan(f_PI / 180 * fov * 0.5f);

    float3 rayOrig(0.0f);
    float3 rayDir(0.0f);

    unsigned int offset = 0;
    for (unsigned int j = 0; j < size; ++j) {
        for (unsigned int i = 0; i < size; ++i) {
            rayDir.x = (2.f * (i + 0.5f) / size - 1) * focal;
            rayDir.y = (1 - 2.f * (j + 0.5f) / size) * focal * 1 / frameAspectRatio; // Maya style
            rayDir.z = -1.f;

            rayDir.normalize();

            auto c = integrate(rayOrig, rayDir, g_geo);

            buffer[offset++] = {
                std::clamp(c.x, 0.f, 1.f),
                std::clamp(c.y, 0.f, 1.f),
                std::clamp(c.z, 0.f, 1.f)
            };
        }
    }
}

template<class Policy>
void DrawVolume_WithPolicy(u32 size, std::vector<float3>& buffer, Policy p)
{
    buffer.resize(size * size, { 0.0f, 0.0f, 0.0f });

    const auto frameAspectRatio = size / float(size);
    const float fov = 45;
    const float focal = tan(f_PI / 180 * fov * 0.5f);

    std::for_each(p, buffer.begin(), buffer.end(), [=, &buffer](auto& data) {
        float3 rayOrig(0.0f);
        float3 rayDir(0.0f);

        auto offset = &data - buffer.data();
        auto i = offset % size;
        auto j = offset / size;
        rayDir.x = (2.f * (i + 0.5f) / size - 1) * focal;
        rayDir.y = (1 - 2.f * (j + 0.5f) / size) * focal * 1 / frameAspectRatio; // Maya style
        rayDir.z = -1.f;

        rayDir.normalize();

        auto c = integrate(rayOrig, rayDir, g_geo);

        data = {
            std::clamp(c.x, 0.f, 1.f),
            std::clamp(c.y, 0.f, 1.f),
            std::clamp(c.z, 0.f, 1.f)
        };
        });
}

void DrawVolume_TBB(u32 size, std::vector<float3>& buffer)
{
    buffer.resize(size * size, { 0.0f, 0.0f, 0.0f });

    auto frameAspectRatio = size / float(size);
    float fov = 45;
    float focal = tan(f_PI / 180 * fov * 0.5f);

    tbb::parallel_for(0U, (u32)buffer.size(), 1U, [=, &buffer](auto offset) {
        float3 rayOrig(0.0f);
        float3 rayDir(0.0f);

        auto i = offset / size;
        auto j = offset % size;
        rayDir.x = (2.f * (i + 0.5f) / size - 1) * focal;
        rayDir.y = (1 - 2.f * (j + 0.5f) / size) * focal * 1 / frameAspectRatio; // Maya style
        rayDir.z = -1.f;

        rayDir.normalize();

        auto c = integrate(rayOrig, rayDir, g_geo);

        buffer[offset] = {
            std::clamp(c.x, 0.f, 1.f),
            std::clamp(c.y, 0.f, 1.f),
            std::clamp(c.z, 0.f, 1.f)
        };
        });
}

struct CB
{
    u32 Size;
    float SphereRadius;
    float pad;
    float pad1;
    float3 SphereCenter;
    float pad2;
};

struct DeviceContext
{
    u32 m_size;
    std::unique_ptr<DeviceDX12> m_devicePtr;
    std::unique_ptr<RenderPass> m_computePassPtr;
    shared_ptr<Texture2D> m_uavTex;
    shared_ptr<ConstantBuffer<CB>> m_cb;
    FileSystem m_fs;

    DeviceContext(u32 s)
        : m_size(s)
    {
        m_devicePtr = std::make_unique<DeviceDX12>();

        m_uavTex = make_shared<Texture2D>("UAV_Tex", forward::DF_R32G32B32A32_FLOAT, s, s, forward::TextureBindPosition::TBP_Shader);
        m_uavTex->SetUsage(RU_CPU_GPU_BIDIRECTIONAL);

        m_cb = make_shared<ConstantBuffer<CB>>("ConstantBuffer");

        m_computePassPtr = std::make_unique<RenderPass>(
            [&]([[maybe_unused]] RenderPassBuilder& builder, ComputePipelineStateObject& pso) {
                // setup shaders
                pso.m_CSState.m_shader = make_shared<ComputeShader>("DrawVolumeShader", L"HelloVolume", "DrawVolume");
                pso.m_CSState.m_uavShaderRes[0] = m_uavTex;
                pso.m_CSState.m_constantBuffers[0] = m_cb;
            },
            [=](CommandList& cmdList) {
                cmdList.Dispatch(m_size / 8, m_size / 8, 1);
            });
    }
};

void DrawVolume_ComputeShader(DeviceContext& ctx)
{
    *ctx.m_cb = CB{
        .Size = ctx.m_size,
        .SphereRadius = 5.0f,
        .SphereCenter = float3(0.f, 0.f, -20.0f),
    };
 
    FrameGraph fg;
    ctx.m_devicePtr->BeginDrawFrameGraph(&fg);
    fg.DrawRenderPass(ctx.m_computePassPtr.get());
    ctx.m_devicePtr->EndDrawFrameGraph();
    ctx.m_devicePtr->FlushDefaultQueue();

#ifndef RUN_BENCHMARK
    ctx.m_devicePtr->SaveTexture(L"HelloVolumeFromComputeShader.dds", ctx.m_uavTex.get());
#endif
}

#ifndef RUN_BENCHMARK
int main()
{
    unsigned int size = 512;
    std::vector<float3> buffer;
    Setup();
    FileSystem fileSystem;
    //DrawVolume(size, buffer);
    DrawVolume_WithPolicy(size, buffer, std::execution::par);
    //DrawVolume_TBB(size, buffer);

    // writing file
    std::wstring exrFilePath = FileSystem::getSingleton().GetSavedFolder() + L"HelloVolumeRendering.exr";
    DirectX::Image resultImage = {
        .width = size,
        .height = size,
        .format = DXGI_FORMAT_R32G32B32_FLOAT,
        .rowPitch = size * sizeof(float3),
        .slicePitch = size * size * sizeof(float3),
        .pixels = reinterpret_cast<uint8_t*>(buffer.data())
    };
    DirectX::SaveToEXRFile(resultImage, exrFilePath.c_str());

    //DeviceContext ctx(size);
    //ProfilingHelper::BeginPixCapture("HelloVolume_PIX_Capture.wpix");
    //DrawVolume_ComputeShader(ctx);
    //ProfilingHelper::EndPixCapture();

}
#else
static void BM_DrawVolume_Default(benchmark::State& state) {

    auto size = static_cast<u32>(state.range_x());
    Setup();
    std::vector<float3> buffer;
    for (auto _ : state)
    {
        DrawVolume(size, buffer);
    }
}
BENCHMARK(BM_DrawVolume_Default)->Arg(256)->Arg(512)->Arg(1024);

static void BM_DrawVolume_Parallel(benchmark::State& state) {

    auto size = static_cast<u32>(state.range_x());
    Setup();
    std::vector<float3> buffer;
    for (auto _ : state)
    {
        DrawVolume_WithPolicy(size, buffer, std::execution::par);
    }
}
BENCHMARK(BM_DrawVolume_Parallel)->Arg(256)->Arg(512)->Arg(1024);

static void BM_DrawVolume_TBB(benchmark::State& state) {

    auto size = static_cast<u32>(state.range_x());
    Setup();
    std::vector<float3> buffer;
    for (auto _ : state)
    {
        DrawVolume_TBB(size, buffer);
    }
}
BENCHMARK(BM_DrawVolume_TBB)->Arg(256)->Arg(512)->Arg(1024);

static void BM_DrawVolume_ComputeShader(benchmark::State& state) {

    auto size = static_cast<u32>(state.range_x());
    DeviceContext ctx(size);
    for (auto _ : state)
    {
        DrawVolume_ComputeShader(ctx);
    }
}
BENCHMARK(BM_DrawVolume_ComputeShader)->Arg(256)->Arg(512)->Arg(1024);

BENCHMARK_MAIN();
#endif
