#include "Component.h"

Component::Component(std::shared_ptr<Entity> ent)
{
	attachFrom = ent;
}

Component::~Component()
{
	attachFrom.reset();
}

void Component::Init() {

}