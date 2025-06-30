#ifndef DEBUG_GRIDS_H 
#define DEBUG_GRIDS_H 

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

class DebugGrids : public DebugRenderer
{
public:
	DebugGrids(std::shared_ptr<Entity> ent);

	~DebugGrids();
protected:
	void Update() override;
private:
	void InitDebugRenderer() override;

	int HorizonNum = 0;
	int VerticalNum = 0;

	const int GripsNum = 100;
};

#endif