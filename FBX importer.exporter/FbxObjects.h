#pragma once
#ifndef FBXOBJECTS_H
#define FBXOBJECTS_H
#include "Linker.h"
#include "Write.h"
#include "Structs.h"

struct BlendInfo
{
	std::vector<float> boneWeights;
	std::vector<unsigned int> boneIndices;
};

class FBX
{
private:
	FbxManager* PManager;
	FbxScene* PScene;
	Write write;
	PointLight* pLight;
	SpotLight* sLight;
	DirectionalLight* dLight;
	FbxSurfaceMaterial* PMaterial;
	FbxNode* ParentNode;

	vector<FbxVector4 *> BlndVert;

	FbxAnimStack* animStack;
	FbxAnimLayer* currLayer;
	unsigned int numAnimLayers;
	FbxTime timePeriod;
	FbxString stackName;
	FbxTakeInfo* takeInfo;
	FbxTime frameStart, frameStop;

	FbxSkin* deformer;
	unsigned int deformerCount;
	std::vector<JointHeader> joints;
	float frameRate;
	std::map<unsigned int, BlendInfo> blendInfoPerVertexIndex;

public:
	FBX();
	~FBX();

	

	void SDKInitialization(FbxManager*& PManager, FbxScene*& PScene);
	FbxMesh* LoadScene(FbxManager* PManager, FbxScene* PScene);
	void SetJointHierarchy(FbxNode* PNode);
	void ImportVertices(FbxMesh* PMesh, vector<MeshHeader>* OutPutMesh, vector<VertexHeader>* OutVertexVector);
	void ImportNormals(FbxMesh* PMesh, vector<VertexHeader>* OutVertexVector);
	void ImportUV(FbxMesh* PMesh, vector<VertexHeader>* OutVertexVector);
	void ImportTangents(FbxMesh* Pmesh, vector<VertexHeader>* OutVertexVector);
	void ImportTexture(FbxMesh* pMesh, MaterialHeader* OutMaterial);
	void ImportCamera(FbxNode * PNode, vector<CameraHeader>* OutCameraVector);
	void ImportLight(FbxNode * PNode, Light* OutLight, vector<PointLight>* OutPointVector, vector<SpotLight>* OutSpotVector, vector<DirectionalLight>* OutDirectionalVector);
	void ImportMorphAnimation(FbxMesh* PMesh, vector<MorphHeader>* OutMorph, MorphCountHeader* OutMorphCount);
	void Initialization();
	void ImportMaterial(FbxMesh* PMesh, MaterialHeader* OutMaterial);
	DirectX::XMMATRIX convertFbxMatrixToXMMatrix(FbxAMatrix input);
	unsigned int findJointIndexByName(const char* jointName);
	void convertFromFloat4X4ToFloat16(DirectX::XMFLOAT4X4 matrix, int &currentJointIndex);
	void ImportSkeleton(FbxMesh * PMesh, FbxScene* PScene, Skeleton* OutSkeleton, vector<JointHeader>* OutJointVector,
		vector <Skinning> &OutSkinPerCtrlPoint, map<int, vector<Keyframe>> &OutKeyframes);
	void recursiveHierarchy(FbxNode * PNode, int index, int jointParentID);

};

#endif // !FBXOBJECTS_H
