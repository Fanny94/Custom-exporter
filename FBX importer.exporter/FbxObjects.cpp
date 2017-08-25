#include "FbxObjects.h"
#define KFBX_DLLINFO

FBX::FBX()
{
	write.MainH.CameraCount = 0;
}

FBX::~FBX()
{
	PManager->Destroy();
}

void FBX::SDKInitialization(FbxManager*& PManager, FbxScene*& PScene)
{
	// Initializing the SDK Manager, it handles all the memory managment
	PManager = FbxManager::Create();

	if (!PManager) //If the manager isn't initialized.
	{
		FBXSDK_printf("Error: Can't create FBX Manager!\n");
		exit(1); //Exit the application.
	}

	else
	{
		// Create the IO settings object. Holds the settings for import/export settings.
		FbxIOSettings *ios = FbxIOSettings::Create(PManager, IOSROOT);

		PManager->SetIOSettings(ios);
		
		PScene = FbxScene::Create(PManager, "myScene"); //Creates the scene

		if (!PScene) //If scene isn't initialized.
		{
			FBXSDK_printf("Error: Can't create FBX Scene\n");
			exit(1); //Exit the application.
		}
	}
}

FbxMesh* FBX::LoadScene(FbxManager* PManager, FbxScene* PScene)
{
	FbxImporter* MyImporter = FbxImporter::Create(PManager, "My Importer");

	FbxMesh* MyMesh = nullptr;

	bool ImporterStat = MyImporter->Initialize("../fbx_files/Ship1.fbx", -1, PManager->GetIOSettings());

	if(!ImporterStat) //If the importer can't be initialized.
	{
		FbxString error = MyImporter->GetStatus().GetErrorString();
		FBXSDK_printf("Error: Can't Initialize importer");
		exit(2);
	} 

	PScene = FbxScene::Create(PManager, "My Scene");

	ImporterStat = MyImporter->Import(PScene);
	if (!ImporterStat) //If the scene can't be opened.
	{
		FBXSDK_printf("Error: Cant import the created scene.");
		exit(3);
	}

	//Get root node of the hierarchy of nodes
	ParentNode = PScene->GetRootNode();

	if (ParentNode)
	{
		SetJointHierarchy(ParentNode);

		//loop through each children in the hierarchy
		for (int t = 0; t < ParentNode->GetChildCount(); t++)
		{
			FbxNode* currentChildNode = ParentNode->GetChild(t);
			
			if (currentChildNode->GetNodeAttribute() == NULL)
				continue;

			//A scene element (mesh, camera, light): defined by combining FBXNode with a subclass of FBXNodeAttribute
			//FBXAttribute define a scene element with a specific pos, rot and scale
			FbxNodeAttribute::EType AttributeType = currentChildNode->GetNodeAttribute()->GetAttributeType();

			if (AttributeType != FbxNodeAttribute::eMesh)
				continue;

			MyMesh = (FbxMesh*)currentChildNode->GetNodeAttribute();
			
			deformerCount = MyMesh->GetDeformerCount(FbxDeformer::eSkin);
			//if the mesh have a deformer, it has a animation. Otherwise, skip this
			if (deformerCount > 0)
			{

				//stack = a collection of animation layers
				//In this case we only have one animationstack with one layer
				//for multiple layers we can find them by iterating throught each anim stack 
				animStack = PScene->GetSrcObject<FbxAnimStack>(0);

				stackName = animStack->GetName();
				
				numAnimLayers = animStack->GetMemberCount<FbxAnimLayer>();

				currLayer = animStack->GetMember<FbxAnimLayer>(0);

				currLayer->Weight = 0;
				currLayer->Mute = false;
				currLayer->Solo = false;

				//get info from the scene to be used later in the animation import
				timePeriod.SetTime(0, 0, 0, 1, 0, PScene->GetGlobalSettings().GetTimeMode());

				frameRate = FbxTime::GetFrameRate(PScene->GetGlobalSettings().GetTimeMode());
				takeInfo = PScene->GetTakeInfo(stackName);

				if (takeInfo)
				{
					frameStart = takeInfo->mLocalTimeSpan.GetStart();
					frameStop = takeInfo->mLocalTimeSpan.GetStop();
				}

				else
				{
					FbxTimeSpan timeSpan;
					PScene->GetGlobalSettings().GetTimelineDefaultTimeSpan(timeSpan);

					frameStart = timeSpan.GetStart();
					frameStop = timeSpan.GetStop();
				}
			}
			else 
			{
				continue;
			}

		}
	}

	return MyMesh;
	MyImporter->Destroy();
}

void FBX::SetJointHierarchy(FbxNode* rootNode)
{
	int i;
	for (i = 0; i < rootNode->GetChildCount(); i++)
	{
		// -1 = parentnode index, 0 = jointindex, starting at 0 
		recursiveHierarchy(rootNode->GetChild(i), 0, -1);
	}
}

