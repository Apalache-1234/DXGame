#ifndef DEBUG_SPOT_LIGHT_H 
#define DEBUG_SPOT_LIGHT_H 

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

class DebugSpotLight : public DebugRenderer
{
public:

	DebugSpotLight(std::shared_ptr<Entity> ent);

	~DebugSpotLight();
protected:
	void Update() override;
private:
	void InitDebugRenderer() override;
};

#endif
