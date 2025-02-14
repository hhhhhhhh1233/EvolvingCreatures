#pragma once

#include "config.h"
#include <physx/PxPhysicsAPI.h>
#include "CreaturePart.h"

class Creature
{
public:
	physx::PxArticulationReducedCoordinate* mArticulation;
	CreaturePart* mRootPart;
	
	Creature(physx::PxPhysics* Physics, physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags, GraphicsNode Node, vec3 Scale);
	~Creature();
	CreaturePart* GetChildlessPart() const;
	CreaturePart* GetRandomPart();
	std::vector<CreaturePart*> GetAllParts();
	std::vector<CreaturePart*> GetAllPartsFrom(CreaturePart* Part);
	void AddRandomPart(physx::PxPhysics* Physics, physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags, GraphicsNode Node);
	void SetPosition(vec3 Position);
	void AddToScene(physx::PxScene* Scene);
	void RemoveFromScene(physx::PxScene* Scene);
	void Update();
	void Activate(float Force);
	void Draw(mat4 ViewProjection);
};
