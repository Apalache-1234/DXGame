#include "Renderer.h"
#include <memory>
#include "RayCast.h"
#include "Entity.h"
#include "DebugRenderer.h"

#ifndef DEBUG_RAYCAST_H
#define DEBUG_RAYCAST_H

class DebugRayCast : public DebugRenderer 
{
public:
	DebugRayCast(std::shared_ptr<Entity> ent);

	~DebugRayCast();
protected:

	void Update() override;
private:
	void InitDebugRenderer() override;
};

#endif

