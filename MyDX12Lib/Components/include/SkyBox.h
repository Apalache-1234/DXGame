#ifndef SKYBOX_H
#define SKYBOX_H

#include "Renderer.h"

class SkyBox : public Renderer
{
public:
	SkyBox(std::shared_ptr<Entity> ent);

	~SkyBox();
protected:
	void Update() override;

	void InitRenderer() override;
private:
	std::unique_ptr<PtrHandles> texHandle;

	bool CreateTexture();
};

#endif

