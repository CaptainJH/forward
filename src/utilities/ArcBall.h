//--------------------------------------------------------------------------------------
// File: ArcBall.h
//
// Ken Shoemake, "Arcball Rotation Control", Graphics Gems IV, pg 176 - 192  
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//--------------------------------------------------------------------------------------

#pragma once
#include <math/Quaternion.h>
#include <math/Vector3f.h>
#include <math/Vector4f.h>

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
        m_qdown = m_qnow;// = DirectX::SimpleMath::Quaternion::Identity;
    }

    void OnBegin( int x, int y )
    {
        m_drag = true;
        m_qdown = m_qnow;
        m_downPoint = ScreenToVector(float(x), float(y));
    }

    void OnMove(int x, int y)
    {
        if (m_drag)
        {
            auto curr = ScreenToVector(float(x), float(y));

            //m_qnow = XMQuaternionMultiply(m_qdown, QuatFromBallPoints(m_downPoint, curr));
            //m_qnow.Normalize();
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

    forward::Quaternion<float> GetQuat() const { return m_qnow; }

    bool IsDragging() const { return m_drag; }

private:
    float                           m_width;
    float                           m_height;
    float                           m_radius;
    forward::Quaternion<float> m_qdown;
    forward::Quaternion<float> m_qnow;
    forward::Vector4f    m_downPoint;
    bool                            m_drag;

    forward::Vector4f ScreenToVector(float screenx, float screeny)
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

        return forward::Vector4f( x, y, z, 0 );
    }

    static forward::Vector3f QuatFromBallPoints(forward::Vector3f vFrom, forward::Vector3f vTo )
    {
        //XMVECTOR dot = XMVector3Dot( vFrom, vTo );
        //XMVECTOR vPart = XMVector3Cross( vFrom, vTo );
        //return XMVectorSelect( dot, vPart, g_XMSelect1110 );
        return vFrom;
    }
};