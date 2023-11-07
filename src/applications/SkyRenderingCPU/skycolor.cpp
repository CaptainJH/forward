//[header]
// Simulating the color of the sky (Nishita model).
//
// See "Display of The Earth Taking into Account Atmospheric Scattering" for more information.
//[/header]
//[compile]
// Download the acceleration.cpp and teapotdata.h file to a folder.
// Open a shell/terminal, and run the following command where the files is saved:
//
// clang++ -std=c++11 -o skycolor skycolor.cpp -O3
//
// You can use c++ if you don't use clang++
//
// Run with: ./skycolor. Open the resulting image (ppm) in Photoshop or any program
// reading PPM files.
//[/compile]
//[ignore]
// Copyright (C) 2016  www.scratchapixel.com
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

#include "PCH.h"
#include "Utils.h"
#include "FileSystem.h"
#include "dxCommon/DirectXTexEXR.h"

using namespace forward;

const float kInfinity = std::numeric_limits<float>::max();

// [comment]
// The atmosphere class. Stores data about the planetory body (its radius), the atmosphere itself
// (thickness) and things such as the Mie and Rayleigh coefficients, the sun direction, etc.
// [/comment]
class Atmosphere
{
public:
    Atmosphere(
        float3 sd = float3(0, 1, 0), 
        float er = 6360e3, float ar = 6420e3,
        float hr = 7994, float hm = 1200) :
        sunDirection(sd),
        earthRadius(er),
        atmosphereRadius(ar),
        Hr(hr),
        Hm(hm)
    {}

    float3 computeIncidentLight(const float3& orig, const float3& dir, float tmin, float tmax) const;

    float3 sunDirection;     // The sun direction (normalized)
    float earthRadius;      // In the paper this is usually Rg or Re (radius ground, eart)
    float atmosphereRadius; // In the paper this is usually R or Ra (radius atmosphere)
    float Hr;               // Thickness of the atmosphere if density was uniform (Hr)
    float Hm;               // Same as above but for Mie scattering (Hm)

    static const float3 betaR;
    static const float3 betaM;
};

const float3 Atmosphere::betaR(3.8e-6f, 13.5e-6f, 33.1e-6f);
const float3 Atmosphere::betaM(21e-6f);

bool solveQuadratic(float a, float b, float c, float& x1, float& x2)
{
    if (b == 0) {
        // Handle special case where the the two vector ray.dir and V are perpendicular
        // with V = ray.orig - sphere.centre
        if (a == 0) return false;
        x1 = 0; x2 = std::sqrtf(-c / a);
        return true;
    }
    float discr = b * b - 4 * a * c;

    if (discr < 0) return false;

    float q = (b < 0.f) ? -0.5f * (b - std::sqrtf(discr)) : -0.5f * (b + std::sqrtf(discr));
    x1 = q / a;
    x2 = c / q;

    return true;
}

// [comment]
// A simple routine to compute the intersection of a ray with a sphere
// [/comment]
bool raySphereIntersect(const float3& orig, const float3& dir, const float& radius, float& t0, float& t1)
{
    // They ray dir is normalized so A = 1 
    float A = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
    float B = 2 * (dir.x * orig.x + dir.y * orig.y + dir.z * orig.z);
    float C = orig.x * orig.x + orig.y * orig.y + orig.z * orig.z - radius * radius;

    if (!solveQuadratic(A, B, C, t0, t1)) return false;

    if (t0 > t1) std::swap(t0, t1);

    return true;
}

