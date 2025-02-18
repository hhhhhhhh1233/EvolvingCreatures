#include "GenerationManager.h"

void GenereationManager::GenerateCreatures(GraphicsNode Node)
{
	for (int i = 0; i < mGenereationSize; i++)
	{
		CreatureStats NewCreature;
		physx::PxShapeFlags ShapeFlags = physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE;
		physx::PxMaterial* MaterialPtr = mPhysics->createMaterial(0.5f, 0.5f, 0.1f);
		NewCreature.Crea = new Creature(mPhysics, MaterialPtr, ShapeFlags, Node, vec3(1,1,1));

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

		NewCreature.Scene = mPhysics->createScene(SceneDesc);
		NewCreature.Scene->setFlag(physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS, true);

		physx::PxRigidStatic* PlaneCollision = mPhysics->createRigidStatic(physx::PxTransformFromPlaneEquation(physx::PxPlane(physx::PxVec3(0.f, 1.f, 0.f), 0.f)));
		{
			physx::PxShape* shape = mPhysics->createShape(physx::PxPlaneGeometry(), &MaterialPtr, 1, true, ShapeFlags);
			PlaneCollision->attachShape(*shape);
			shape->release();
		}

		NewCreature.Scene->addActor(*PlaneCollision);

		/// ----------------------------------------
		/// [END] CREATURE PERSONAL SCENE SETUP
		/// ----------------------------------------
			
		mCreatures.push_back(NewCreature);
	}
}
