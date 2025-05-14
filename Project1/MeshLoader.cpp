#include "MeshLoader.h"

MeshLoader::MeshLoader()
{
}

MeshLoader::~MeshLoader()
{
}

bool MeshLoader::LoadMesh(const std::string filePath)
{
	GameManager::GetInstance().PauseTime();

	std::vector<MeshData> meshDatas;

	int flag = 0;

	flag |= aiProcess_Triangulate;

	flag |= aiProcess_GenSmoothNormals;

	flag |= aiProcess_ConvertToLeftHanded;

	scene = *import.ReadFile(filePath, flag);

	if (scene.HasMeshes())
	{
		meshDatas.resize(scene.mNumMeshes);

		StoreVertices(filePath);

		bool isSuccess = true;

		if (!isSuccess)
		{
			std::cerr << "“Ç‚Ýž‚Ý‚Ì‰Šú‰»‚ÌŽ¸”s" << std::endl;

			return false;
		}

		GameManager::GetInstance().UnpauseTime();

		return true;
	}
	else
	{
		std::cout << "ƒƒbƒVƒ…‚ðŽ‚Á‚Ä‚¢‚È‚¢" << std::endl;

		GameManager::GetInstance().UnpauseTime();

		return false;
	}
}

void MeshLoader::StoreVertices(const std::string filePath, std::vector<MeshData> meshDatas)
{
	aiVector3D zero = aiVector3D(0, 0, 0);

	UINT baseVertexNum = 0;
	UINT baseIndexNum = 0;

	for (int i = 0; i < scene.mNumMeshes; i++)
	{
		aiMesh* mesh = scene.mMeshes[i];

		meshDatas[i].VertexDatas.resize(mesh->mNumVertices);
		meshDatas[i].vertexBoneDatas.resize(mesh->mNumVertices);

		for (int j = 0; j < mesh->mNumVertices; j++)
		{
			VertexData* data = &meshDatas[i].VertexDatas[j];

			auto pos = mesh->mVertices[j];

			auto nor = mesh->mNormals[j];

			auto tan = mesh->HasTangentsAndBitangents() ? mesh->mTangents[j] : zero;

			auto coord = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][j] : zero;

			data->vertex = XMFLOAT3(pos.x, pos.y, pos.z);
			data->normal = XMFLOAT3(nor.x, nor.y, nor.z);
			data->tangent = XMFLOAT3(tan.x, tan.y, tan.z);
			data->texCoord = XMFLOAT2(coord.x, coord.y);
		}

		meshDatas[i].indices.resize(mesh->mNumFaces * 3);

		for (int j = 0; j < mesh->mNumFaces; j++)
		{
			auto face = mesh->mFaces[j];

			meshDatas[i].indices[j * 3 + 0] = face.mIndices[0];
			meshDatas[i].indices[j * 3 + 1] = face.mIndices[1];
			meshDatas[i].indices[j * 3 + 2] = face.mIndices[2];
		}

		meshDatas[i].baseVertexNumber = baseVertexNum;
		meshDatas[i].baseIndexNumber = baseIndexNum;

		baseVertexNum += mesh->mNumVertices;
		baseIndexNum += meshDatas[i].indices.size();

		if (scene.HasMaterials())
			InitMaterial(filePath, *mesh, meshDatas[i], i);

		if (mesh->HasBones())
			InitBone(*mesh, meshDatas[i]);
	}
}

void MeshLoader::InitBone(const aiMesh& mesh, MeshData& meshData)
{
	for (UINT j = 0; j < mesh.mNumBones; j++)
	{

		auto bone = mesh.mBones[j];

		UINT boneIndex = 0;

		if (boneMapping.find(bone->mName.C_Str()) == boneMapping.end())
		{
			boneIndex = boneMapping.size();

			BoneInfo bi;

			meshData.boneInfos.emplace_back(bi);


			boneMapping[bone->mName.C_Str()] = boneIndex;

			std::cout << bone->mName.C_Str() << std::endl;
			std::cout << boneMapping[bone->mName.C_Str()] << std::endl;
		}
		else
		{
			boneIndex = boneMapping[bone->mName.C_Str()];
		}

		meshData.boneInfos[boneIndex].offset = bone->mOffsetMatrix;

		for (UINT k = 0; k < bone->mNumWeights; k++)
		{
			auto weight = bone->mWeights[k];

			meshData.vertexBoneDatas[weight.mVertexId].boneID.emplace_back(boneIndex);
			meshData.vertexBoneDatas[weight.mVertexId].weight.emplace_back(weight.mWeight);
		}
	}

	for (int j = 0; j < mesh.mNumVertices; j++)
	{
		VertexData& vertexData = meshData.VertexDatas[j];

		auto& boneiD = meshData.vertexBoneDatas[j].boneID;
		auto& weight = meshData.vertexBoneDatas[j].weight;

		if (boneiD.size() == 0)
		{
			vertexData.boneID = XMFLOAT4(0, 0, 0, 0);
			vertexData.weight = XMFLOAT4(0, 0, 0, 0);
		}
		else if (boneiD.size() == 1)
		{
			vertexData.boneID = XMFLOAT4(boneiD[0], 0, 0, 0);
			vertexData.weight = XMFLOAT4(weight[0], 0, 0, 0);
		}
		else if (boneiD.size() == 2)
		{
			vertexData.boneID = XMFLOAT4(boneiD[0], boneiD[1], 0, 0);
			vertexData.weight = XMFLOAT4(weight[0], weight[1], 0, 0);
		}
		else if (boneiD.size() == 3)
		{
			vertexData.boneID = XMFLOAT4(boneiD[0], boneiD[1], boneiD[2], 0);
			vertexData.weight = XMFLOAT4(weight[0], weight[1], weight[2], 0);
		}
		else
		{
			vertexData.boneID = XMFLOAT4(boneiD[0], boneiD[1], boneiD[2], boneiD[3]);
			vertexData.weight = XMFLOAT4(weight[0], weight[1], weight[2], weight[3]);
		}


	}
}

void MeshLoader::InitMaterial(const std::string filePath, const aiMesh& mesh, MeshData& data, const int meshIndex)
{
	auto index = filePath.find_last_of("/");

	std::string dir = filePath.substr(0, index);

	aiString path;

	aiMaterial* mat = scene.mMaterials[mesh.mMaterialIndex];

	if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
	{
		mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);

		std::string str = path.data;

		std::replace(str.begin(), str.end(), '\\', '/');

		auto indexPeriod = str.find_last_of(".");

		str = str.substr(0, indexPeriod) + ".dds";

		LoadTexture(dir + "/" + str, data);
	}
	else
	{
		LoadTexture("Textures/error.dds", data);
	}
}

