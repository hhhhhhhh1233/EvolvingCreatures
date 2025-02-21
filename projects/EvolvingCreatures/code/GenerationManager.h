#pragma once

#include "config.h"
#include "Creature.h"
#include <PxPhysicsAPI.h>

struct CreatureStats
{
	Creature* mCreature;
	float mFitness;
	physx::PxScene* mScene;
	float mAverageSpeed;
	float mSumHorizontalSpeed;

	CreatureStats(Creature* Crea, physx::PxScene* Scene)
	{
		mCreature = Crea;
		mFitness = 0;
		mScene = Scene;
		mAverageSpeed = 0;
		mSumHorizontalSpeed = 0;
	}
};

class GenerationManager
{
public:
	unsigned int mGenerationSize = 50;
	std::vector<CreatureStats*> mCreatures;
	physx::PxPhysics* mPhysics;

	/// Variables for keeping track of how long an evaluation period was, in seconds
	bool bEvaluating = false;
	std::chrono::steady_clock::time_point mEvaluationStartTime;
	float mEvaluationDuration;

	GenerationManager(physx::PxPhysics* Physics);

	/// This will populate the vector above with creatures and scenes with a plane, with mGenerationSize amount of creatures
	void GenerateCreatures(GraphicsNode Node);

	void Simulate(float StepSize);
	void UpdateCreatures();
	void DrawCreatures(mat4 ViewProjection);
	void Activate(float Vel);

	void StartEvalutation();
	void EndEvaluation();

	/// Remove creatures that don't have enough
	void CullGeneration(float MinimumFitnessValue);

	/// Mutate the creatures based on the ones that were fit to fill up the mCreatures vector to the set generation size
	void EvolveCreatures();
};
