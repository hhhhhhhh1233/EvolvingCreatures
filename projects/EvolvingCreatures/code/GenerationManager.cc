#include "GenerationManager.h"
#include "RandomUtils.h"

GenerationManager::GenerationManager(physx::PxPhysics* Physics, physx::PxDefaultCpuDispatcher* Dispatcher, GraphicsNode CubeNode) : mPhysics(Physics), mDispatcher(Dispatcher), mCubeNode(CubeNode)
{
	/// Intentionally left blank
}

void GenerationManager::GenerateCreatures(int GenerationSize, bool bUseLoadedCreatures)
{
	/// Destroys any creatures that exist in the list already
	for (int i = 0; i < mCreatures.size(); i++)
	{
		delete mCreatures[i];
	}
	mCreatures.erase(mCreatures.begin(), mCreatures.end());

	mGenerationSize = GenerationSize;
	physx::PxMaterial* MaterialPtr = mPhysics->createMaterial(0.5f, 0.5f, 0.1f);
	physx::PxShapeFlags ShapeFlags = physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE;

	if (bUseLoadedCreatures)
	{
		for (auto LoadedBundle : mLoadedCreatures)
		{
			Creature* NewCreature = LoadedBundle->mCreature->GetCreatureCopy(mPhysics);
			NewCreature->SetPosition(vec3(0, 20, 0));

			/// ----------------------------------------
			/// [BEGIN] CREATURE PERSONAL SCENE SETUP
			/// ----------------------------------------

			physx::PxTolerancesScale ToleranceScale;

			ToleranceScale.length = 1;
			ToleranceScale.speed = 981;

			physx::PxSceneDesc SceneDesc(ToleranceScale);
			SceneDesc.gravity = { 0, -9.8, 0 };
			SceneDesc.cpuDispatcher = mDispatcher;
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

			NewCreature->AddToScene(Scene);
				
			CreatureBundle* NewCreatureStats = new CreatureBundle(NewCreature, Scene, PlaneCollision);
			mCreatures.push_back(NewCreatureStats);

			/// If someone theoretically loaded in more creatures then the generation allows only copy the amount that will fill the generation
			if (mCreatures.size() >= mGenerationSize)
				break;
		}
	}

	for (int i = mCreatures.size(); i < mGenerationSize; i++)
	{
		Creature* reature = new Creature(mPhysics, MaterialPtr, ShapeFlags, mCubeNode, vec3(RandomFloatInRange(0.5, 3), RandomFloatInRange(0.5, 3), RandomFloatInRange(0.5, 3)));

		int NumberOfBodyParts = RandomIntInRange(1, 4);
		for (int i = 0; i < NumberOfBodyParts; i++)
			reature->AddRandomPart(mPhysics, MaterialPtr, ShapeFlags, mCubeNode);

		reature->SetPosition(vec3(0, 20, 0));

		/// ----------------------------------------
		/// [BEGIN] CREATURE PERSONAL SCENE SETUP
		/// ----------------------------------------

		physx::PxTolerancesScale ToleranceScale;

		ToleranceScale.length = 1;
		ToleranceScale.speed = 981;

		physx::PxSceneDesc SceneDesc(ToleranceScale);
		SceneDesc.gravity = { 0, -9.8, 0 };
		SceneDesc.cpuDispatcher = mDispatcher;
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
			
		CreatureBundle* NewCreature = new CreatureBundle(reature, Scene, PlaneCollision);
		mCreatures.push_back(NewCreature);
	}

	MaterialPtr->release();
}

void GenerationManager::Simulate(float StepSize)
{
	for (auto Bundle : mCreatures)
	{
		Bundle->mScene->simulate(StepSize);
		Bundle->mScene->fetchResults(true);

		if (mCurrentState == GenerationManagerState::Running)
		{
			physx::PxVec3 Vel = Bundle->mCreature->mRootPart->mLink->getLinearVelocity();
			physx::PxVec3 HorizontalVel = { Vel.x, 0, Vel.z };
			Bundle->mSumHorizontalSpeed += HorizontalVel.magnitude();
		}
	}

	for (auto Bundle : mLoadedCreatures)
	{
		Bundle->mScene->simulate(StepSize);
		Bundle->mScene->fetchResults(true);
	}
}

void GenerationManager::UpdateCreatures(float dt)
{
	for (auto Bundle : mCreatures)
	{
		Bundle->mLifetime += dt;
		Bundle->mCreature->Update();
	}
}

void GenerationManager::DrawCreatures(mat4 ViewProjection)
{
	for (auto Bundle : mCreatures)
	{
		Bundle->mCreature->Draw(ViewProjection);
	}
}

void GenerationManager::DrawFinishedCreatures(mat4 ViewProjection, int CreatureIndex)
{
	/// Assert that sorted list is not empty and that you aren't sending an out of bounds index
	assert(mSortedCreatures.size() > 0 && CreatureIndex <= mSortedCreatures.size());

	mSortedCreatures[CreatureIndex].first->Draw(ViewProjection);
}

