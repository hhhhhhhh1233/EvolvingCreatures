#include "Creature.h"

Creature::Creature(physx::PxPhysics* Physics, vec3 Position, std::shared_ptr<ShaderResource> Shader, vec3 RootScale)
{
	mArticulation = Physics->createArticulationReducedCoordinate();
	mArticulation->setArticulationFlag(physx::PxArticulationFlag::eDISABLE_SELF_COLLISION, true);

	mRootPart = new CreaturePart(Physics, mArticulation, Position, RootScale, Shader);
}

void Creature::AddToScene(physx::PxScene* Scene)
{
	Scene->addArticulation(*mArticulation);
}

void Creature::Update()
{
	mRootPart->Update();
}

void Creature::Draw(mat4 viewProjection)
{
	mRootPart->Draw(viewProjection);
}
