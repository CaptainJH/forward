#include "CascadedShadowMapMatrixSet.h"

#undef max

using namespace forward;

CascadedShadowMapMatrixSet::CascadedShadowMapMatrixSet(const Camera& ca) 
	: m_bAntiFlickerOn(true)
	, m_fCascadeTotalRange(50.0f)
	, m_camera(ca)
{

}

CascadedShadowMapMatrixSet::~CascadedShadowMapMatrixSet()
{

}

void CascadedShadowMapMatrixSet::Init(i32 iShadowMapSize)
{
	m_iShadowMapSize = iShadowMapSize;

	// Set the range values
	m_arrCascadeRanges[0] = m_camera.getNearClip();
	m_arrCascadeRanges[1] = 10.0f;
	m_arrCascadeRanges[2] = 25.0f;
	m_arrCascadeRanges[3] = m_fCascadeTotalRange;

	for(auto i = 0; i < m_iTotalCascades; ++i)
	{
		m_arrCascadeBoundCenter[i] = Vector3f(0.0f, 0.0f, 0.0f);
		m_arrCascadeBoundRadius[i] = 0.0f;
	}
}

void CascadedShadowMapMatrixSet::Update(const Vector3f& vDirectionalDir)
{
	// Find the view matrix
	Vector3f vWorldCenter = m_camera.getWorldEyePos() + m_camera.getWorldLookingDir() * m_fCascadeTotalRange * 0.5f;
	Vector3f vPos = vWorldCenter;
	Vector3f vLookAt = vWorldCenter + vDirectionalDir * m_camera.getFarClip();
	Vector3f vRight = Vector3f(1.0f, 0.0f, 0.0f);
	auto vUp = Vector3f::Cross(vDirectionalDir, vRight);
	vUp.Normalize();
	auto matShadowView = Matrix4f::LookAtLHMatrix(vPos, vLookAt, vUp);

	// Get the bounds for the shadow space
	f32 fRadius;
	ExtractFrustumBoundSphere(m_arrCascadeRanges[0], m_arrCascadeRanges[3], m_vShadowBoundCenter, fRadius);
	m_fShadowBoundRadius = std::max(m_fShadowBoundRadius, fRadius); // Expend the radius to compensate for numerical errors

	// Find the projection matrix
	Matrix4f matShadowProj = Matrix4f::OrthographicLHMatrix(-m_fShadowBoundRadius, m_fShadowBoundRadius, m_fShadowBoundRadius, m_fShadowBoundRadius);

	// The combined transformation from world to shadow space
	m_WorldToShadowSpace = matShadowView * matShadowProj;

	// For each cascade find the transformation from shadow to cascade space
	Matrix4f matShadowViewInv = matShadowView.Transpose();
	for(auto iCascadeIdx = 0; iCascadeIdx < m_iTotalCascades; ++iCascadeIdx)
	{
		Matrix4f cascadeTrans;
		Matrix4f cascadeScale;
		if(m_bAntiFlickerOn)
		{
			// To avoid anti flickering we need to make the transformation invariant to camera rotation and translation
			// By encapsulating the cascade frustum with a sphere we achive the rotation invariance
			Vector3f vNewCenter;
			ExtractFrustumBoundSphere(m_arrCascadeRanges[iCascadeIdx], m_arrCascadeRanges[iCascadeIdx+1], vNewCenter, fRadius);
			m_arrCascadeBoundRadius[iCascadeIdx] = std::max(m_arrCascadeBoundRadius[iCascadeIdx], fRadius); // Expend the radius to compensate for numerical errors

			// Only update the cascade bounds if it moved at least a full pixel unit
			// This makes the transformation invariant to translation
			Vector3f vOffset;
			if(CascadeNeedsUpdate(matShadowView, iCascadeIdx, vNewCenter, vOffset))
			{
				// To avoid flickering we need to move the bound center in full units
				Vector3f vOffsetOut = (matShadowViewInv * Vector4f(vOffset, 0.0f)).xyz();
				//D3DXVec3TransformNormal( &vOffsetOut, &vOffset, &mShadowViewInv);
				m_arrCascadeBoundCenter[iCascadeIdx] += vOffsetOut;
			}

			// Get the cascade center in shadow space
			Vector3f vCascadeCenterShadowSpace = (m_WorldToShadowSpace * Vector4f(m_arrCascadeBoundCenter[iCascadeIdx], 1.0f)).xyz();
			//D3DXVec3TransformCoord(&vCascadeCenterShadowSpace, &m_arrCascadeBoundCenter[iCascadeIdx], &m_WorldToShadowSpace);

			// Update the translation from shadow to cascade space
			m_vToCascadeOffsetX[iCascadeIdx] = -vCascadeCenterShadowSpace.x;
			m_vToCascadeOffsetY[iCascadeIdx] = -vCascadeCenterShadowSpace.y;
			cascadeTrans.MakeIdentity();
			cascadeTrans.SetTranslation(Vector3f(m_vToCascadeOffsetX[iCascadeIdx], m_vToCascadeOffsetY[iCascadeIdx], 0.0f));
			//Matrix4fTranslation( &cascadeTrans, m_vToCascadeOffsetX[iCascadeIdx], m_vToCascadeOffsetY[iCascadeIdx], 0.0f );

			// Update the scale from shadow to cascade space
			m_vToCascadeScale[iCascadeIdx] = m_fShadowBoundRadius / m_arrCascadeBoundRadius[iCascadeIdx];
			cascadeScale.ScaleMatrix(Vector3f(m_vToCascadeScale[iCascadeIdx], m_vToCascadeScale[iCascadeIdx], 1.0f));
			//Matrix4fScaling( &cascadeScale, m_vToCascadeScale[iCascadeIdx], m_vToCascadeScale[iCascadeIdx], 1.0f );
		}
		else
		{
			// Since we don't care about flickering we can make the cascade fit tightly around the frustum
			// Extract the bounding box
			Vector3f arrFrustumPoints[8];
			ExtractFrustumPoints(m_arrCascadeRanges[iCascadeIdx], m_arrCascadeRanges[iCascadeIdx+1], arrFrustumPoints);

			// Transform to shadow space and extract the minimum andn maximum
			Vector3f vMin = Vector3f(FLT_MAX, FLT_MAX, FLT_MAX);
			Vector3f vMax = Vector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			for(auto i = 0; i < 8; i++)
			{
				Vector3f vPointInShadowSpace = (m_WorldToShadowSpace * Vector4f(arrFrustumPoints[i], 1.0f)).xyz();
				//D3DXVec3TransformCoord(&vPointInShadowSpace, &arrFrustumPoints[i], &m_WorldToShadowSpace);

				for(auto j = 0; j < 3; ++j)
				{
					if((&vMin.x)[j] > (&vPointInShadowSpace.x)[j])
						(&vMin.x)[j] = (&vPointInShadowSpace.x)[j];
					if((&vMax.x)[j] < (&vPointInShadowSpace.x)[j])
						(&vMax.x)[j] = (&vPointInShadowSpace.x)[j];
				}
			}

			Vector3f vCascadeCenterShadowSpace = (vMin + vMax) * 0.5f;

			// Update the translation from shadow to cascade space
			m_vToCascadeOffsetX[iCascadeIdx] = -vCascadeCenterShadowSpace.x;
			m_vToCascadeOffsetY[iCascadeIdx] = -vCascadeCenterShadowSpace.y;
			cascadeTrans.MakeIdentity();
			cascadeTrans.SetTranslation(Vector3f(m_vToCascadeOffsetX[iCascadeIdx], m_vToCascadeOffsetY[iCascadeIdx], 0.0f));
			//Matrix4fTranslation( &cascadeTrans, m_vToCascadeOffsetX[iCascadeIdx], m_vToCascadeOffsetY[iCascadeIdx], 0.0f );

			// Update the scale from shadow to cascade space
			m_vToCascadeScale[iCascadeIdx] = 2.0f / std::max(vMax.x - vMin.x, vMax.y - vMin.y);
			cascadeScale.ScaleMatrix(Vector3f(m_vToCascadeScale[iCascadeIdx], m_vToCascadeScale[iCascadeIdx], 1.0f));
			//Matrix4fScaling( &cascadeScale, m_vToCascadeScale[iCascadeIdx], m_vToCascadeScale[iCascadeIdx], 1.0f );
		}

		// Combine the matrices to get the transformation from world to cascade space
		m_arrWorldToCascadeProj[iCascadeIdx] = m_WorldToShadowSpace * cascadeTrans * cascadeScale;
	}

	// Set the values for the unused slots to someplace outside the shadow space
	for(auto i = m_iTotalCascades; i < 4; ++i)
	{
		m_vToCascadeOffsetX[i] = 250.0f;
		m_vToCascadeOffsetY[i] = 250.0f;
		m_vToCascadeScale[i] = 0.1f;
	}
}

