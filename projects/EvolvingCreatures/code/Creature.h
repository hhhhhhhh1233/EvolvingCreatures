#pragma once

#include "config.h"
#include <physx/PxPhysicsAPI.h>
#include "CreaturePart.h"
#include "BoundingBox.h"

class Creature
{
public:
	physx::PxArticulationReducedCoordinate* mArticulation;
	CreaturePart* mRootPart;
	std::map<CreaturePart*, BoundingBox> mShapes;
	
	Creature(physx::PxPhysics* Physics, physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags, GraphicsNode Node, vec3 Scale);
	~Creature();

	CreaturePart* GetChildlessPart() const;
	void RemoveChildlessPart();
	CreaturePart* GetRandomPart();
	std::vector<CreaturePart*> GetAllParts();
	std::vector<CreaturePart*> GetAllPartsFrom(CreaturePart* Part);
	void DrawBoundingBoxes(mat4 ViewProjection, vec3 Position, GraphicsNode Node);
	bool IsColliding(BoundingBox Box, CreaturePart* ToIgnore = nullptr);

	std::pair<BoundingBox, CreaturePart*> GetRandomShape();
	void AddRandomPart(physx::PxPhysics* Physics, physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags, GraphicsNode Node);
	void SetPosition(vec3 Position);
	void ClearForceAndTorque();

	void AddToScene(physx::PxScene* Scene);
	void RemoveFromScene(physx::PxScene* Scene);

	void Update();
	void Activate(float TimePassed);
	void Draw(mat4 ViewProjection);

	void EnableGravity(bool NewState);

	/// Mutation chance is a float from 0 to 1 that represents how likely a mutation is per randomization chance
	Creature* GetMutatedCreature(physx::PxPhysics* Physics, float MutationChance, float MutationSeverity);
};

/// TODO: Implement these features so that interesting creatures can be saved for later
Creature* LoadCreatureFromFile(std::string FileName);
void SaveCreatureToFile(Creature* CreatureToSave, std::string FileName);
