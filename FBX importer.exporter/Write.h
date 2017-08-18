#pragma once
#ifndef WRITE_H
#define WRITE_H
#include "Structs.h"
#include "Linker.h"

class Write
{
public:
	vector<VertexHeader> OutVertexVector;
	vector<MeshHeader> OutPutMesh;
	vector<CameraHeader> OutCameraVector;
	vector<MorphHeader> OutMorphVector;
	MaterialHeader OutMaterial;
	MainHeader MainH;
	Light OutLight;
	MorphCountHeader OutMorphCount;
	vector<PointLight> OutPointVector;
	vector<SpotLight> OutSpotVector;
	vector<DirectionalLight> OutDirectionalVector;

	Skeleton OutSkeleton;
	vector<JointHeader> OutJointVector;
	vector<Skinning> OutSkinPerCtrlPoint; //one weight affecting for each control point
	map<int, vector<Keyframe>> OutKeyframes;

	void writeBinary();
	Write();
	~Write();
};

#endif // !WRITE_H