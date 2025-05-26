#ifndef SKYBOX_H
#define SKYBOX_H

#include "Renderer.h"

class SkyBox : public Renderer, public std::enable_shared_from_this<SkyBox>
{
public:
	SkyBox(std::shared_ptr<Entity> ent);

	~SkyBox();
protected:
	void Update() override;

	void InitRenderer() override;
private:
	std::unique_ptr<DescriptorHeapManager::PtrHandles> texHandle;

	bool CreateTexture();
};

#endif

