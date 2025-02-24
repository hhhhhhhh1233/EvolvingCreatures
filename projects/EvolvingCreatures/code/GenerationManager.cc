#include "GenerationManager.h"

GenerationManager::GenerationManager(physx::PxPhysics* Physics) : mPhysics(Physics)
{
	/// Intentionally left blank
}

void GenerationManager::GenerateCreatures(GraphicsNode Node)
{
	for (int i = 0; i < mGenerationSize; i++)
	{
		physx::PxShapeFlags ShapeFlags = physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE;
		physx::PxMaterial* MaterialPtr = mPhysics->createMaterial(0.5f, 0.5f, 0.1f);
		Creature* reature = new Creature(mPhysics, MaterialPtr, ShapeFlags, Node, vec3(2,1,1));
		reature->AddRandomPart(mPhysics, MaterialPtr, ShapeFlags, Node);
		reature->AddRandomPart(mPhysics, MaterialPtr, ShapeFlags, Node);
		reature->SetPosition(vec3(0, 10, 0));

		/// ----------------------------------------
		/// [BEGIN] CREATURE PERSONAL SCENE SETUP
		/// ----------------------------------------

		physx::PxDefaultCpuDispatcher* Dispatcher = NULL;

		physx::PxTolerancesScale ToleranceScale;

		ToleranceScale.length = 1;
		ToleranceScale.speed = 981;

		physx::PxSceneDesc SceneDesc(ToleranceScale);
		SceneDesc.gravity = { 0, -9.8, 0 };
		Dispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		SceneDesc.cpuDispatcher = Dispatcher;
		SceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		SceneDesc.kineKineFilteringMode = physx::PxPairFilteringMode::eKEEP;
		SceneDesc.staticKineFilteringMode = physx::PxPairFilteringMode::eKEEP;

		physx::PxScene* Scene = mPhysics->createScene(SceneDesc);
		Scene->setFlag(physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS, true);

		physx::PxRigidStatic* PlaneCollision = mPhysics->createRigidStatic(physx::PxTransformFromPlaneEquation(physx::PxPlane(physx::PxVec3(0.f, 1.f, 0.f), 0.f)));
		{
			physx::PxShape* shape = mPhysics->createShape(physx::PxPlaneGeometry(), &MaterialPtr, 1, true, ShapeFlags);
			PlaneCollision->attachShape(*shape);
			shape->release();
		}

		Scene->addActor(*PlaneCollision);

		/// ----------------------------------------
		/// [END] CREATURE PERSONAL SCENE SETUP
		/// ----------------------------------------

		reature->AddToScene(Scene);
			
		CreatureStats* NewCreature = new CreatureStats(reature, Scene);
		mCreatures.push_back(NewCreature);
	}
}

void GenerationManager::Simulate(float StepSize)
{
	for (auto creature : mCreatures)
	{
		creature->mScene->simulate(StepSize);
		creature->mScene->fetchResults(true);

		if (bEvaluating)
		{
			physx::PxVec3 Vel = creature->mCreature->mRootPart->mLink->getLinearVelocity();
			physx::PxVec3 HorizontalVel = { Vel.x, 0, Vel.z };
			creature->mSumHorizontalSpeed += HorizontalVel.magnitude();
		}
	}
}

void GenerationManager::UpdateCreatures()
{
	for (auto creature : mCreatures)
	{
		creature->mCreature->Update();
	}
}

void GenerationManager::DrawCreatures(mat4 ViewProjection)
{
	for (auto creature : mCreatures)
	{
		creature->mCreature->Draw(ViewProjection);
	}
}

void GenerationManager::Activate(float Force)
{
	for (auto creature : mCreatures)
	{
		creature->mCreature->Activate(Force);
	}
}

void GenerationManager::StartEvalutation()
{
	for (auto creature : mCreatures)
	{
		creature->mSumHorizontalSpeed = 0;
	}

	bEvaluating = true;
	mEvaluationStartTime = std::chrono::high_resolution_clock::now();
}

void GenerationManager::EndEvaluation()
{
	mEvaluationDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - mEvaluationStartTime).count() / 1000.f;
	bEvaluating = false;

	for (auto creature : mCreatures)
	{
		creature->mAverageSpeed = creature->mSumHorizontalSpeed / mEvaluationDuration;

		creature->mFitness = creature->mAverageSpeed;
	}
}

static bool IsSorted(const std::vector<std::pair<Creature*, float>>& Arr)
{
	for (int i = 0; i < Arr.size() - 1; i++)
	{
		if (Arr[i + 1].second > Arr[i].second)
			return false;
	}
	return true;
}

