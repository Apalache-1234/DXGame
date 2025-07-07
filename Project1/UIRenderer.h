#ifndef UIRENDERER_H
#define UIRENDERER_H

#include "Renderer.h"

class UIRenderer : public Renderer, public std::enable_shared_from_this<UIRenderer>
{
public:
	UIRenderer(std::shared_ptr<Entity> ent);

	~UIRenderer();

	D3D12_CPU_DESCRIPTOR_HANDLE GetTextureCPUHandle();

	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureGPUHandle();

	bool CreateTexture(const std::string fileName);

	UINT GetImageWidth();

	UINT GetImageHeight();

	void SetImageWidth(UINT size);

	void SetImageHeight(UINT size);

	XMFLOAT2 GetResultScale();

	DirectX::XMVECTOR colPosition = DirectX::XMVectorZero();

	DirectX::XMVECTOR colScale = DirectX::XMVectorSplatOne();
protected:
	void InitRenderer() override;

	void Update() override;

	Transform GetTransform();

	void CheckPointerInbound();
private:
	std::unique_ptr<DescriptorHeapManager::PtrHandles> texHandle;

	UINT imageWidth = 1;

	UINT imageHeight = 1;

	float isOnMousePointer = false;
};

#endif