void FBX::recursiveHierarchy(FbxNode * PNode, int index, int jointParentID)
{
	//Gather joints for each node (PNode)

	//if the node holds a skeleton, if not, skip this 
	if (PNode->GetNodeAttribute() && PNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		JointHeader currJoint;
		string jointName;
		jointName = PNode->GetName();
		for (size_t i = 0; i < jointName.size(); i++)
		{
			currJoint.jointName[i] = jointName[i];
		}
		currJoint.ParentID = jointParentID;
		currJoint.jointIndex = index;

		write.OutSkeleton.numberOfJoints++;
		joints.push_back(currJoint);
	}
	for (int i = 0; i < PNode->GetChildCount(); i++)
	{
		//recursively loop through each node in the hierarchy
		recursiveHierarchy(PNode->GetChild(i), joints.size(), index);
	}
}

void FBX::ImportVertices(FbxMesh* PMesh,vector<MeshHeader>* OutPutMesh, vector<VertexHeader>* OutVertexVector)
{
	MeshHeader MeshH;
	int NumVex = 0;
	int NumberVertices;
	FbxVector4* vertices = PMesh->GetControlPoints();

	string name;
	FbxNode* n = PMesh->GetNode();
	name = n->GetName();

	for (size_t i = 0; i < name.size(); i++)
	{
		MeshH.MeshName[i] = name[i];
	}

	write.MainH.MeshCount++;

	for (int j = 0; j < PMesh->GetPolygonCount(); j++)
	{
		NumberVertices = PMesh->GetPolygonSize(j);
		NumVex += NumberVertices;

		assert(NumberVertices >= 0);

		for (int i = 0; i < NumberVertices; i++)
		{
			//ControlPoints are vertices for each polygon
			int ControlPointIndices = PMesh->GetPolygonVertex(j, i);

			VertexHeader data;

			//Getting vertex positions
			data.Pos[0] = (float)vertices[ControlPointIndices].mData[0];
			data.Pos[1] = (float)vertices[ControlPointIndices].mData[1];
			data.Pos[2] = -(float)vertices[ControlPointIndices].mData[2];

			//Sending the data to the VertexHeader
			OutVertexVector->push_back(data);
		}
	}

	MeshH.VertexCount = NumVex;
	OutPutMesh->push_back(MeshH);
}