void CascadedShadowMapMatrixSet::ExtractFrustumPoints(f32 fNear, f32 fFar, Vector3f* arrFrustumCorners)
{
	// Get the camera bases
	const Vector3f& camPos = m_camera.getWorldEyePos();
	const Vector3f& camRight = m_camera.getWorldRight();
	const Vector3f& camUp = m_camera.getWorldUp();
	const Vector3f& camForward = m_camera.getWorldLookingDir();

	// Calculate the tangent values (this can be cached
	const f32 fTanFOVX = tanf(m_camera.getAspectRatio() * m_camera.getFOV());
	const f32 fTanFOVY = tanf(m_camera.getAspectRatio());

	// Calculate the points on the near plane
	arrFrustumCorners[0] = camPos + (-camRight * fTanFOVX + camUp * fTanFOVY + camForward) * fNear;
	arrFrustumCorners[1] = camPos + (camRight * fTanFOVX + camUp * fTanFOVY + camForward) * fNear;
	arrFrustumCorners[2] = camPos + (camRight * fTanFOVX - camUp * fTanFOVY + camForward) * fNear;
	arrFrustumCorners[3] = camPos + (-camRight * fTanFOVX - camUp * fTanFOVY + camForward) * fNear;

	// Calculate the points on the far plane
	arrFrustumCorners[4] = camPos + (-camRight * fTanFOVX + camUp * fTanFOVY + camForward) * fFar;
	arrFrustumCorners[5] = camPos + (camRight * fTanFOVX + camUp * fTanFOVY + camForward) * fFar;
	arrFrustumCorners[6] = camPos + (camRight * fTanFOVX - camUp * fTanFOVY + camForward) * fFar;
	arrFrustumCorners[7] = camPos + (-camRight * fTanFOVX - camUp * fTanFOVY + camForward) * fFar;
}