void GenerationManager::SetPositionOfCreatures(vec3 Position)
{
	for (auto Bundle : mCreatures)
	{
		//creature->mCreature->RemoveFromScene(creature->mScene);
		Bundle->mCreature->SetPosition(Position);
		//creature->mCreature->AddToScene(creature->mScene);
		Bundle->mCreature->ClearForceAndTorque();
	}
}

void GenerationManager::Activate()
{
	for (auto Bundle : mCreatures)
	{
		Bundle->mCreature->Activate(Bundle->mLifetime);
	}
}

void GenerationManager::Start(int NumberOfGenerations, float GenTime, int GenerationSurvivors, float MutationChance, float MutationSeverity, int GenerationSize, bool bUseLoadedCreatures)
{
	mCurrentState = GenerationManagerState::Running;

	GenerateCreatures(GenerationSize, bUseLoadedCreatures);

	/// Clear out all loaded creatures
	while (mLoadedCreatures.size() > 0)
	{
		RemoveLoadedCreature(0);
	}

	/// Clear the sorted list of victors
	mSortedCreatures.erase(mSortedCreatures.begin(), mSortedCreatures.end());

	mNumberOfGenerations = NumberOfGenerations;
	mGenerationDurationSeconds = GenTime;

	mGenerationSurvivors = GenerationSurvivors;
	mMutationChance = MutationChance;
	mMutationSeverity = MutationSeverity;

	mCurrentGeneration = 0;
	mCurrentGenerationDuration = 0;

	StartEvalutation();
}

/// Utility for checking if the creatures are sorted yet by their fitness, only used by cull generation and when a generation finishes so we can render the best ones
static bool IsSorted(const std::vector<std::pair<Creature*, float>>& Arr)
{
	for (int i = 0; i < Arr.size() - 1; i++)
	{
		if (Arr[i + 1].second > Arr[i].second)
			return false;
	}
	return true;
}

/// Utility for checking if the creatures are sorted yet by their fitness, only used by cull generation and when a generation finishes so we can render the best ones
static void Sort(std::vector<std::pair<Creature*, float>>& Arr)
{
	/// Sort the creatures based on their fitness
	while (!IsSorted(Arr))
	{
		for (int i = 0; i < Arr.size(); i++)
		{
			int j = i;
			while (j < Arr.size() - 1 && Arr[j].second < Arr[j + 1].second)
				j++;
			std::pair<Creature*, float> Temp = Arr[j];
			Arr[j] = Arr[i];
			Arr[i] = Temp;
		}
	}
}

void GenerationManager::Update(float DeltaTime)
{
	if (mCurrentState == GenerationManagerState::Running)
	{
		mCurrentGenerationDuration += DeltaTime;
		if (mCurrentGenerationDuration > mGenerationDurationSeconds)
		{
			mCurrentGenerationDuration = 0;
			mCurrentGeneration += 1;

			EndEvaluation();

			if (!(mCurrentGeneration >= mNumberOfGenerations))
			{
				CullAndMutateGeneration(mGenerationSurvivors, mMutationChance, mMutationSeverity);
				StartEvalutation();
			}

			SetPositionOfCreatures(vec3(0, 20, 0));

			if (mCurrentGeneration >= mNumberOfGenerations)
			{
				mCurrentState = GenerationManagerState::Finished;

				for (auto Bundle : mCreatures)
				{
					mSortedCreatures.push_back({ Bundle->mCreature, Bundle->mFitness });
				}

				/// Sort the creatures based on their fitness
				Sort(mSortedCreatures);
			}
		}
	}
}

void GenerationManager::StartEvalutation()
{
	for (auto Bundle : mCreatures)
	{
		Bundle->mSumHorizontalSpeed = 0;
	}

	mEvaluationStartTime = std::chrono::high_resolution_clock::now();
}

void GenerationManager::EndEvaluation()
{
	mEvaluationDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - mEvaluationStartTime).count() / 1000.f;

	for (auto Bundle : mCreatures)
	{
		Bundle->mAverageSpeed = Bundle->mSumHorizontalSpeed / mEvaluationDuration;

		/// This is to only consider horizontal movement interesting in fitness calculation
		physx::PxVec3 Pos = Bundle->mCreature->mRootPart->mLink->getGlobalPose().p;
		Pos = { Pos.x, 0, Pos.z };
		Bundle->mFitness = Pos.magnitude();
	}
}

