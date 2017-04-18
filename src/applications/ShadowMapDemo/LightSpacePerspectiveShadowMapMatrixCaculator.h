#pragma once
#include "PCH.h"
#include "Matrix4f.h"
#include "Camera.h"
#include "Pipeline\Executors\GeometryDX11.h"

namespace forward
{
	class LightSpacePerspectiveShadowMapMatrixCaculator
	{
	public:
		LightSpacePerspectiveShadowMapMatrixCaculator(const Camera& lc, const Camera& vc);
		~LightSpacePerspectiveShadowMapMatrixCaculator() {}

		void addSceneObjects(const GeometryDX11* obj);
		void linkWorldMatrixOfObject(const GeometryDX11* obj, const Matrix4f* worldMat);
		Matrix4f update();
		const Matrix4f& getMatrix() const { return m_finalMat; }

	private:
		const Camera& m_lightCamera;
		const Camera& m_viewCamera;

		std::vector<const GeometryDX11*> m_sceneObjects;
		std::map<const GeometryDX11*, const Matrix4f*> m_matrixLinks;

		Matrix4f m_finalMat;

	private:
		Vector3f getNearCameraPointE() const;
		Vector3f getProjViewDir_ls(const Matrix4f& lightSpace) const;
		Matrix4f getLispSmMtx(const Matrix4f& lightSpace) const;
		std::pair<Vector3f, Vector3f> computeBoundingBox(const Matrix4f& mat) const;
		f32 calcNoptGeneral(const Matrix4f& lightSpace, const std::pair<Vector3f, Vector3f>& B_ls) const;
		Vector3f getZ0_ls(const Matrix4f& lightSpace, const Vector3f& e, const f32& b_lsZmax, const Vector3f& eyeDir) const;

	private:
		/// these are the intermedia caculation result. Outside user should never touch it
		Vector3f _E; // store the closest vertex to the near plane of view camera in world space

	};
}