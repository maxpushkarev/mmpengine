#ifndef MMPENGINE_FRONTEND_SHADER
#define MMPENGINE_FRONTEND_SHADER 1

#if MMPENGINE_HLSL

struct CameraData
{
	float4x4 viewMat;
	float4x4 projMat;
	float4 worldPosition;
};

struct MeshRendererData
{
	float4x4 worldMat;
	float4x4 worldMatIT;
};

#endif


#endif