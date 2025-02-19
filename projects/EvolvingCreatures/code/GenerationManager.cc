#include "GenerationManager.h"

GenerationManager::GenerationManager(physx::PxPhysics* Physics) : mPhysics(Physics)
{
	/// Intentionally left blank
}

void GenerationManager::GenerateCreatures(GraphicsNode Node)
{
	for (int i = 0; i < mGenereationSize; i++)
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
