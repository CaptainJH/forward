#pragma once
#include "PCH.h"
#include "Matrix4f.h"
#include "Camera.h"

namespace forward
{
	class CascadedShadowMapMatrixSet
	{
	public:

		CascadedShadowMapMatrixSet(const Camera& ca);
		~CascadedShadowMapMatrixSet();

		void Init(i32 iShadowMapSize);

		void Update(const Vector3f& vDirectionalDir);

		// Change the antiflicker state
		void SetAntiFlicker(bool bIsOn) { m_bAntiFlickerOn = bIsOn; }

		const Matrix4f& GetWorldToShadowSpace() const { return m_WorldToShadowSpace; }
		const Matrix4f& GetWorldToCascadeProj(i32 i) const { return m_arrWorldToCascadeProj[i]; }
		const Vector4f GetToCascadeOffsetX() const { return m_vToCascadeOffsetX; }
		const Vector4f GetToCascadeOffsetY() const { return m_vToCascadeOffsetY; }
		const Vector4f GetToCascadeScale() const { return m_vToCascadeScale; }

		static const i32 m_iTotalCascades = 3;

	private:

		// Extract the frustum corners for the given near and far values
		void ExtractFrustumPoints(f32 fNear, f32 fFar, Vector3f* arrFrustumCorners);

		// Extract the frustum bounding sphere for the given near and far values
		void ExtractFrustumBoundSphere(f32 fNear, f32 fFar, Vector3f& vBoundCenter, f32& fBoundRadius);

		// Test if a cascade needs an update
		bool CascadeNeedsUpdate(const Matrix4f& mShadowView, i32 iCascadeIdx, const Vector3f& newCenter, Vector3f& vOffset);

		bool m_bAntiFlickerOn;
		i32 m_iShadowMapSize;
		f32 m_fCascadeTotalRange;
		std::array<f32, m_iTotalCascades + 1> m_arrCascadeRanges;

		Vector3f m_vShadowBoundCenter;
		f32 m_fShadowBoundRadius;
		std::array<Vector3f, m_iTotalCascades> m_arrCascadeBoundCenter;
		std::array<f32, m_iTotalCascades> m_arrCascadeBoundRadius;

		Matrix4f m_WorldToShadowSpace;
		std::array<Matrix4f, m_iTotalCascades> m_arrWorldToCascadeProj;

		Vector4f m_vToCascadeOffsetX;
		Vector4f m_vToCascadeOffsetY;
		Vector4f m_vToCascadeScale;

		const Camera& m_camera;
	};
}