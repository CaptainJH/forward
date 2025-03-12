//--------------------------------------------------------------------------------------
// File: ArcBall.h
//
// Ken Shoemake, "Arcball Rotation Control", Graphics Gems IV, pg 176 - 192  
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//--------------------------------------------------------------------------------------

#pragma once
#include <ImathQuat.h>

class ArcBall
{
public:
    ArcBall() :
        m_width(800.f),
        m_height(400.f),
        m_radius(1.f),
        m_drag(false) { Reset();  }

    void Reset()
    {
        m_qdown = m_qnow = Imath::Quatf::identity();
    }

    void OnBegin( float x, float y )
    {
        m_drag = true;
        m_qdown = m_qnow;
        m_downPoint = ScreenToVector(x, y);
    }

    void OnMove(float x, float y)
    {
        if (m_drag)
        {
            auto curr = ScreenToVector(x, y);

            m_qnow = m_qdown * QuatFromBallPoints(m_downPoint, curr);
            m_qnow.normalize();
        }
    }

    void OnEnd()
    {
        m_drag = false;
    }

    void SetWindow(int width, int height)
    {
        m_width = float(width);
        m_height = float(height);
    }

    void SetRadius(float radius)
    {
        m_radius = radius;
    }

    Imath::Quatf GetQuat() const { return m_qnow; }

    bool IsDragging() const { return m_drag; }

private:
    float                           m_width;
    float                           m_height;
    float                           m_radius;
    Imath::Quatf m_qdown;
    Imath::Quatf m_qnow;
    forward::float3       m_downPoint;
    bool                            m_drag;

    forward::float3 ScreenToVector(float screenx, float screeny)
    {
        float x = -( screenx - m_width / 2.f ) / ( m_radius * m_width / 2.f );
        float y = ( screeny - m_height / 2.f ) / ( m_radius * m_height / 2.f );

        float z = 0.0f;
        float mag = x * x + y * y;

        if( mag > 1.0f )
        {
            float scale = 1.0f / sqrtf( mag );
            x *= scale;
            y *= scale;
        }
        else
            z = sqrtf( 1.0f - mag );

        return forward::float3( x, y, z);
    }

    static Imath::Quatf QuatFromBallPoints(forward::float3 vFrom, forward::float3 vTo )
    {
        auto dot = vFrom.dot(vTo);
        auto vPart = vTo.cross(vFrom);
        return Imath::Quatf(dot, vPart);
    }
};