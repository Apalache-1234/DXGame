#ifndef UIRENDERER_H
#define UIRENDERER_H

#include "Renderer.h"

class UIRenderer : public Renderer, public std::enable_shared_from_this<UIRenderer>
{
public:
	UIRenderer(std::shared_ptr<Entity> ent);

	~UIRenderer();

	bool CreateTexture(const std::string fileName);

	UINT GetImageWidth();

	UINT GetImageHeight();
protected:
	void InitRenderer() override;

	void Update() override;
private:
	std::unique_ptr<DescriptorHeapManager::PtrHandles> texHandle;

	float imageWidth = 1;

	float imageHeight = 1;
};

#endif

