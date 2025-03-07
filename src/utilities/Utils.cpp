#include "Utils.h"

using namespace forward;

float4 Colors::White = { 1.0f, 1.0f, 1.0f, 1.0f };
float4 Colors::Black = { 0.0f, 0.0f, 0.0f, 1.0f };
float4 Colors::Red = { 1.0f, 0.0f, 0.0f, 1.0f };
float4 Colors::Green = { 0.0f, 1.0f, 0.0f, 1.0f };
float4 Colors::Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
float4 Colors::Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
float4 Colors::Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
float4 Colors::Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
float4 Colors::Silver = { 0.75f, 0.75f, 0.75f, 1.0f };
float4 Colors::LightSteelBlue = { 0.69f, 0.77f, 0.87f, 1.0f };

float4x4 forward::ToFloat4x4(const Matrix4f& mat)
{
	float4x4 ret(mat[0], mat[1], mat[2], mat[3],
		mat[4], mat[5], mat[6], mat[7], 
		mat[8], mat[9], mat[10], mat[11], 
		mat[12], mat[13], mat[14], mat[15]);
	return ret;
}