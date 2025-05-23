
#include "MeshLoader.h"

MeshLoader::MeshLoader()
{
	Init();
}

MeshLoader::~MeshLoader()
{
}

void MeshLoader::Init()
{
	LoadMesh("Models/SpartanWarrior/Spartan_Warrior.fbx", SPARTAN);
	LoadMesh("Models/Knight01/Animated_Low_Poly_Dark_Knight_BAKED.fbx", KNIGHT);
	LoadMesh("Models/Box.fbx", BOX);
	LoadMesh("Models/Box.fbx", PLANE);
}

std::unique_ptr<MeshLoader> MeshLoader::instance = nullptr;

MeshLoader& MeshLoader::GetInstance()
{
	if (!instance) {
		instance.reset(new MeshLoader());
	}
	return *instance;
}

MeshLoader::MeshData* MeshLoader::GetMesh(MESHTYPE type)
{
	assert(meshData.find(type) != meshData.end());

	return &meshData[type];
}

bool MeshLoader::LoadMesh(const std::string filePath , MESHTYPE type)
{
	GameManager::GetInstance().PauseTime();

	std::vector<Mesh> meshes;

	int flag = 0;

	flag |= aiProcess_Triangulate;

	flag |= aiProcess_GenSmoothNormals;

	flag |= aiProcess_ConvertToLeftHanded;

	meshData[type].scene = *meshData[type].import.ReadFile(filePath, flag);

	currentScene = meshData[type].scene;

	if (currentScene.HasMeshes())
	{
		meshes.resize(currentScene.mNumMeshes);

		StoreVertices(filePath , meshes , meshData[type]);

		bool isSuccess = true;

		if (!isSuccess)
		{
			std::cerr << "読み込みの初期化の失敗" << std::endl;

			return false;
		}

		GameManager::GetInstance().UnpauseTime();

		meshData[type].meshes = meshes;

		RelateMeshWithBone(meshData[type], *meshData[type].scene.mRootNode);

		Init(type);

		return true;
	}
	else
	{
		std::cout << "メッシュを持っていない" << std::endl;

		GameManager::GetInstance().UnpauseTime();

		return false;
	}
}

void MeshLoader::RelateMeshWithBone(MeshData& meshData ,aiNode& node)
{
	string name(node.mName.C_Str());


	for (int i = 0; i < node.mNumMeshes; i++)
	{
		if (node.mParent == meshData.scene.mRootNode)
			continue;

		UINT boneIndex = 0;

		if (meshData.boneMapping.find(name) == meshData.boneMapping.end())
		{
			boneIndex = meshData.boneMapping.size();

			aiMatrix4x4 indentify = aiMatrix4x4();

			meshData.boneOffset.push_back(indentify);

			meshData.boneMapping[name] = boneIndex;

			for (int j = 0; j < meshData.meshes[node.mMeshes[i]].vertexDatas.size(); j++)
			{
				meshData.meshes[node.mMeshes[i]].vertexDatas[j].parentBone = boneIndex;
			}
		}
		else if (meshData.meshes[node.mMeshes[i]].vertexDatas[0].parentBone == 0)
		{
			boneIndex = meshData.boneMapping[name];

			for (int j = 0; j < meshData.meshes[node.mMeshes[i]].vertexDatas.size(); j++)
			{
				meshData.meshes[node.mMeshes[i]].vertexDatas[j].parentBone = boneIndex;
			}
		}
	}



	for (int i = 0;i < node.mNumChildren;i++)
	{
		RelateMeshWithBone(meshData, *node.mChildren[i]);
	}
}

void MeshLoader::Init(MESHTYPE type)
{
	for (int i = 0; i <  meshData[type].meshes.size();i++)
	{
		auto& uploadVertexBuffer = DXRenderSystem::GetInstance().GetEmptyUploadHeap();

		InitVBVResource(meshData[type].meshes[i], uploadVertexBuffer);

		MapVB(meshData[type].meshes[i], uploadVertexBuffer);

		DXRenderSystem::GetInstance().ExecuteCommands();

		VBUploadToDefultHeap(meshData[type].meshes[i], uploadVertexBuffer);

		auto& uploadIndexBuffer = DXRenderSystem::GetInstance().GetEmptyUploadHeap();

		InitIBVResource(meshData[type].meshes[i], uploadIndexBuffer);

		MapIB(meshData[type].meshes[i], uploadIndexBuffer);

		DXRenderSystem::GetInstance().ExecuteCommands();

		IBUploadToDefultHeap(meshData[type].meshes[i], uploadIndexBuffer);
	}

}

