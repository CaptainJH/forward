//***************************************************************************************
// FPCamera.h by Heqi Ju 2023
//   
// Simple first person style camera class that lets the viewer explore the 3D scene.
//   -It keeps track of the camera coordinate system relative to the world space
//    so that the view matrix can be constructed.  
//   -It keeps track of the viewing frustum of the camera so that the projection
//    matrix can be obtained.
//***************************************************************************************
#pragma once
#include "pCH.h"
#include "utilities/Utils.h"


namespace forward
{
	class FPCamera
	{
	public:

		FPCamera();
		~FPCamera() = default;

		// Get/Set world camera position.
		float3 GetPosition() const;
		void SetPosition(f32 x, f32 y, f32 z);
		void SetPosition(const float3 v);

		// Get camera basis vectors.
		float3 GetRight() const;
		float3 GetUp() const;
		float3 GetLook() const;

		// Get frustum properties.
		f32 GetNearZ() const;
		f32 GetFarZ() const;
		f32 GetAspect() const;
		f32 GetFovY() const;
		f32 GetFovX() const;

		// Get near and far plane dimensions in view space coordinates.
		f32 GetNearWindowWidth() const;
		f32 GetNearWindowHeight() const;
		f32 GetFarWindowWidth() const;
		f32 GetFarWindowHeight() const;

		// Set frustum.
		void SetLens(f32 fovY, f32 aspect, f32 zn, f32 zf);

		// Define camera space via LookAt parameters.
		void LookAt(const float3 pos, const float3 target, const float3 up);

		// Get View/Proj matrices.
		float4x4 GetViewMatrix() const;
		float4x4 GetProjectionMatrix() const;

		// Strafe/Walk the camera a distance d.
		void Strafe(f32 d);
		void Walk(f32 d);

		// Rotate the camera.
		void Pitch(f32 angle);
		void RotateY(f32 angle);

		// After modifying camera position/orientation, call to rebuild the view matrix.
		bool UpdateViewMatrix();

	private:

		// Camera coordinate system with coordinates relative to world space.
		float3 mPosition = { 0.0f, 0.0f, -5.0f };
		float3 mRight = { 1.0f, 0.0f, 0.0f };
		float3 mUp = { 0.0f, 1.0f, 0.0f };
		float3 mLook = { 0.0f, 0.0f, 1.0f };

		// Cache frustum properties.
		f32 mNearZ = 0.0f;
		f32 mFarZ = 0.0f;
		f32 mAspect = 0.0f;
		f32 mFovY = 0.0f;
		f32 mNearWindowHeight = 0.0f;
		f32 mFarWindowHeight = 0.0f;

		bool mViewDirty = true;

		// Cache View/Proj matrices.
		float4x4 mView;
		float4x4 mProj;
	};
}