// [comment]
// This is where all the magic happens. We first raymarch along the primary ray (from the camera origin
// to the point where the ray exits the atmosphere or intersect with the planetory body). For each
// sample along the primary ray, we then "cast" a light ray and raymarch along that ray as well.
// We basically shoot a ray in the direction of the sun.
// [/comment]
float3 Atmosphere::computeIncidentLight(const float3& orig, const float3& dir, float tmin, float tmax) const
{
    float t0, t1;
    if (!raySphereIntersect(orig, dir, atmosphereRadius, t0, t1) || t1 < 0) return float3(0.0f);
    if (t0 > tmin && t0 > 0) tmin = t0;
    if (t1 < tmax) tmax = t1;
    uint32_t numSamples = 16;
    uint32_t numSamplesLight = 8;
    float segmentLength = (tmax - tmin) / numSamples;
    float tCurrent = tmin;
    float3 sumR(0), sumM(0); // mie and rayleigh contribution
    float opticalDepthR = 0, opticalDepthM = 0;
    float mu = dir.dot(sunDirection); // mu in the paper which is the cosine of the angle between the sun direction and the ray direction
    float phaseR = 3.f / (16.f * f_PI) * (1.f + mu * mu);
    float g = 0.76f;
    float phaseM = 3.f / (8.f * f_PI) * ((1.f - g * g) * (1.f + mu * mu)) / ((2.f + g * g) * pow(1.f + g * g - 2.f * g * mu, 1.5f));
    for (uint32_t i = 0; i < numSamples; ++i) {
        float3 samplePosition = orig + (tCurrent + segmentLength * 0.5f) * dir;
        float height = samplePosition.length() - earthRadius;
        // compute optical depth for light
        float hr = exp(-height / Hr) * segmentLength;
        float hm = exp(-height / Hm) * segmentLength;
        opticalDepthR += hr;
        opticalDepthM += hm;
        // light optical depth
        float t0Light, t1Light;
        raySphereIntersect(samplePosition, sunDirection, atmosphereRadius, t0Light, t1Light);
        float segmentLengthLight = t1Light / numSamplesLight, tCurrentLight = 0;
        float opticalDepthLightR = 0, opticalDepthLightM = 0;
        uint32_t j;
        for (j = 0; j < numSamplesLight; ++j) {
            float3 samplePositionLight = samplePosition + (tCurrentLight + segmentLengthLight * 0.5f) * sunDirection;
            float heightLight = samplePositionLight.length() - earthRadius;
            if (heightLight < 0) break;
            opticalDepthLightR += exp(-heightLight / Hr) * segmentLengthLight;
            opticalDepthLightM += exp(-heightLight / Hm) * segmentLengthLight;
            tCurrentLight += segmentLengthLight;
        }
        if (j == numSamplesLight) {
            float3 tau = betaR * (opticalDepthR + opticalDepthLightR) + betaM * 1.1f * (opticalDepthM + opticalDepthLightM);
            float3 attenuation(exp(-tau.x), exp(-tau.y), exp(-tau.z));
            sumR += attenuation * hr;
            sumM += attenuation * hm;
        }
        tCurrent += segmentLength;
    }

    // [comment]
    // We use a magic number here for the intensity of the sun (20). We will make it more
    // scientific in a future revision of this lesson/code
    // [/comment]
    return (sumR * betaR * phaseR + sumM * betaM * phaseM) * 20;
}

