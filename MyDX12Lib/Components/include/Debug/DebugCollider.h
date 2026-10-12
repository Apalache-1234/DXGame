#ifndef DEBUG_COLLIDER_CUBE_H 
#define DEBUG_COLLIDER_CUBE_H 

#include <memory>
#include <iostream>
#include <vector>
#include "Entity.h"
#include "windows.h"
#include <sstream>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "Window.h"
#include "DXRenderSystem.h"
#include "Utility.h"
#include "Renderer.h"
#include "DebugRenderer.h"
#include "Collider.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

class DebugCollider : public DebugRenderer
{
public:
	DebugCollider(std::shared_ptr<Entity> ent, Collider::ColliderType type = Collider::ColliderType::Cube);

	~DebugCollider();
protected:

	void Update() override;
	void InitDebugColliderRenderer();
private:
	void InitDebugRenderer() override;

	Collider::ColliderType debugForType = Collider::ColliderType::Cube;

	std::vector<std::shared_ptr<Collider>> cols;
};

#endif
