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
	void Activate(float TimePassed);
	void Draw(mat4 ViewProjection);

	void EnableGravity(bool NewState);

	Creature* GetMutatedCreature(physx::PxPhysics* Physics);
};

/// TODO: Implement these features so that interesting creatures can be saved for later
Creature* LoadCreatureFromFile(std::string FileName);
void SaveCreatureToFile(Creature* CreatureToSave, std::string FileName);
