#ifndef SKINNED_RENDERER_H
#define SKINNED_RENDERER_H

#include "Renderer.h"

class SkinnedRenderer : public Renderer, public std::enable_shared_from_this<SkinnedRenderer>
{
public:
	struct MeshData;

	SkinnedRenderer(std::shared_ptr<Entity> ent);

	~SkinnedRenderer();

	void StartAnimation(UINT index , float speed , bool isLoop);

	float GetAnimCurrentTime();

	float GetAnimTime();

	int GetAnimIndex();

	void SetAnimTime(float time);

	std::vector<string> GetAnimationNameList();

	void ResetAnimationStatue();

	bool isAnimPlaying = false;

	float animSpeed = 1;

	bool animLoop = false;

	std::type_index GetBaseType() override;
protected:
	void InitRenderer() override;

	void Update() override;

	void InitInCreateMesh() override;

	void AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList) override;
private:
	std::unique_ptr<DescriptorHeapManager::PtrHandles> boneHandle;

	ComPtr<ID3D12Resource> boneBuffer;

	int boneBufferParamNum = 0;

	std::vector<XMMATRIX> finalBoneMats;

	float animTimeCur = 0;

	float animTime = 0;

	UINT animIndex = 0;

	bool InitBoneResource();

	bool MapBB();

	bool InitSRVForBoneBuffer();

	void ReadHierarchyNode(aiNode& node, const aiMatrix4x4& parentMat);

	void ReadHierarchyNode(float animationTime, aiNode& node, aiMatrix4x4& parentMat, aiAnimation& anim);

	void ProcessAnimation(float time);
};

#endif