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

#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#include "shared.h"

struct SceneConstantBuffer
{
    float4x4 projectionToWorld;
    float4 cameraPosition;
    float4 lightPosition;
    float4 lightAmbientColor;
    float4 lightDiffuseColor;
};

struct CubeConstantBuffer
{
    float4 albedo;
};

struct Vertex
{
    float3 position;
    float3 normal;
};

RaytracingAccelerationStructure Scene : register(t0, space99);
RWTexture2D<float4> RenderTarget : register(u0);

ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);
ConstantBuffer<CubeConstantBuffer> g_cubeCB : register(b1);

ByteAddressBuffer Indices[MAX_INSTANCES_COUNT] : register(t0, space2);
StructuredBuffer<Vertex> Vertices[MAX_INSTANCES_COUNT] : register(t0, space3);

struct RayPayload
{
    float4 color;
};

// Retrieve hit world position.
float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

// Retrieve attribute at a hit position interpolated from vertex attributes using the hit's barycentrics.
float3 HitAttribute(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

// Generate a ray in world space for a camera pixel corresponding to an index from the dispatched 2D grid.
inline void GenerateCameraRay(uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a ray.
    float4 world = mul(float4(screenPos, 0, 1), g_sceneCB.projectionToWorld);

    world.xyz /= world.w;
    origin = g_sceneCB.cameraPosition.xyz;
    direction = normalize(world.xyz - origin);
}

// Diffuse lighting calculation.
float4 CalculateDiffuseLighting(float3 hitPosition, float3 normal)
{
    float3 pixelToLight = normalize(g_sceneCB.lightPosition.xyz - hitPosition);

    // Diffuse contribution.
    float fNDotL = max(0.0f, dot(pixelToLight, normal));

    return g_cubeCB.albedo * g_sceneCB.lightDiffuseColor * fNDotL;
}

[shader("raygeneration")]
void MyRaygenShader()
{
    float3 rayDir;
    float3 origin;
    
    // Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    GenerateCameraRay(DispatchRaysIndex().xy, origin, rayDir);

    // Trace the ray.
    // Set the ray's extents.
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0) };
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);

    // Write the raytraced color to the output texture.
    RenderTarget[DispatchRaysIndex().xy] = payload.color;
}

[shader("closesthit")]
void HitGroup_MyClosestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    float3 hitPosition = HitWorldPosition();

    uint indexSizeInBytes = 4;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    uint baseIndex = PrimitiveIndex() * triangleIndexStride;
    const uint3 indices = Indices[0].Load3(baseIndex);

    // Retrieve corresponding vertex normals for the triangle vertices.
    float3 vertexNormals[3] = { 
        Vertices[0][indices[0]].normal, 
        Vertices[0][indices[1]].normal, 
        Vertices[0][indices[2]].normal 
    };

    // Compute the triangle's normal.
    // This is redundant and done for illustration purposes 
    // as all the per-vertex normals are the same and match triangle's normal in this sample. 
    float3 triangleNormal = HitAttribute(vertexNormals, attr);

    float4 diffuseColor = CalculateDiffuseLighting(hitPosition, triangleNormal);
    float4 color = g_sceneCB.lightAmbientColor + diffuseColor;

    payload.color = color;
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    float4 background = float4(0.0f, 0.2f, 0.4f, 1.0f);
    payload.color = background;
}

#endif // RAYTRACING_HLSL