void CascadedShadowMapMatrixSet::ExtractFrustumBoundSphere(f32 fNear, f32 fFar, Vector3f& vBoundCenter, f32& fBoundRadius)
{
	// Get the camera bases
	const Vector3f& camPos = m_camera.getWorldEyePos();
	const Vector3f& camRight = m_camera.getWorldRight();
	const Vector3f& camUp = m_camera.getWorldUp();
	const Vector3f& camForward = m_camera.getWorldLookingDir();

	// Calculate the tangent values (this can be cached as long as the FOV doesn't change)
	const float fTanFOVX = tanf(m_camera.getAspectRatio() * m_camera.getFOV());
	const float fTanFOVY = tanf(m_camera.getAspectRatio());

	// The center of the sphere is in the center of the frustum
	vBoundCenter = camPos + camForward * (fNear + 0.5f * (fNear + fFar));
	
	// Radius is the distance to one of the frustum far corners
	const Vector3f vBoundSpan = camPos + (-camRight * fTanFOVX + camUp * fTanFOVY + camForward) * fFar - vBoundCenter;
	fBoundRadius = vBoundSpan.Magnitude();
}

bool CascadedShadowMapMatrixSet::CascadeNeedsUpdate(const Matrix4f& matShadowView, i32 iCascadeIdx, const Vector3f& newCenter, Vector3f& vOffset)
{
	// Find the offset between the new and old bound ceter
	Vector3f vOldCenterInCascade = (matShadowView * Vector4f(m_arrCascadeBoundCenter[iCascadeIdx], 1.0f)).xyz();
	//D3DXVec3TransformCoord(&vOldCenterInCascade, &m_arrCascadeBoundCenter[iCascadeIdx], &matShadowView);
	Vector3f vNewCenterInCascade = (matShadowView * Vector4f(newCenter, 1.0f)).xyz();
	//D3DXVec3TransformCoord(&vNewCenterInCascade, &newCenter, &matShadowView);
	Vector3f vCenterDiff = vNewCenterInCascade - vOldCenterInCascade;

	// Find the pixel size based on the diameters and map pixel size
	f32 fPixelSize = (f32)m_iShadowMapSize / (2.0f * m_arrCascadeBoundRadius[iCascadeIdx]);

	f32 fPixelOffX = vCenterDiff.x * fPixelSize;
	f32 fPixelOffY = vCenterDiff.y * fPixelSize;

	// Check if the center moved at least half a pixel unit
	bool bNeedUpdate = abs(fPixelOffX) > 0.5f || abs(fPixelOffY) > 0.5f;
	if(bNeedUpdate)
	{
		// Round to the 
		vOffset.x = floorf(0.5f + fPixelOffX) / fPixelSize;
		vOffset.y = floorf(0.5f + fPixelOffY) / fPixelSize;
		vOffset.z = vCenterDiff.z;
	}
	
	return bNeedUpdate;
}