void MeshLoader::StoreVertices(const std::string filePath, std::vector<Mesh>& meshes , MeshData& meshData)
{
	aiVector3D zero = aiVector3D(0, 0, 0);

	UINT baseVertexNum = 0;
	UINT baseIndexNum = 0;

	for (int i = 0; i < currentScene.mNumMeshes; i++)
	{
		aiMesh* mesh = currentScene.mMeshes[i];

		meshes[i].vertexDatas.resize(mesh->mNumVertices);
		meshes[i].vertexBoneDatas.resize(mesh->mNumVertices);

		for (int j = 0; j < mesh->mNumVertices; j++)
		{
			VertexData* data = &meshes[i].vertexDatas[j];

			auto pos = mesh->mVertices[j];

			auto nor = mesh->mNormals[j];

			auto tan = mesh->HasTangentsAndBitangents() ? mesh->mTangents[j] : zero;

			auto coord = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][j] : zero;

			data->vertex = XMFLOAT3(pos.x, pos.y, pos.z);
			data->normal = XMFLOAT3(nor.x, nor.y, nor.z);
			data->tangent = XMFLOAT3(tan.x, tan.y, tan.z);
			data->texCoord = XMFLOAT2(coord.x, coord.y);
		}

		meshes[i].indices.resize(mesh->mNumFaces * 3);

		for (int j = 0; j < mesh->mNumFaces; j++)
		{
			auto face = mesh->mFaces[j];

			meshes[i].indices[j * 3 + 0] = face.mIndices[0];
			meshes[i].indices[j * 3 + 1] = face.mIndices[1];
			meshes[i].indices[j * 3 + 2] = face.mIndices[2];
		}

		meshes[i].baseVertexNumber = baseVertexNum;
		meshes[i].baseIndexNumber = baseIndexNum;

		baseVertexNum += mesh->mNumVertices;
		baseIndexNum += meshes[i].indices.size();

		if (currentScene.HasMaterials())
			InitMaterial(filePath, *mesh, meshes[i]);

		if (mesh->HasBones())
			InitBone(*mesh, meshes[i] , meshData);
	}
}

void MeshLoader::InitBone(const aiMesh& aiMesh, Mesh& mesh , MeshData& meshData)
{
	for (UINT j = 0; j < aiMesh.mNumBones; j++)
	{
		auto bone = aiMesh.mBones[j];

		UINT boneIndex = 0;

		if (meshData.boneMapping.find(bone->mName.C_Str()) == meshData.boneMapping.end())
		{
			boneIndex = meshData.boneMapping.size();

			meshData.boneMapping[bone->mName.C_Str()] = boneIndex;

			meshData.boneOffset.emplace_back(bone->mOffsetMatrix);
		}
		else
		{
			boneIndex = meshData.boneMapping[bone->mName.C_Str()];

			meshData.boneOffset[boneIndex] = bone->mOffsetMatrix;
		}



		for (UINT k = 0; k < bone->mNumWeights; k++)
		{
			auto weight = bone->mWeights[k];

			mesh.vertexBoneDatas[weight.mVertexId].boneID.emplace_back(boneIndex);
			mesh.vertexBoneDatas[weight.mVertexId].weight.emplace_back(weight.mWeight);
		}
	}

	for (int j = 0; j < aiMesh.mNumVertices; j++)
	{
		VertexData& vertexData = mesh.vertexDatas[j];

		auto& boneiD = mesh.vertexBoneDatas[j].boneID;
		auto& weight = mesh.vertexBoneDatas[j].weight;

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

void MeshLoader::InitMaterial(const std::string filePath, const aiMesh& mesh, Mesh& data)
{
	auto index = filePath.find_last_of("/");

	std::string dir = filePath.substr(0, index);

	aiString path;

	aiMaterial* mat = currentScene.mMaterials[mesh.mMaterialIndex];

	if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
	{
		mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);

		std::string str = path.data;

		std::replace(str.begin(), str.end(), '\\', '/');

		auto indexPeriod = str.find_last_of(".");

		str = str.substr(0, indexPeriod) + ".dds";

		data.texResource = TextureLoader::GetInstance().LoadTexture(dir + "/" + str);
	}
	else
	{
		data.texResource = TextureLoader::GetInstance().LoadTexture("Textures/error.dds");
	}
}

bool MeshLoader::InitVBVResource(MeshLoader::Mesh& mesh, ComPtr<ID3D12Resource>& uploadVertexBuffer)
{
	D3D12_HEAP_PROPERTIES prop = {};

	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	prop.Type = D3D12_HEAP_TYPE_UPLOAD;

	prop.CreationNodeMask = 1;

	prop.VisibleNodeMask = 1;

	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {  };

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

	desc.Alignment = 0;

	desc.Height = 1;

	desc.Width = sizeof(VertexData) * mesh.vertexDatas.size();

	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.DepthOrArraySize = 1;

	desc.MipLevels = 1;

	desc.Format = DXGI_FORMAT_UNKNOWN;

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadVertexBuffer.GetAddressOf())

	);

	if (FAILED(hr))
	{
		std::cerr << "VBVのアップロードリソース作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool MeshLoader::MapVB(MeshLoader::Mesh& mesh, ComPtr<ID3D12Resource>& uploadVertexBuffer)
{
	void* ptr = nullptr;

	hr = uploadVertexBuffer->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "頂点データのマッピング失敗" << std::endl;

		return false;
	}
	memcpy(ptr, mesh.vertexDatas.data(), sizeof(VertexData) * mesh.vertexDatas.size());

	uploadVertexBuffer->Unmap(0, nullptr);



	return true;
}