void GenerationManager::CullGeneration(int NumberToKeep)
{
	std::vector<std::pair<Creature*, float>> SortedCreatures;

	for (auto creature : mCreatures)
	{
		SortedCreatures.push_back({ creature->mCreature, creature->mFitness });
		creature->mCreature->RemoveFromScene(creature->mScene);
	}

	while (!IsSorted(SortedCreatures))
	{
		for (int i = 0; i < SortedCreatures.size(); i++)
		{
			int j = i;
			while (j < SortedCreatures.size() - 1 && SortedCreatures[j].second < SortedCreatures[j + 1].second)
				j++;
			std::pair<Creature*, float> Temp = SortedCreatures[j];
			SortedCreatures[j] = SortedCreatures[i];
			SortedCreatures[i] = Temp;
		}
	}

	for (int i = NumberToKeep; i < SortedCreatures.size(); i++)
	{
		delete SortedCreatures[i].first;
	}
	SortedCreatures.erase(SortedCreatures.begin() + NumberToKeep, SortedCreatures.end());

	mCreatures.erase(mCreatures.begin(), mCreatures.end());
	for (int i = 0; i < SortedCreatures.size(); i++)
	{
		physx::PxShapeFlags ShapeFlags = physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE;
		physx::PxMaterial* MaterialPtr = mPhysics->createMaterial(0.5f, 0.5f, 0.1f);

		/// ----------------------------------------
		/// [BEGIN] CREATURE PERSONAL SCENE SETUP
		/// ----------------------------------------

		physx::PxDefaultCpuDispatcher* Dispatcher = NULL;

		physx::PxTolerancesScale ToleranceScale;

		ToleranceScale.length = 1;
		ToleranceScale.speed = 981;

		physx::PxSceneDesc SceneDesc(ToleranceScale);
		SceneDesc.gravity = { 0, -9.8, 0 };
		Dispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		SceneDesc.cpuDispatcher = Dispatcher;
		SceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		SceneDesc.kineKineFilteringMode = physx::PxPairFilteringMode::eKEEP;
		SceneDesc.staticKineFilteringMode = physx::PxPairFilteringMode::eKEEP;

		physx::PxScene* Scene = mPhysics->createScene(SceneDesc);
		Scene->setFlag(physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS, true);

		physx::PxRigidStatic* PlaneCollision = mPhysics->createRigidStatic(physx::PxTransformFromPlaneEquation(physx::PxPlane(physx::PxVec3(0.f, 1.f, 0.f), 0.f)));
		{
			physx::PxShape* shape = mPhysics->createShape(physx::PxPlaneGeometry(), &MaterialPtr, 1, true, ShapeFlags);
			PlaneCollision->attachShape(*shape);
			shape->release();
		}

		Scene->addActor(*PlaneCollision);

		/// ----------------------------------------
		/// [END] CREATURE PERSONAL SCENE SETUP
		/// ----------------------------------------

		SortedCreatures[i].first->AddToScene(Scene);

		CreatureStats* a = new CreatureStats(SortedCreatures[i].first, Scene);

		mCreatures.push_back(a);
	}
}

void GenerationManager::EvolveCreatures()
{
	/// HAVE THESE BE PASSED IN INSTEAD
	float MutationChance = 0.3;
	float MutationSeverity = 0.5;

	physx::PxShapeFlags ShapeFlags = physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE;
	physx::PxMaterial* MaterialPtr = mPhysics->createMaterial(0.5f, 0.5f, 0.1f);

	int NumberOfCreaturesToEvolve = mCreatures.size();

	int i = 0;
	while (mCreatures.size() < mGenerationSize)
	{
		Creature* Creat = mCreatures[i]->mCreature->GetMutatedCreature(mPhysics, MutationChance, MutationSeverity);
		Creat->SetPosition(vec3(0, 10, 0));

		/// ----------------------------------------
		/// [BEGIN] CREATURE PERSONAL SCENE SETUP
		/// ----------------------------------------

		physx::PxDefaultCpuDispatcher* Dispatcher = NULL;

		physx::PxTolerancesScale ToleranceScale;

		ToleranceScale.length = 1;
		ToleranceScale.speed = 981;

		physx::PxSceneDesc SceneDesc(ToleranceScale);
		SceneDesc.gravity = { 0, -9.8, 0 };
		Dispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		SceneDesc.cpuDispatcher = Dispatcher;
		SceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		SceneDesc.kineKineFilteringMode = physx::PxPairFilteringMode::eKEEP;
		SceneDesc.staticKineFilteringMode = physx::PxPairFilteringMode::eKEEP;

		physx::PxScene* Scene = mPhysics->createScene(SceneDesc);
		Scene->setFlag(physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS, true);

		physx::PxRigidStatic* PlaneCollision = mPhysics->createRigidStatic(physx::PxTransformFromPlaneEquation(physx::PxPlane(physx::PxVec3(0.f, 1.f, 0.f), 0.f)));
		{
			physx::PxShape* shape = mPhysics->createShape(physx::PxPlaneGeometry(), &MaterialPtr, 1, true, ShapeFlags);
			PlaneCollision->attachShape(*shape);
			shape->release();
		}

		Scene->addActor(*PlaneCollision);

		/// ----------------------------------------
		/// [END] CREATURE PERSONAL SCENE SETUP
		/// ----------------------------------------

		CreatureStats* a = new CreatureStats(Creat, Scene);

		Creat->AddToScene(Scene);

		mCreatures.push_back(a);

		i = (i + 1) % NumberOfCreaturesToEvolve;
	}
}
