#include "FloorScript.h"
#include "SkinnedRenderer.h"
FloorScript::FloorScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

FloorScript::~FloorScript()
{
}

std::shared_ptr<SkinnedRenderer> sRend;

void FloorScript::OnStart()
{
	attachFrom->SetWorldPosition(DirectX::XMVectorSet(0,-5,0,1));
	attachFrom->SetWorldScale(DirectX::XMVectorSet(50,1,50,1));

	sRend = attachFrom->GetComponent<SkinnedRenderer>();

	Material mat;

	mat.UVSCALE = XMFLOAT2(10,10);

	sRend->SetMaterial(0 , mat);
}
