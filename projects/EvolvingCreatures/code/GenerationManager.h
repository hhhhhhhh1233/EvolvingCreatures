#pragma once

#include "config.h"
#include "Creature.h"
#include <PxPhysicsAPI.h>

struct CreatureStats
{
	Creature* Crea;
	float Fitness;
	physx::PxScene* Scene;
};

class GenerationManager
{
public:
	unsigned int mGenereationSize = 50;
	std::vector<CreatureStats> mCreatures;
	physx::PxPhysics* mPhysics;

	GenerationManager(physx::PxPhysics* Physics);

	/// This will populate the vector above with creatures and scenes with a plane, with mGenerationSize amount of creatures
	void GenerateCreatures(GraphicsNode Node);

	/// This will simulate the creatures and set the fitness value for the creature
	void RunGeneration();

	void StartGeneration();

	void EndGeneration();

	/// Remove creatures that don't have enough
	void CullGeneration(float MinimumFitnessValue);

	/// Mutate the creatures based on the ones that were fit to fill up the mCreatures vector to the set generation size
	void EvolveCreatures();
};
