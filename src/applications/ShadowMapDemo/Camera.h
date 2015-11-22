#pragma once
#include "Vector3f.h"
#include "Matrix4f.h"

namespace forward
{
	class Camera
	{
	public:
		Camera();
		~Camera() {}

		void update(f32 delta);
		void updateWithKeyboardInput(i8 key);
		void updateWithMouseInput(i32 x, i32 y);

		Matrix4f updateViewMatrix();
		void setViewMatrix(const Matrix4f& mat4);

		void pitch(f32 radians);
		void yaw(f32 radians);

		void setProjectionParams(f32 fov, f32 aspect, f32 nearPlane, f32 farPlane);
		const Matrix4f& getProjectionMatrix() const;


	protected:
		Vector3f m_pos;
		Matrix3f m_rot;

		f32 m_frameTime;

		f32 m_fov;
		f32 m_near, m_far;
		f32 m_aspect;
		Matrix4f m_projMat;
	};
}