void GenerationManager::CullAndMutateGeneration(int NumberToKeep, float MutationChance, float MutationSeverity)
{
	std::vector<std::pair<Creature*, float>> SortedCreatures;

	for (auto Bundle : mCreatures)
	{
		SortedCreatures.push_back({ Bundle->mCreature->GetCreatureCopy(mPhysics), Bundle->mFitness });
	}

	/// Sort the creatures based on their fitness
	Sort(SortedCreatures);

	/// Clear the creatures that are not up to snuff
	for (int i = NumberToKeep; i < SortedCreatures.size(); i++)
	{
		delete SortedCreatures[i].first;
	}
	SortedCreatures.erase(SortedCreatures.begin() + NumberToKeep, SortedCreatures.end());

	/// Delete the all of the CreatureBundle
	for (int i = 0; i < mCreatures.size(); i++)
	{
		delete mCreatures[i];
	}
	mCreatures.erase(mCreatures.begin(), mCreatures.end());

	physx::PxShapeFlags ShapeFlags = physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE;
	physx::PxMaterial* MaterialPtr = mPhysics->createMaterial(0.5f, 0.5f, 0.1f);

	/// Refill the mCreatures array with creatures based on mutations from the fittest
	for (int i = 0; i < mGenerationSize; i++)
	{
		/// ----------------------------------------
		/// [BEGIN] CREATURE PERSONAL SCENE SETUP
		/// ----------------------------------------

		physx::PxTolerancesScale ToleranceScale;

		ToleranceScale.length = 1;
		ToleranceScale.speed = 981;

		physx::PxSceneDesc SceneDesc(ToleranceScale);
		SceneDesc.gravity = { 0, -9.8, 0 };
		SceneDesc.cpuDispatcher = mDispatcher;
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
		Creature* MutatedCreature = SortedCreatures[i % SortedCreatures.size()].first->GetMutatedCreature(mPhysics, MutationChance, MutationSeverity);
		MutatedCreature->AddToScene(Scene);

		CreatureBundle* a = new CreatureBundle(MutatedCreature, Scene, PlaneCollision);

		mCreatures.push_back(a);
	}

	MaterialPtr->release();
}

void GenerationManager::LoadCreature(std::string FileName)
{
	physx::PxShapeFlags ShapeFlags = physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE;
	physx::PxMaterial* MaterialPtr = mPhysics->createMaterial(0.5f, 0.5f, 0.1f);

	Creature* LoadedCreature = LoadCreatureFromFile(FileName, mPhysics, MaterialPtr, ShapeFlags, mCubeNode);

	/// ----------------------------------------
	/// [BEGIN] CREATURE PERSONAL SCENE SETUP
	/// ----------------------------------------

	physx::PxTolerancesScale ToleranceScale;

	ToleranceScale.length = 1;
	ToleranceScale.speed = 981;

	physx::PxSceneDesc SceneDesc(ToleranceScale);
	SceneDesc.gravity = { 0, -9.8, 0 };
	SceneDesc.cpuDispatcher = mDispatcher;
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

	CreatureBundle* Stats = new CreatureBundle(LoadedCreature, Scene, PlaneCollision);

	LoadedCreature->AddToScene(Scene);

	LoadedCreature->SetPosition(vec3(0, 20, 0));

	mLoadedCreatures.push_back(Stats);

	/// Push back the filename as the creatures name
	{
		char* CreatureName = new char[30];
		std::string Temp = FileName;
		if (Temp.find_last_of("/") != std::string::npos)
		{
			Temp = Temp.substr(Temp.find_last_of("/") + 1, Temp.length());
		}

		if (Temp.find_last_of("\\") != std::string::npos)
		{
			Temp = Temp.substr(Temp.find_last_of("\\") + 1, Temp.length());
		}

		strcpy(CreatureName, Temp.c_str());
		mLoadedCreatureNames.push_back(CreatureName);
	}


	MaterialPtr->release();
}

void GenerationManager::UpdateAndDrawLoadedCreatures(mat4 ViewProjection, float dt)
{
	for (auto Bundle : mLoadedCreatures)
	{
		Bundle->mLifetime += dt;
		Bundle->mCreature->Update();
		Bundle->mCreature->Draw(ViewProjection);
	}
}

void GenerationManager::ActivateLoadedCreatures()
{
	for (auto Bundle : mLoadedCreatures)
	{
		if (Bundle->bActive)
			Bundle->mCreature->Activate(Bundle->mLifetime);
	}
}

void GenerationManager::SetLoadedCreaturePosition(int CreatureIndex, vec3 Position)
{
	/// Must be in range
	assert(CreatureIndex >= 0 && CreatureIndex < mLoadedCreatures.size());

	mLoadedCreatures[CreatureIndex]->mCreature->SetPosition(Position);
}

void GenerationManager::RemoveLoadedCreature(int CreatureIndex)
{
	/// Must be in range
	assert(CreatureIndex >= 0 && CreatureIndex < mLoadedCreatures.size());

	//mLoadedCreatures[CreatureIndex]->mCreature->RemoveFromScene(mLoadedCreatures[CreatureIndex]->mScene);
	//delete mLoadedCreatures[CreatureIndex]->mCreature;
	//mLoadedCreatures[CreatureIndex]->mScene->removeActor(*mLoadedCreatures[CreatureIndex]->mPlaneCollision);
	//mLoadedCreatures[CreatureIndex]->mPlaneCollision->release();
	//mLoadedCreatures[CreatureIndex]->mScene->release();
	delete mLoadedCreatures[CreatureIndex];
	mLoadedCreatures.erase(mLoadedCreatures.begin() + CreatureIndex);

	delete mLoadedCreatureNames[CreatureIndex];
	mLoadedCreatureNames.erase(mLoadedCreatureNames.begin() + CreatureIndex);
}
