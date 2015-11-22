#include "Camera.h"

#pragma warning( disable : 4189 )

using namespace forward;

Camera::Camera()
	: m_frameTime(0.0f)
{
	m_pos.MakeZero();
	m_rot.MakeIdentity();
}

void Camera::update(f32 delta)
{
	m_frameTime = delta;
}

void Camera::updateWithKeyboardInput(i8 key)
{
	const f32 moveSpeed = 0.6f;

	if (key == 'w')
	{
		Vector3f zAxis = m_rot.GetRow(2);
		m_pos += zAxis * moveSpeed * m_frameTime;
	}
	else if (key == 's')
	{
		Vector3f zAxis = m_rot.GetRow(2);
		m_pos -= zAxis * moveSpeed * m_frameTime;
	}
	else if (key == 'a')
	{
		Vector3f xAxis = m_rot.GetRow(0);
		m_pos -= xAxis * moveSpeed * m_frameTime;
	}
	else if (key == 'd')
	{
		Vector3f xAxis = m_rot.GetRow(0);
		m_pos += xAxis * moveSpeed * m_frameTime;
	}
}

void Camera::updateWithMouseInput(i32 x, i32 y)
{
	const float mouseMoveSpeed = 0.0010f;
	const float rotY = x * mouseMoveSpeed;
	const float rotX = y * mouseMoveSpeed;

	pitch(rotX);
	yaw(rotY);
}

void Camera::pitch(f32 radians)
{
	Matrix3f mat;
	mat.MakeIdentity();
	mat.RotationEuler(m_rot.GetRow(0), radians);

	m_rot.SetRow(1, mat * m_rot.GetRow(1));
	m_rot.SetRow(2, mat * m_rot.GetRow(2));
}

void Camera::yaw(f32 radians)
{
	Matrix3f mat;
	mat.MakeIdentity();
	mat.RotationEuler(m_rot.GetRow(1), radians);

	m_rot.SetRow(0, mat * m_rot.GetRow(0));
	m_rot.SetRow(1, mat * m_rot.GetRow(1));
	m_rot.SetRow(2, mat * m_rot.GetRow(2));
}

void Camera::setViewMatrix(const Matrix4f& mat4)
{
	auto inv = mat4.Inverse();
	m_pos = inv.GetTranslation();
	m_rot = inv.GetRotation();
}

Matrix4f Camera::upViewMatrix()
{
	Matrix4f viewMat;

	Vector3f dir = m_rot.GetRow(2);
	Vector3f target = m_pos + dir;
	Vector3f up(0.0f, 1.0f, 0.0f);
	viewMat = Matrix4f::LookAtLHMatrix(m_pos, target, up);
	setViewMatrix(viewMat);

	return viewMat;
}

void Camera::setProjectionParams(f32 fov, f32 aspect, f32 nearPlane, f32 farPlane)
{
	m_fov = fov;
	m_near = nearPlane;
	m_far = farPlane;
	m_aspect = aspect;

	m_projMat = Matrix4f::PerspectiveFovLHMatrix(m_fov, m_aspect, m_near, m_far);
}

const Matrix4f& Camera::getProjectionMatrix() const
{
	return m_projMat;
}