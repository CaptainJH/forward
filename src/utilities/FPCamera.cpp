#include "FPCamera.h"

using namespace forward;

FPCamera::FPCamera()
{
	SetLens(f_PIDIV4, 16.0f / 9.0f, 1.0f, 1000.0f);
}

float3 FPCamera::GetPosition() const
{
	return mPosition;
}

void FPCamera::SetPosition(f32 x, f32 y, f32 z)
{
	mPosition = float3(x, y, z);
	mViewDirty = true;
}

void FPCamera::SetPosition(const float3 v)
{
	mPosition = v;
	mViewDirty = true;
}

float3 FPCamera::GetRight() const
{
	return mRight;
}

float3 FPCamera::GetUp()const
{
	return mUp;
}

float3 FPCamera::GetLook()const
{
	return mLook;
}

f32 FPCamera::GetNearZ() const
{
	return mNearZ;
}

f32 FPCamera::GetFarZ() const
{
	return mFarZ;
}

f32 FPCamera::GetAspect() const
{
	return mAspect;
}

f32 FPCamera::GetFovY() const
{
	return mFovY;
}

f32 FPCamera::GetFovX() const
{
	auto halfWidth = 0.5f * GetNearWindowWidth();
	return 2.0f * atan(halfWidth / mNearZ);
}

f32 FPCamera::GetNearWindowWidth() const
{
	return mAspect * mNearWindowHeight;
}

f32 FPCamera::GetNearWindowHeight() const
{
	return mNearWindowHeight;
}

f32 FPCamera::GetFarWindowWidth() const
{
	return mAspect * mFarWindowHeight;
}

f32 FPCamera::GetFarWindowHeight() const
{
	return mFarWindowHeight;
}

void FPCamera::SetLens(f32 fovY, f32 aspect, f32 zn, f32 zf)
{
	// cache properties
	mFovY = fovY;
	mAspect = aspect;
	mNearZ = zn;
	mFarZ = zf;

	mNearWindowHeight = 2.0f * mNearZ * tanf(0.5f * mFovY);
	mFarWindowHeight = 2.0f * mFarZ * tanf(0.5f * mFovY);

	mProj = ToFloat4x4(Matrix4f::PerspectiveFovLHMatrix(fovY, mAspect, mNearZ, mFarZ));
}

void FPCamera::LookAt(const float3 pos, const float3 target, const float3 up)
{
	mLook = (target - pos).normalize();
	mRight = up.cross(mLook).normalize();
	mUp = mLook.cross(mRight);

	mViewDirty = true;
}

float4x4 FPCamera::GetViewMatrix() const
{
	assert(!mViewDirty);
	return mView;
}

float4x4 FPCamera::GetProjectionMatrix() const
{
	return mProj;
}


void FPCamera::Strafe(f32 d)
{
	mPosition += d * mRight;
	mViewDirty = true;
}

void FPCamera::Walk(f32 d)
{
	mPosition += d * mLook;
	mViewDirty = true;
}

void FPCamera::Pitch(f32 angle)
{
	// Rotate up and look vector about the right vector.
	float4x4 rotMat;
	rotMat.setAxisAngle(mRight, angle * f_PI / 180.0f);
	mUp = mUp * rotMat;
	mLook = mLook * rotMat;

	mViewDirty = true;
}

void FPCamera::RotateY(f32 angle)
{
	// Rotate the basis vectors about the world y-axis.
	float4x4 rotMat;
	rotMat.setAxisAngle(float3(0, 1, 0), angle * f_PI / 180.0f);

	mRight = mRight * rotMat;
	mUp = mUp * rotMat;
	mLook = mLook * rotMat;

	mViewDirty = true;
}

void FPCamera::UpdateViewMatrix()
{
	if (mViewDirty)
	{
		float3 R = mRight;
		float3 U = mUp;
		float3 L = mLook;
		float3 P = mPosition;

		// Keep camera's axes orthogonal to each other and of unit length.
		L = L.normalize();
		U = L.cross(R).normalize();

		// U, L already ortho-normal, so no need to normalize cross product.
		R = U.cross(L);

		// Fill in the view matrix entries.
		auto x = -P.dot(R);
		auto y = -P.dot(U);
		auto z = -P.dot(L);

		mRight = R;
		mUp = U;
		mLook = L;

		mView = {
			mRight.x, mUp.x, mLook.x, 0.0f,
			mRight.y, mUp.y, mLook.y, 0.0f,
			mRight.z, mUp.z, mLook.z, 0.0f,
			x, y, z, 1.0f,
		};

		mViewDirty = false;
	}
}