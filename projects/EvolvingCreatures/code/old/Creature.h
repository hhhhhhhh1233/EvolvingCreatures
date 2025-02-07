#pragma once

#include "config.h"
#include <physx/PxPhysicsAPI.h>
#include "CreaturePart.h"

class Creature
{
public:
	physx::PxArticulationReducedCoordinate* mArticulation;
	CreaturePart* mRootPart;

	Creature(physx::PxPhysics* Physics, vec3 Position, std::shared_ptr<ShaderResource> Shader, vec3 RootScale = vec3(1, 1, 1));

	void AddToScene(physx::PxScene* Scene);
	void Update();
	void Draw(mat4 viewProjection);
};
