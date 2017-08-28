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
	unsigned int VertexCount;
	unsigned int MaterialID;
	unsigned int ObjectID;
	unsigned int ParentID;
	char MeshName[256] = "";
	unsigned int AttributeCount;
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