void FBX::ImportNormals(FbxMesh* PMesh, vector<VertexHeader>* OutVertexVector)
{
	//Get the normal element of a mesh
	FbxGeometryElementNormal* NormalEle = PMesh->GetElementNormal();

	if (NormalEle)
	{
		int PolygonVerInd = 0;

		//Get Normals of each polygon, since the mapping mode for normal element is Polygon-Vertex
		for (int PolygonInd = 0; PolygonInd < PMesh->GetPolygonCount(); PolygonInd++)
		{
			//Get Polygon size, to find out the amount of vertexes in the current polygon
			int PolygonSize = PMesh->GetPolygonSize(PolygonInd);

			for (int t = 0; t < PolygonSize; t++) //Get all vertexes of the polygon
			{
				int NormalInd = 0;

				//Reference mode is direct since normal index == polygonvertexindex
				if (NormalEle->GetReferenceMode() == FbxGeometryElement::eDirect)
				{
					NormalInd = PolygonVerInd;
				}

				//Reference mode is index to direct, getting normals index to direct
				if (NormalEle->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
				{
					NormalInd = NormalEle->GetIndexArray().GetAt(PolygonVerInd);
				}

				FbxVector4 Normals = NormalEle->GetDirectArray().GetAt(NormalInd);

				OutVertexVector->at(PolygonVerInd).Nor[0] = Normals.mData[0];
				OutVertexVector->at(PolygonVerInd).Nor[1] = Normals.mData[1];
				OutVertexVector->at(PolygonVerInd).Nor[2] = -Normals.mData[2];

				PolygonVerInd++;
			}
		}
	}
}

void FBX::ImportUV(FbxMesh* PMesh, vector<VertexHeader>* OutVertexVector)
{
	FbxStringList UVNameList;
	PMesh->GetUVSetNames(UVNameList);

	for (int SetIndex = 0; SetIndex < UVNameList.GetCount(); SetIndex++)
	{
		const char* UVSetName = UVNameList.GetStringAt(SetIndex);
		const FbxGeometryElementUV* UVEle = PMesh->GetElementUV(UVSetName);

		if (!UVEle)
			continue;

		if (UVEle->GetMappingMode() != FbxGeometryElement::eByPolygonVertex && UVEle->GetMappingMode() != FbxGeometryElement::eByControlPoint)
			return;

		const bool UseIndex = UVEle->GetReferenceMode() != FbxGeometryElement::eDirect && UVEle->GetReferenceMode() == FbxGeometryElement::eIndexToDirect;

		const int IndexCounter = (UseIndex) ? UVEle->GetIndexArray().GetCount() : 0;

		const int PolyCounter = PMesh->GetPolygonCount();

		if (UVEle->GetMappingMode() == FbxGeometryElement::eByControlPoint)
		{
			for (int PolyIndex = 0; PolyIndex < PolyCounter; PolyIndex++)
			{
				const int PolySize = PMesh->GetPolygonSize(PolyIndex);

				for (int VertexIndex = 0; VertexIndex < PolySize; VertexIndex++)
				{
					FbxVector2 uv;

					int PolyVertexIndex = PMesh->GetPolygonVertex(PolyIndex, VertexIndex);

					int UVIndex = UseIndex ? UVEle->GetIndexArray().GetAt(UVIndex) : PolyVertexIndex;

					uv = UVEle->GetDirectArray().GetAt(UVIndex);

					OutVertexVector->at(VertexIndex).UV[0] = uv.mData[0];
					OutVertexVector->at(VertexIndex).UV[1] = 1 - uv.mData[1];
				
				}
			}
		}
		else if (UVEle->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			int PolygonIndCounter = 0;
			for (int PolygonInd = 0; PolygonInd < PolyCounter; PolygonInd++)
			{
				const int PolygonSize = PMesh->GetPolygonSize(PolygonInd);
				for (int VertexIndex = 0; VertexIndex < PolygonSize; VertexIndex++)
				{
					FbxVector2 uv;

					int UVIndex = UseIndex ? UVEle->GetIndexArray().GetAt(PolygonIndCounter) : PolygonIndCounter;

					uv = UVEle->GetDirectArray().GetAt(UVIndex);

					OutVertexVector->at(PolygonIndCounter).UV[0] = uv.mData[0];
					OutVertexVector->at(PolygonIndCounter).UV[1] = 1 - uv.mData[1];

					PolygonIndCounter++;
				}
			}
		}
	}
}

void FBX::ImportTangents(FbxMesh* PMesh, vector<VertexHeader>* OutVertexVector)//FbxGeometryElementTangent* PTElement, int index, Float* EndTangent)
{
	FbxGeometryElementTangent* TanEle;
	VertexHeader ver;
	for(int t = 0; t < PMesh->GetElementTangentCount(); t++)
	{
		FbxGeometryElementTangent* TanEle = PMesh->GetElementTangent(t);

		if(TanEle->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			for (int VertIndex = 0; VertIndex < PMesh->GetControlPointsCount(); VertIndex++)
			{
				int TangentIndex = 0;

				if (TanEle->GetReferenceMode() == FbxGeometryElement::eDirect)
				{
					TangentIndex = VertIndex;
				}

				if (TanEle->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
				{
					TangentIndex = TanEle->GetIndexArray().GetAt(TangentIndex);
				}
			
				FbxVector4 Tangents = TanEle->GetDirectArray().GetAt(TangentIndex);

				OutVertexVector->at(VertIndex).Tan[0] = Tangents.mData[0];
				OutVertexVector->at(VertIndex).Tan[1] = Tangents.mData[1];
				OutVertexVector->at(VertIndex).Tan[2] = Tangents.mData[2];
			}
		}

		if (TanEle->GetMappingMode() == FbxGeometryElement::eByControlPoint)
		{
			for (int VertIndex = 0; VertIndex < PMesh->GetControlPointsCount(); VertIndex++)
			{
				int TangentIndex = 0;

				//If reference mode is direct that means that the Tangent index and the vertex index are the same
				if (TanEle->GetReferenceMode() == FbxGeometryElement::eDirect)
				{
					TangentIndex = VertIndex;
				}

				//If reference mode is Index to Direct, that means that the Tangents are collected by Index to Direct
				if (TanEle->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
				{
					TangentIndex = TanEle->GetIndexArray().GetAt(TangentIndex);
				}

				FbxVector4 Tangents = TanEle->GetDirectArray().GetAt(TangentIndex);

				OutVertexVector->at(VertIndex).Tan[0] = Tangents.mData[0];
				OutVertexVector->at(VertIndex).Tan[1] = Tangents.mData[1];
				OutVertexVector->at(VertIndex).Tan[2] = Tangents.mData[2];
			}
		}
	}
}

void FBX::ImportMaterial(FbxMesh* PMesh, MaterialHeader* OutMaterial)
{
	int MatCounter = 0;

	if (PMesh)
	{
		MatCounter = PMesh->GetNode()->GetMaterialCount();
		write.MainH.MaterialCount = MatCounter;
	}

	if (MatCounter > 0)
	{
		FbxPropertyT<FbxDouble3> CAmbient;
		FbxPropertyT<FbxDouble3> CDiffuse;
		FbxPropertyT<FbxDouble3> CSpecular;
		FbxPropertyT<FbxDouble> CTransparency;
		FbxPropertyT<FbxDouble> CShininess;
		FbxPropertyT<FbxDouble> CReflection;

		for (int MatIndex = 0; MatIndex < MatCounter; MatIndex++)
		{
			FbxSurfaceMaterial* Mat = PMesh->GetNode()->GetMaterial(MatIndex);
		
			FbxString MatName = Mat->GetName();

			//If material is phong
			//If there is an error ex. "UNRESOLVED EXTERNAL" that names ClassId as wrong, change "libfbxsdk.lib" to "libfbxsdk-md.lib"
			if (Mat->GetClassId().Is(FbxSurfacePhong::ClassId))
			{
				CAmbient = ((FbxSurfacePhong*)Mat)->Ambient;
				CDiffuse = ((FbxSurfacePhong*)Mat)->Diffuse;
				CSpecular = ((FbxSurfacePhong*)Mat)->Specular;
				CTransparency = ((FbxSurfacePhong*)Mat)->TransparencyFactor;
				CShininess = ((FbxSurfacePhong*)Mat)->Shininess;
				CReflection = ((FbxSurfacePhong*)Mat)->ReflectionFactor;

				//Puts Ambient values into the struct
				OutMaterial->Ambient[0] = CAmbient.Get()[0];
				OutMaterial->Ambient[1] = CAmbient.Get()[1];
				OutMaterial->Ambient[2] = CAmbient.Get()[2];

				//Puts Diffuse values into the struct
				OutMaterial->Diffuse[0] = CDiffuse.Get()[0];
				OutMaterial->Diffuse[1] = CDiffuse.Get()[1];
				OutMaterial->Diffuse[2] = CDiffuse.Get()[2];

				//Puts Specular values into the struct
				OutMaterial->Specular[0] = CSpecular.Get()[0];
				OutMaterial->Specular[1] = CSpecular.Get()[1];
				OutMaterial->Specular[2] = CSpecular.Get()[2];

				//Multiple Properties that the material needs
				OutMaterial->Transparency = CTransparency.Get();
				OutMaterial->Shininess = CShininess.Get();
				OutMaterial->Reflection = CReflection.Get();
			}

			//If Material is Lambert
			//If there is an error ex. "UNRESOLVED EXTERNAL" that names ClassId as wrong, change "libsdkfbx.lib" to "libsdkfbx-md.lib"
			else if (Mat->GetClassId().Is(FbxSurfaceLambert::ClassId))
			{
				CAmbient = ((FbxSurfaceLambert*)Mat)->Ambient;
				CDiffuse = ((FbxSurfaceLambert*)Mat)->Diffuse;
				CTransparency = ((FbxSurfaceLambert*)Mat)->TransparencyFactor;

				OutMaterial->Ambient[0] = CAmbient.Get()[0];
				OutMaterial->Ambient[1] = CAmbient.Get()[1];
				OutMaterial->Ambient[2] = CAmbient.Get()[2];

				OutMaterial->Diffuse[0] = CDiffuse.Get()[0];
				OutMaterial->Diffuse[1] = CDiffuse.Get()[1];
				OutMaterial->Diffuse[2] = CDiffuse.Get()[2];


				for (int i = 0; i == 3; i++)
					OutMaterial->Specular[i] = 0;

				OutMaterial->Transparency = CTransparency.Get();
			}
			else
			{
				FBXSDK_printf("Error loading material");
			}
		}
	}
}

DirectX::XMMATRIX FBX::convertFbxMatrixToXMMatrix(FbxAMatrix input)
{
	float matrixV[16];

	unsigned int localCounter = 0;

	for (unsigned int g = 0; g < 4; g++)
	{
		for (unsigned int h = 0; h < 4; h++)
		{
			matrixV[localCounter] = input.Get(g, h);
			localCounter++;
		}
	}

	DirectX::XMMATRIX output = DirectX::XMMatrixSet(matrixV[0], matrixV[1], matrixV[2], matrixV[3],
		matrixV[4], matrixV[5], matrixV[6], matrixV[7],
		matrixV[8], matrixV[9], matrixV[10], matrixV[11],
		matrixV[12], matrixV[13], matrixV[14], matrixV[15]);
		
	return output;
}

unsigned int FBX::findJointIndexByName(const char * jointName)
{
	try
	{
		for (unsigned int i = 0; i < joints.size(); i++)
		{

			int compareValue = std::strcmp(jointName, joints[i].jointName);
			if (compareValue == 0) { //no matching name can be found
				return joints[i].jointIndex; //parentIndex + 1 gets the index of this joint.
			}
		}
	}
	
	catch(const std::exception&)
	{
		printf("Error, cannot find matching joint name\n");
	}

}

void FBX::convertFromFloat4X4ToFloat16(DirectX::XMFLOAT4X4 matrix, int &currentJointIndex)
{
	//gather information, bindposematrix in skeleton struct -> Jointheader
	joints[currentJointIndex].globalBindPoseInverse[0] = matrix._11;
	joints[currentJointIndex].globalBindPoseInverse[1] = matrix._12;
	joints[currentJointIndex].globalBindPoseInverse[2] = matrix._13;
	joints[currentJointIndex].globalBindPoseInverse[3] = matrix._14;

	joints[currentJointIndex].globalBindPoseInverse[4] = matrix._21;
	joints[currentJointIndex].globalBindPoseInverse[5] = matrix._22;
	joints[currentJointIndex].globalBindPoseInverse[6] = matrix._23;
	joints[currentJointIndex].globalBindPoseInverse[7] = matrix._24;

	joints[currentJointIndex].globalBindPoseInverse[8] = matrix._31;
	joints[currentJointIndex].globalBindPoseInverse[9] = matrix._32;
	joints[currentJointIndex].globalBindPoseInverse[10] = matrix._33;
	joints[currentJointIndex].globalBindPoseInverse[11] = matrix._34;

	joints[currentJointIndex].globalBindPoseInverse[12] = matrix._41;
	joints[currentJointIndex].globalBindPoseInverse[13] = matrix._42;
	joints[currentJointIndex].globalBindPoseInverse[14] = matrix._43;
	joints[currentJointIndex].globalBindPoseInverse[15] = matrix._44;

}

void FBX::ImportSkeleton(FbxMesh * PMesh, FbxScene* PScene, Skeleton* OutSkeleton, vector<JointHeader>* OutJointVector, vector <Skinning> &OutSkinPerCtrlPoint, map<int, vector<Keyframe>> &OutKeyframes)
{	
	
	int currentJointIndex;

	//Deformers are ways of deforming your mesh
	//A skeleton has deformers of joint-type
	unsigned int deformerCount = PMesh->GetDeformerCount(FbxDeformer::eSkin);

	if (deformerCount <= 0)
	{	
		return; //if the mesh has no skin deformer, then don´t proceed
	}

	//get informations per skin deformer
	for (unsigned int defIndex = 0; defIndex < deformerCount; defIndex++)
	{

		FbxSkin* currentSkin = reinterpret_cast<FbxSkin*>(PMesh->GetDeformer(defIndex, FbxDeformer::eSkin)); //for each skin of that deformer

		if (!currentSkin)
			continue;

		//how many clusters per deformer
		//The cluster in that deformer, links it to the actual joint  
		unsigned int numberOfClusters = currentSkin->GetClusterCount();
		for (unsigned int clusterIndex = 0; clusterIndex < numberOfClusters; clusterIndex++)
		{
			FbxCluster* currentCluster = currentSkin->GetCluster(clusterIndex);
			FbxNode* currentJoint = currentCluster->GetLink();
			const char* currentJointName = currentJoint->GetName();

			currentJointIndex = findJointIndexByName(currentJointName);

			FbxAMatrix transformMatrix;
			FbxAMatrix transformLinkMatrix;
			FbxAMatrix GlobalBPoseInverseMatrix;

			currentCluster->GetTransformMatrix(transformMatrix); //transform of mesh at the time in bind pose
			currentCluster->GetTransformLinkMatrix(transformLinkMatrix); //matrix local to the joint
			//transformation of the cluster in bind pose time, from local- to world space
			GlobalBPoseInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix;

			DirectX::XMMATRIX bindPoseMatrix;

			//convert fxbmatrix to XMMatrix
			bindPoseMatrix = convertFbxMatrixToXMMatrix(GlobalBPoseInverseMatrix);

			DirectX::XMFLOAT4X4 finalBindPose;

			//convert from XMMatrix to XMFloat 4X4
			DirectX::XMStoreFloat4x4(&finalBindPose, bindPoseMatrix);

			convertFromFloat4X4ToFloat16(finalBindPose, currentJointIndex);

			//number of indices per control point for the current cluster
			unsigned int numOfIndices = currentCluster->GetControlPointIndicesCount();

			//get the weights and (in value) index affecting each joint
			for (unsigned int i = 0; i < numOfIndices; i++)
			{

				double blendingWeight = currentCluster->GetControlPointWeights()[i];
				unsigned int ctrlPointIndex = currentCluster->GetControlPointIndices()[i];
				/*Associate each joint with control points it affects*/
				auto it = blendInfoPerVertexIndex.find(ctrlPointIndex);
			
				if (it == blendInfoPerVertexIndex.end())
				{
					//connect wich index the joint affects with the amount of weight for that index
					it = blendInfoPerVertexIndex.insert(std::make_pair(ctrlPointIndex, BlendInfo())).first;
				}
			
				it->second.boneIndices.push_back(currentJointIndex);
				it->second.boneWeights.push_back(blendingWeight);
			}

			currLayer->Weight = 100;

			FbxAnimCurve* translationCurve_X = currentJoint->LclTranslation.GetCurve(currLayer, FBXSDK_CURVENODE_COMPONENT_X);

			//The animation evaluator is used to compute node transforms and property values at specific times during an animation.
			FbxAnimEvaluator* animEvaluator = currentJoint->GetAnimationEvaluator();

			for (long keyIndex = 0; keyIndex < translationCurve_X->KeyGetCount(); ++keyIndex)
			{
				float asRadians = DirectX::XM_PI / 180;
				FbxAnimCurveKey currKey = translationCurve_X->KeyGet(keyIndex);

				//get the transformation values, FbxVector4 = double * 4
				FbxVector4 translation = animEvaluator->GetNodeLocalTranslation(currentJoint, currKey.GetTime());
				FbxVector4 rotation = animEvaluator->GetNodeLocalRotation(currentJoint, currKey.GetTime());
				FbxVector4 scale = animEvaluator->GetNodeLocalScaling(currentJoint, currKey.GetTime());

				//converts right-handed coordinate system to left-handed, directX
				translation[2] *= -1.0;
				rotation[0] *= -1.0;
				rotation[1] *= -1.0;

				DirectX::XMVECTOR translationValues;
				DirectX::XMVECTOR rotationValues;
				DirectX::XMVECTOR scalingValues;

				translationValues = DirectX::XMVectorSet(translation[0], translation[1], translation[2], translation[3]);
				rotationValues = DirectX::XMVectorSet(rotation[0] * asRadians, rotation[1] * asRadians, rotation[2] * asRadians, rotation[3]);
				scalingValues = DirectX::XMVectorSet(scale[0], scale[1], scale[2], scale[3]);

				//store the right value in XMFLOAT4
				DirectX::XMFLOAT4 storeTranslate;
				DirectX::XMFLOAT4 storeRotate;
				DirectX::XMFLOAT4 storeScale;
				DirectX::XMStoreFloat4(&storeTranslate, translationValues);
				DirectX::XMStoreFloat4(&storeRotate, rotationValues);
				DirectX::XMStoreFloat4(&storeScale, scalingValues);

				//get the time values for the current keyframe 
				FbxTime keyTimeTemp = currKey.GetTime();
				float keyTime = keyTimeTemp.GetSecondDouble();

				Keyframe keyframe;
				keyframe.keyTime = keyTime;

				//convert XMFLOAT4 to float * 4, to be able to export it
				keyframe.translation[0] = storeTranslate.x;
				keyframe.translation[1] = storeTranslate.y;
				keyframe.translation[2] = storeTranslate.z;
				keyframe.translation[3] = storeTranslate.w;

				keyframe.rotation[0] = storeRotate.x;
				keyframe.rotation[1] = storeRotate.y;
				keyframe.rotation[2] = storeRotate.z;
				keyframe.rotation[3] = storeRotate.w;

				keyframe.scale[0] = storeScale.x;
				keyframe.scale[1] = storeScale.y;
				keyframe.scale[2] = storeScale.z;
				keyframe.scale[3] = storeScale.w;

				//store the keyframes per joint in our vector
				OutKeyframes[currentJointIndex].push_back(keyframe);

			}
		}

		for (int i = 0; i < blendInfoPerVertexIndex.size(); i++)
		{
			while (blendInfoPerVertexIndex[i].boneWeights.size() < 4 && blendInfoPerVertexIndex[i].boneIndices.size() < 4)
			{
				blendInfoPerVertexIndex[i].boneWeights.push_back(0);
				blendInfoPerVertexIndex[i].boneIndices.push_back(0);
			}
		}

		const int polyCount = PMesh->GetPolygonCount();
		for (int polyIndex = 0; polyIndex < polyCount; ++polyIndex)
		{
			const int polySize = PMesh->GetPolygonSize(polyIndex);
			for (int vertexIndex = 0; vertexIndex < polySize; ++vertexIndex)
			{
				int indexPerCntrl = PMesh->GetPolygonVertex(polyIndex, vertexIndex);

				BlendInfo BI = blendInfoPerVertexIndex[indexPerCntrl];
				assert(BI.boneIndices.size() >= 1);

				while (BI.boneIndices.size() > 4)
				{
#undef min
					unsigned int small = std::min(BI.boneWeights.begin(), BI.boneWeights.end()) - BI.boneWeights.begin();
					BI.boneIndices.erase(BI.boneIndices.begin() + small);
					BI.boneWeights.erase(BI.boneWeights.begin() + small);

				}

				float sumOfWeight = 0.0;
				//We want Weight to end up as 1
				for (unsigned int index = 0; index < BI.boneWeights.size(); index++)
				{
					sumOfWeight += BI.boneWeights[index];
				}

				for (unsigned int index = 0; index < BI.boneWeights.size(); index++)
				{
					BI.boneWeights[index] /= sumOfWeight;
				}

				Skinning s;
				float sizeWeights[4];

				memset(&s.weights, 0, sizeof(float) * 4);

				for (unsigned int weightIndex = 0; weightIndex < BI.boneIndices.size() && weightIndex < 4; weightIndex++)
				{
					float* pointWeight = (float*)&s.weights;
					pointWeight[weightIndex] = BI.boneWeights[weightIndex];

					unsigned int* pointI = (unsigned int*)&s.boneIndices;
					pointI[weightIndex] = BI.boneIndices[weightIndex];
				}

				OutSkinPerCtrlPoint.push_back(s);
			}
		}

		for (unsigned int l = 0; l < joints.size(); l++)
		{
			OutJointVector->push_back(joints[l]);
			OutJointVector->at(l).nrOfKeyframes = OutKeyframes[l].size();
		}

		OutSkeleton->numberOfSkinWeights = OutSkinPerCtrlPoint.size();
		write.MainH.SkeletonAnimationCount++;
	}

}

void FBX::ImportTexture(FbxMesh* PMesh, MaterialHeader* OutMaterial)
{
	FbxProperty Prop;

	if (PMesh->GetNode() == NULL)
		return;

	int Mat = PMesh->GetNode()->GetSrcObjectCount<FbxSurfaceMaterial>();

	for (int matIndex = 0; matIndex < Mat; matIndex++)
	{
		FbxSurfaceMaterial *pMat = PMesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(matIndex);

		if (pMat)
		{
			Prop = pMat->FindProperty(FbxSurfaceMaterial::sDiffuse);
			if (Prop.IsValid())
			{
				int texCount = Prop.GetSrcObjectCount<FbxTexture>();
				
				for (int i = 0; i < texCount; i++)
				{
					FbxLayeredTexture *layerTex = Prop.GetSrcObject<FbxLayeredTexture>(i);

					if (layerTex)
					{
						FbxLayeredTexture * LayerTexture = Prop.GetSrcObject <FbxLayeredTexture>(i);
						int TexturesIndex = LayerTexture->GetSrcObjectCount<FbxTexture>();

						for (int t = 0; t < TexturesIndex; t++)
						{
							FbxTexture* Texture = LayerTexture->GetSrcObject<FbxTexture>(t);

							if (Texture)
							{
								FbxLayeredTexture::EBlendMode BlendM;
								LayerTexture->GetTextureBlendMode(t, BlendM);
							}
						}
					}

					FbxTexture* Tex = Prop.GetSrcObject<FbxTexture>(i);
					FbxFileTexture * FileTex = FbxCast<FbxFileTexture>(Tex);
					FbxString TexFileName = FileTex->GetFileName();

					wchar_t* out;
					FbxUTF8ToWC(TexFileName.Buffer(), out, NULL);

					for (int i = 0; i < 256; i++)
					{
						OutMaterial->diffuseMap[i] = out[i];
					}

					FbxFree(out);
				}
			}
		}
	}
}

void FBX::ImportCamera(FbxNode* PNode, vector<CameraHeader>* OutCameraVector)
{
	for (int i = 0; i < PNode->GetChildCount(); i++)
		ImportCamera(PNode->GetChild(i), OutCameraVector);

	FbxCamera* pCamera = PNode->GetCamera();

	CameraHeader Cam;

	if (pCamera != NULL)
	{
		FbxVector4 tmpCampos = pCamera->Position.Get();

		Cam.CamPos[0] = (float)tmpCampos[0];
		Cam.CamPos[1] = (float)tmpCampos[1];
		Cam.CamPos[2] = (float)tmpCampos[2];

		FbxDouble3 tmpRotation = PNode->LclRotation;

		Cam.CamRotation[0] = (float)tmpRotation[0];
		Cam.CamRotation[1] = (float)tmpRotation[1];
		Cam.CamRotation[2] = (float)tmpRotation[2];

		OutCameraVector->push_back(Cam);
		write.MainH.CameraCount++;
	}
}

void FBX::ImportLight(FbxNode* PNode, Light* OutLight, vector<PointLight>* OutPointVector, vector<SpotLight>* OutSpotVector, vector<DirectionalLight>* OutDirectionalVector)
{
	for (int i = 0; i < PNode->GetChildCount(); i++)
		ImportLight(PNode->GetChild(i), OutLight, OutPointVector, OutSpotVector, OutDirectionalVector);

	FbxLight* myLight = PNode->GetLight();

	// If Light Node Exist
	if (myLight != NULL)
	{
		// Point Light
		if (myLight->LightType.Get() == FbxLight::ePoint)
		{
			PointLight pLight;

			FbxAMatrix pLightPos = myLight->GetNode()->EvaluateGlobalTransform();
			FbxDouble3 pos = pLightPos.GetT();

			for (int j = 0; j < 3; j++)
			{
				pLight.Position[j] = pos[j];
			}

			FbxDouble3 tmpPLight = myLight->Color.Get();

			for (int i = 0; i < 3; i++)
			{
				pLight.Color[i] = (float)tmpPLight[i];
			}

			float Intensity = myLight->Intensity.Get();
			pLight.Intensity = Intensity;

			OutPointVector->push_back(pLight);
			OutLight->PointLightCount++;
		}

		// Spot Light
		if (myLight->LightType.Get() == FbxLight::eSpot)
		{
			SpotLight sLight;

			FbxAMatrix sLightPos = myLight->GetNode()->EvaluateGlobalTransform();
			FbxDouble3 pos = sLightPos.GetT();
			FbxDouble3 sLightRot = myLight->GetNode()->EvaluateLocalRotation();

			for (int j = 0; j < 3; j++)
			{
				sLight.Position[j] = pos[j];
				sLight.Rotation[j] = sLightRot[j];
			}

			FbxDouble3 tmpSLight = myLight->Color.Get();

			for (int j = 0; j < 3; j++)
			{
				sLight.Color[j] = (float)tmpSLight[j];
			}

			float Intensity = myLight->Intensity.Get();
			sLight.Intensity = Intensity;

			OutSpotVector->push_back(sLight);
			OutLight->SpotLightCount++;
		}
		

		// Directional Light
		if (myLight->LightType.Get() == FbxLight::eDirectional)
		{
			DirectionalLight dLight;

			FbxAMatrix dLightPos = myLight->GetNode()->EvaluateGlobalTransform();
			FbxDouble3 pos = dLightPos.GetT();
			FbxDouble3 dLightRot = myLight->GetNode()->EvaluateLocalRotation();

			for (int j = 0; j < 3; j++)
			{
				dLight.Position[j] = pos[j];
				dLight.Rotation[j] = dLightRot[j];
			}

			FbxDouble3 tmpDLight = myLight->Color.Get();

			for (int l = 0; l < 3; l++)
			{
				dLight.Color[l] = (float)tmpDLight[l];
			}

			float Intensity = myLight->Intensity.Get();
			dLight.Intensity = Intensity;

			OutDirectionalVector->push_back(dLight);
			OutLight->DirectionalLightCount++;
		}
	}
	write.MainH.LightCount = OutLight->PointLightCount + OutLight->SpotLightCount + OutLight->DirectionalLightCount;
}

void FBX::ImportMorphAnimation(FbxMesh* PMesh, vector<MorphHeader>* OutMorph, MorphCountHeader* OutMorphCount)
{
	int BlndShapeCount = PMesh->GetDeformerCount(FbxDeformer::eBlendShape);
	write.MainH.MorphCount = BlndShapeCount;

	if (BlndShapeCount > 0)
	{
		MorphHeader morphH;

		for (int bsIndex = 0; bsIndex < BlndShapeCount; bsIndex++)
		{
			FbxBlendShape* PBlendShape = (FbxBlendShape*)PMesh->GetDeformer(bsIndex, FbxDeformer::eBlendShape);

			int BlendShapeChannelCount = PBlendShape->GetBlendShapeChannelCount();

			for (int ChannelIndex = 0; ChannelIndex < BlendShapeChannelCount; ChannelIndex++)
			{
				FbxBlendShapeChannel* Channel = PBlendShape->GetBlendShapeChannel(ChannelIndex);

				int ShapeCount = Channel->GetTargetShapeCount();

				for (int ShapeIndex = 0; ShapeIndex < ShapeCount; ShapeIndex++)
				{
					FbxShape * PShape = Channel->GetTargetShape(ShapeIndex);
					BlndVert.push_back(PShape->GetControlPoints());
					FbxVector4 tmpBlendVert = *BlndVert[ShapeIndex];
					morphH.MorphShape[0] = (float)tmpBlendVert[0];
					morphH.MorphShape[1] = (float)tmpBlendVert[1];
					morphH.MorphShape[2] = (float)tmpBlendVert[2];
					morphH.MorphShape[3] = (float)tmpBlendVert[3];
					OutMorph->push_back(morphH);
					OutMorphCount->MorphShapeCount++;
				}
			}
		}
	}
}

void FBX::Initialization()
{
	SDKInitialization(PManager, PScene); //intialize all SDK objects for FBX import
	FbxMesh* PMesh = LoadScene(PManager, PScene); //Imports scene and returns mesh from FBX file
	ImportVertices(PMesh, &write.OutPutMesh, &write.OutVertexVector); //Imports all the vertexes
	ImportNormals(PMesh, &write.OutVertexVector);
	ImportUV(PMesh, &write.OutVertexVector);
	ImportMaterial(PMesh, &write.OutMaterial);
	ImportTexture(PMesh, &write.OutMaterial);
	ImportCamera(ParentNode, &write.OutCameraVector);
	ImportLight(ParentNode, &write.OutLight, &write.OutPointVector, &write.OutSpotVector, &write.OutDirectionalVector);
	ImportMorphAnimation(PMesh, &write.OutMorphVector, &write.OutMorphCount);
	ImportSkeleton(PMesh, PScene, &write.OutSkeleton, &write.OutJointVector, write.OutSkinPerCtrlPoint, write.OutKeyframes);
	write.writeBinary();
}