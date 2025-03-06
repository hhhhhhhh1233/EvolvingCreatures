#pragma once

#include "config.h"
#include "Creature.h"
#include <PxPhysicsAPI.h>
#include "render/GraphicsNode.h"

struct CreatureBundle
{
	Creature* mCreature;
	float mFitness;
	physx::PxScene* mScene;
	physx::PxRigidStatic* mPlaneCollision;
	float mAverageSpeed;
	float mSumHorizontalSpeed;
	float mLifetime;
	bool bActive = true;
	bool bDrawBoundingBox = false;

	CreatureBundle(Creature* Crea, physx::PxScene* Scene, physx::PxRigidStatic* PlaneCollision)
	{
		mCreature = Crea;
		mFitness = 0;
		mScene = Scene;
		mPlaneCollision = PlaneCollision;
		mAverageSpeed = 0;
		mSumHorizontalSpeed = 0;
		mLifetime = 0;
	}

	~CreatureBundle()
	{
		mCreature->RemoveFromScene(mScene);
		delete mCreature;
		mScene->removeActor(*mPlaneCollision);
		mPlaneCollision->release();
		mScene->release();
	}

};

enum GenerationManagerState {
	Nothing,
	Running,
	Finished,
	Waiting,
};

class GenerationManager
{
public:

/// FIELDS
	unsigned int mGenerationSize = 50;
	std::vector<CreatureBundle*> mCreatures;
	physx::PxPhysics* mPhysics;
	physx::PxDefaultCpuDispatcher* mDispatcher = NULL;

	GraphicsNode mCubeNode;

	GenerationManagerState mCurrentState = GenerationManagerState::Nothing;

	unsigned int mCurrentGeneration = 0;
	unsigned int mNumberOfGenerations = 0;

	float mGenerationDurationSeconds = 60.0f;
	float mCurrentGenerationDuration = 0.0f;

	int mGenerationSurvivors = 0; 
	float mMutationChance = 0; 
	float mMutationSeverity = 0;

	/// Variables for keeping track of how long an evaluation period was, in seconds
	std::chrono::steady_clock::time_point mEvaluationStartTime;
	float mEvaluationDuration = 0;

	std::vector<std::pair<Creature*, float>> mSortedCreatures;

	/// These are not part of the generations, they are loaded in from file by the user
	std::vector<CreatureBundle*> mLoadedCreatures;
	std::vector<char*> mLoadedCreatureNames;

/// METHODS
	GenerationManager(physx::PxPhysics* Physics, physx::PxDefaultCpuDispatcher* Dispatcher, GraphicsNode CubeNode);

	/// This will populate the vector above with creatures and scenes with a plane, with mGenerationSize amount of creatures
	void GenerateCreatures(int GenerationSize, bool bUseLoadedCreatures);

	void Simulate(float StepSize);
	void UpdateCreatures(float dt);
	void DrawCreatures(mat4 ViewProjection);
	void DrawFinishedCreatures(mat4 ViewProjection, int CreatureIndex);
	void SetPositionOfCreatures(vec3 Position);
	void Activate();

	void Start(int NumberOfGenerations, float GenTime, int GenerationSurvivors, float MutationChance, float MutationSeverity, int GenerationSize, bool bUseLoadedCreatures);
	void Update(float DeltaTime);

	void StartEvalutation();
	void EndEvaluation();

	/// This is the fundamental method of this class, that will
	void CullAndMutateGeneration(int NumberToKeep, float MutationChance, float MutationSeverity);

	void LoadCreature(std::string FileName);
	void UpdateAndDrawLoadedCreatures(mat4 ViewProjection, float dt);
	void SetLoadedCreaturePosition(int CreatureIndex, vec3 Position);
	void RemoveLoadedCreature(int CreatureIndex);
	void ActivateLoadedCreatures();
};