bool MeshLoader::VBUploadToDefultHeap(MeshLoader::Mesh& mesh , ComPtr<ID3D12Resource>& uploadVertexBuffer)
{
	D3D12_HEAP_PROPERTIES prop = {};

	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	prop.Type = D3D12_HEAP_TYPE_DEFAULT;

	prop.CreationNodeMask = 1;

	prop.VisibleNodeMask = 1;

	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {  };

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

	desc.Alignment = 0;

	desc.Height = 1;

	desc.Width = sizeof(VertexData) * mesh.vertexDatas.size();

	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.DepthOrArraySize = 1;

	desc.MipLevels = 1;

	desc.Format = DXGI_FORMAT_UNKNOWN;

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(mesh.vertexBuffer.GetAddressOf())
	);

	if (FAILED(hr))
	{
		std::cerr << "VBVのリソース作成失敗" << std::endl;

		return false;
	}

	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = mesh.vertexDatas.data();
	subresourceData.RowPitch = sizeof(VertexData) * mesh.vertexDatas.size();
	subresourceData.SlicePitch = subresourceData.RowPitch;

	DXRenderSystem::GetInstance().GetCommandList()->CopyBufferRegion(
		mesh.vertexBuffer.Get(), 
		0,            
		uploadVertexBuffer.Get(),  
		0,            
		sizeof(VertexData) * mesh.vertexDatas.size()); 

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mesh.vertexBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
	);
	DXRenderSystem::GetInstance().GetCommandList()->ResourceBarrier(1, &barrier);

	mesh.vbView.BufferLocation = mesh.vertexBuffer->GetGPUVirtualAddress();
	mesh.vbView.SizeInBytes = sizeof(VertexData) * mesh.vertexDatas.size();
	mesh.vbView.StrideInBytes = sizeof(VertexData);

	return true;
}

bool MeshLoader::InitIBVResource(MeshLoader::Mesh& mesh, ComPtr<ID3D12Resource>& uploadIndexBuffer)
{
	D3D12_HEAP_PROPERTIES prop = {};

	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	prop.Type = D3D12_HEAP_TYPE_UPLOAD;

	prop.CreationNodeMask = 1;

	prop.VisibleNodeMask = 1;

	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {  };

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

	desc.Alignment = 0;

	desc.Height = 1;

	desc.Width = sizeof(int) * mesh.indices.size();

	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.DepthOrArraySize = 1;

	desc.MipLevels = 1;

	desc.Format = DXGI_FORMAT_UNKNOWN;

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadIndexBuffer.GetAddressOf())

	);

	if (FAILED(hr))
	{
		std::cerr << "IBVのリソース作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool MeshLoader::MapIB(MeshLoader::Mesh& mesh, ComPtr<ID3D12Resource>& uploadIndexBuffer)
{
	void* ptr = nullptr;

	hr = uploadIndexBuffer->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "インデックスデータのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, mesh.indices.data(), sizeof(int) * mesh.indices.size());

	uploadIndexBuffer->Unmap(0, nullptr);

	return true;
}

bool MeshLoader::IBUploadToDefultHeap(MeshLoader::Mesh& mesh, ComPtr<ID3D12Resource>& uploadIndexBuffer)
{
	D3D12_HEAP_PROPERTIES prop = {};

	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	prop.Type = D3D12_HEAP_TYPE_DEFAULT;

	prop.CreationNodeMask = 1;

	prop.VisibleNodeMask = 1;

	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {  };

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

	desc.Alignment = 0;

	desc.Height = 1;

	desc.Width = sizeof(int) * mesh.indices.size();

	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.DepthOrArraySize = 1;

	desc.MipLevels = 1;

	desc.Format = DXGI_FORMAT_UNKNOWN;

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(mesh.indexBuffer.GetAddressOf())

	);

	if (FAILED(hr))
	{
		std::cerr << "IBVのリソース作成失敗" << std::endl;

		return false;
	}

	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = mesh.indices.data();
	subresourceData.RowPitch = sizeof(int) * mesh.indices.size();
	subresourceData.SlicePitch = subresourceData.RowPitch;

	DXRenderSystem::GetInstance().GetCommandList()->CopyBufferRegion(
		mesh.indexBuffer.Get(),
		0,               
		uploadIndexBuffer.Get(),   
		0,               
		sizeof(int) * mesh.indices.size()); 

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mesh.indexBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_INDEX_BUFFER
	);
	DXRenderSystem::GetInstance().GetCommandList()->ResourceBarrier(1, &barrier);

	mesh.ibView.BufferLocation = mesh.indexBuffer->GetGPUVirtualAddress();
	mesh.ibView.SizeInBytes = sizeof(int) * mesh.indices.size();;
	mesh.ibView.Format = DXGI_FORMAT_R32_UINT;

	return true;
}

