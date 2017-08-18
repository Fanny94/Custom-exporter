#pragma once
#include "Linker.h"

struct MainHeader
{
	unsigned int MeshCount;
	unsigned int MaterialCount;
	unsigned int LightCount;
	unsigned int CameraCount;
	unsigned int SkeletonAnimationCount;
	unsigned int MorphCount;
};

struct VertexHeader
{
	float Pos[3];
	float Nor[3];
	float UV[2];
	float Tan[3] = { 0.0, 0.0, 0.0 };
	float BiTan[3] = { 0.0, 0.0, 0.0 };

};

struct Skinning
{
	float weights[4];
	unsigned int boneIndices[4];
};

struct MeshHeader
{
	char MeshName[256] = "";
	unsigned int ObjectID = 0;
	unsigned int ParentID = 0;

	unsigned int VertexCount = 0;
	unsigned int MaterialID = 0;

	unsigned int AttributeCount = 0;

	float Translation[3] = { 0.0, 0.0, 0.0 };
	float Rotation[3] = { 0.0, 0.0, 0.0 };
	float Scale[3] = { 1.0, 1.0, 1.0 };
};

struct JointHeader //one joint
{
	char jointName[256] = "";
	int ParentID = 0;
	unsigned int jointIndex = 0;
	unsigned int nrOfKeyframes = 0;
	float globalBindPoseInverse[16];
};

struct Keyframe
{
	float translation[4];
	float rotation[4];
	float scale[4];
	float keyTime;
};

struct Skeleton
{
	unsigned int numberOfJoints = 0; //The number of Jointheaders
	unsigned int numberOfSkinWeights = 0; /*as big as the number of control points, 
									  	containing weightdata for each control point*/

};

struct MaterialHeader
{
	unsigned int ID;
	unsigned int Count = 0;
	char MatName[256];

	float Diffuse[3];
	float Specular[3];
	float Ambient[3];
	float Transparency;
	float Shininess;
	float Reflection;
	char diffuseMap[256] = "";
};

struct Light
{
	unsigned int PointLightCount = 0;
	unsigned int SpotLightCount = 0;
	unsigned int DirectionalLightCount = 0;
};

struct PointLight
{
	float Intensity;
	float Color[3];
	float Position[3];
};

struct SpotLight
{
	float Intensity;
	float Color[3];
	float Position[3];
	float Rotation[3];
};

struct DirectionalLight
{
	float Intensity;
	float Color[3];
	float Position[3];
	float Rotation[3];
};

struct IndexHeader
{
	unsigned int Index;
	unsigned int VertexIndex;
};

struct CameraHeader
{
	float CamPos[3];
	float CamRotation[3];
};

struct MorphHeader
{
	float MorphShape[4];
};

struct MorphCountHeader
{
	unsigned int MorphShapeCount = 0;
};