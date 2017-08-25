#include "Write.h"

void Write::writeBinary()
{
	ofstream binaryFile("BinaryDataShip.dat", ios::out | ios::binary);

	binaryFile.write((const char*)&MainH.MeshCount, sizeof(unsigned int));
	binaryFile.write((const char*)&MainH.MaterialCount, sizeof(unsigned int));
	binaryFile.write((const char*)&MainH.LightCount, sizeof(unsigned int));
	binaryFile.write((const char*)&MainH.CameraCount, sizeof(unsigned int));
	binaryFile.write((const char*)&MainH.SkeletonAnimationCount, sizeof(unsigned int));
	binaryFile.write((const char*)&MainH.MorphCount, sizeof(unsigned int));

	for (int i = 0; i < MainH.MeshCount; i++)
	{
		binaryFile.write((const char*)&OutPutMesh.at(i).VertexCount, sizeof(unsigned int));
		binaryFile.write((const char*)&OutPutMesh.at(i).MaterialID, sizeof(unsigned int));
		binaryFile.write((const char*)&OutPutMesh.at(i).ObjectID, sizeof(unsigned int));
		binaryFile.write((const char*)&OutPutMesh.at(i).ParentID, sizeof(unsigned int));
		binaryFile.write((const char*)&OutPutMesh.at(i).MeshName, sizeof(char) * 256);
		binaryFile.write((const char*)&OutPutMesh.at(i).AttributeCount, sizeof(unsigned int));
		binaryFile.write((const char*)&OutPutMesh.at(i).Translation, sizeof(float) * 3);
		binaryFile.write((const char*)&OutPutMesh.at(i).Rotation, sizeof(float) * 3);
		binaryFile.write((const char*)&OutPutMesh.at(i).Scale, sizeof(float) * 3);

		for (int j = 0; j < OutPutMesh.at(i).VertexCount; j++)
		{
			binaryFile.write((const char*)&OutVertexVector.at(j).Pos, sizeof(float) * 3);
			binaryFile.write((const char*)&OutVertexVector.at(j).Nor, sizeof(float) * 3);
			binaryFile.write((const char*)&OutVertexVector.at(j).UV, sizeof(float) * 2);
			binaryFile.write((const char*)&OutVertexVector.at(j).Tan, sizeof(float) * 3);
			binaryFile.write((const char*)&OutVertexVector.at(j).BiTan, sizeof(float) * 3);
		}
	}

	for (int i = 0; i < MainH.MaterialCount; i++)
	{
		binaryFile.write((const char*)&OutMaterial.Diffuse, sizeof(float) * 3);
		binaryFile.write((const char*)&OutMaterial.Specular, sizeof(float) * 3);
		binaryFile.write((const char*)&OutMaterial.Ambient, sizeof(float) * 3);

		binaryFile.write((const char*)&OutMaterial.Transparency, sizeof(float));
		binaryFile.write((const char*)&OutMaterial.Shininess, sizeof(float));
		binaryFile.write((const char*)&OutMaterial.Reflection, sizeof(float));

		binaryFile.write((char*)&OutMaterial.diffuseMap, sizeof(char) * 256);
	}

	for (int i = 0; i < MainH.LightCount; i++)
	{
		binaryFile.write((const char*)&OutLight.PointLightCount, sizeof(unsigned int));
		binaryFile.write((const char*)&OutLight.SpotLightCount, sizeof(unsigned int));
		binaryFile.write((const char*)&OutLight.DirectionalLightCount, sizeof(unsigned int));

		for (int j = 0; j < OutLight.PointLightCount; j++)
		{
			binaryFile.write((const char*)&OutPointVector.at(j).Intensity, sizeof(float));
			binaryFile.write((const char*)&OutPointVector.at(j).Color, sizeof(float) * 3);
			binaryFile.write((const char*)&OutPointVector.at(j).Position, sizeof(float) * 3);
		}

		for (int k = 0; k < OutLight.SpotLightCount; k++)
		{
			binaryFile.write((const char*)&OutSpotVector.at(k).Intensity, sizeof(float));
			binaryFile.write((const char*)&OutSpotVector.at(k).Color, sizeof(float) * 3);
			binaryFile.write((const char*)&OutSpotVector.at(k).Position, sizeof(float) * 3);
			binaryFile.write((const char*)&OutSpotVector.at(k).Rotation, sizeof(float) * 3);
		}

		for (int l = 0; l < OutLight.DirectionalLightCount; l++)
		{
			binaryFile.write((const char*)&OutDirectionalVector.at(l).Intensity, sizeof(float));
			binaryFile.write((const char*)&OutDirectionalVector.at(l).Color, sizeof(float) * 3);
			binaryFile.write((const char*)&OutDirectionalVector.at(l).Position, sizeof(float) * 3);
			binaryFile.write((const char*)&OutDirectionalVector.at(l).Rotation, sizeof(float) * 3);
		}
	}

	for (int i = 0; i < MainH.CameraCount; i++)
	{
		binaryFile.write((const char*)&OutCameraVector.at(i).CamPos, sizeof(float) * 3);
		binaryFile.write((const char*)&OutCameraVector.at(i).CamRotation, sizeof(float) * 3);
	}

	for (int i = 0; i < MainH.SkeletonAnimationCount; i++)
	{
		binaryFile.write((const char*)&OutSkeleton.numberOfJoints, sizeof(unsigned int));
		binaryFile.write((const char*)&OutSkeleton.numberOfSkinWeights, sizeof(unsigned int));

		for (int j = 0; j < OutSkeleton.numberOfJoints; j++)
		{

			binaryFile.write((const char*)&OutJointVector.at(j).jointName, sizeof(char) * 256);
			binaryFile.write((const char*)&OutJointVector.at(j).ParentID, sizeof(unsigned int));
			binaryFile.write((const char*)&OutJointVector.at(j).jointIndex, sizeof(unsigned int));
			binaryFile.write((const char*)&OutJointVector.at(j).nrOfKeyframes, sizeof(unsigned int));
			binaryFile.write((const char*)&OutJointVector.at(j).globalBindPoseInverse, sizeof(float) * 16);
		
			for (int k = 0; k < OutJointVector.at(j).nrOfKeyframes; k++)
			{
				binaryFile.write((const char*)&OutKeyframes[j].at(k).translation, sizeof(float) * 4);
				binaryFile.write((const char*)&OutKeyframes[j].at(k).rotation, sizeof(float) * 4);
				binaryFile.write((const char*)&OutKeyframes[j].at(k).scale, sizeof(float) * 4);			
				binaryFile.write((const char*)&OutKeyframes[j].at(k).keyTime, sizeof(float) * 4);
			}
		}
		for (int j = 0; j < OutSkeleton.numberOfSkinWeights; j++)
		{
			binaryFile.write((const char*)&OutSkinPerCtrlPoint.at(j).boneIndices, sizeof(unsigned int) * 4);
			binaryFile.write((const char*)&OutSkinPerCtrlPoint.at(j).weights, sizeof(float) * 4);
		}

	}

	for (int i = 0; i < MainH.MorphCount; i++)
	{
		binaryFile.write((const char*)&OutMorphCount.MorphShapeCount, sizeof(unsigned int));
		for (int j = 0; j < OutMorphCount.MorphShapeCount; j++)
		{
			binaryFile.write((const char*)&OutMorphVector.at(j).MorphShape, sizeof(float) * 4);
		}
	}

	binaryFile.close();
}

Write::Write() {}

Write::~Write() {}