void renderSkydome(const float3& sunDir, const wchar_t* filename)
{
    Atmosphere atmosphere(sunDir);
    auto t0 = std::chrono::high_resolution_clock::now();
#if 1
    // [comment]
    // Render fisheye
    // [/comment]
    const unsigned width = 512, height = 512;
    float3 *image = new float3[width * height], *p = image;
    memset(image, 0x0, sizeof(float3) * width * height);
    for (unsigned j = 0; j < height; ++j) {
        float y = 2.f * (j + 0.5f) / float(height - 1) - 1.f;
        for (unsigned i = 0; i < width; ++i, ++p) {
            float x = 2.f * (i + 0.5f) / float(width - 1) - 1.f;
            float z2 = x * x + y * y;
            if (z2 <= 1) {
                float phi = std::atan2(y, x);
                float theta = std::acos(1 - z2);
                float3 dir(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
                // 1 meter above sea level
                *p = atmosphere.computeIncidentLight(float3(0, atmosphere.earthRadius + 1, 0), dir, 0, kInfinity);
            }
        }
    }
#else
    // [comment]
    // Render from a normal camera
    // [/comment]
    const unsigned width = 640, height = 480;
    float3 *image = new float3[width * height], *p = image;
    memset(image, 0x0, sizeof(float3) * width * height);
    float aspectRatio = width / float(height);
    float fov = 65;
    float angle = std::tan(fov * M_PI / 180 * 0.5f);
    unsigned numPixelSamples = 4;
    float3 orig(0, atmosphere.earthRadius + 1000, 30000); // camera position
    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(0, 1); // to generate random floats in the range [0:1]
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x, ++p) {
            for (unsigned m = 0; m < numPixelSamples; ++m) {
                for (unsigned n = 0; n < numPixelSamples; ++n) {
                    float rayx = (2 * (x + (m + distribution(generator)) / numPixelSamples) / float(width) - 1) * aspectRatio * angle;
                    float rayy = (1 - (y + (n + distribution(generator)) / numPixelSamples) / float(height) * 2) * angle;
                    float3 dir(rayx, rayy, -1);
                    normalize(dir);
                    // [comment]
                    // Does the ray intersect the planetory body? (the intersection test is against the Earth here
                    // not against the atmosphere). If the ray intersects the Earth body and that the intersection
                    // is ahead of us, then the ray intersects the planet in 2 points, t0 and t1. But we
                    // only want to comupute the atmosphere between t=0 and t=t0 (where the ray hits
                    // the Earth first). If the viewing ray doesn't hit the Earth, or course the ray
                    // is then bounded to the range [0:INF]. In the method computeIncidentLight() we then
                    // compute where this primary ray intersects the atmosphere and we limit the max t range 
                    // of the ray to the point where it leaves the atmosphere.
                    // [/comment]
                    float t0, t1, tMax = kInfinity;
                    if (raySphereIntersect(orig, dir, atmosphere.earthRadius, t0, t1) && t1 > 0)
                        tMax = std::max(0.f, t0);
                    // [comment]
                    // The *viewing or camera ray* is bounded to the range [0:tMax]
                    // [/comment]
                    *p += atmosphere.computeIncidentLight(orig, dir, 0, tMax);
                }
            }
            *p *= 1.f / (numPixelSamples * numPixelSamples);
        }
        fprintf(stderr, "\b\b\b\b%3d%c", (int)(100 * y / (width - 1)), '%');
    }
#endif
    std::cout << "\b\b\b\b" << ((std::chrono::duration<float>)(std::chrono::high_resolution_clock::now() - t0)).count() << " seconds" << std::endl;

    // writing file
    std::wstring exrFilePath = FileSystem::getSingleton().GetSavedFolder() + filename;
    DirectX::Image resultImage = {
        .width = width,
        .height = height,
        .format = DXGI_FORMAT_R32G32B32_FLOAT,
        .rowPitch = width * sizeof(float3),
        .slicePitch = width * height * sizeof(float3),
        .pixels = (uint8_t*)image
    };
    DirectX::SaveToEXRFile(resultImage, exrFilePath.c_str());
    delete[] image;
}

int main()
{
    FileSystem fileSystem;
#if 1
    // [comment]
    // Render a sequence of images (sunrise to sunset)
    // [/comment]
    unsigned nangles = 128;
    std::vector<std::pair<u32, f32>> anglesVec;
    for (unsigned i = 0; i < nangles; ++i) {
        float angle = i / float(nangles - 1) * f_PI * 0.6f;
        anglesVec.emplace_back(std::make_pair(i, angle));
    }
    std::for_each(std::execution::par, anglesVec.begin(), anglesVec.end(), [](auto& p) {
        std::wstringstream filename;
        filename << "skydome." << p.first << ".exr";
        auto angle = p.second;
        std::cout << "Rendering image" << p.first << "angle = " << angle * 180 / f_PI << std::endl;
        renderSkydome(float3(sin(angle), cos(angle), 0), filename.str().c_str());
        });
#else
    // [comment]
    // Render one single image
    // [/comment]
    float angle = M_PI * 0;
    float3 sunDir(0, std::cos(angle), -std::sin(angle));
    std::cout << "Sun direction: " << sunDir << std::endl;
    renderSkydome(sunDir, L"skydome.exr");
#endif

    return 0;
}