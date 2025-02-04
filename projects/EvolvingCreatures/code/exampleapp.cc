//------------------------------------------------------------------------------
// exampleapp.cc
// (C) 2015-2022 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "config.h"
#include "exampleapp.h"
#include <cstring>
#include "render/GraphicsNode.h"
#include "render/camera.h"
#include "render/grid.h"
#include "render/PointLightSource.h"
#include <chrono>
#include <PxPhysicsAPI.h>
#include <PxPhysics.h>
#include <pvd/PxPvd.h>
#include <pvd/PxPvdTransport.h>
#include <pvd/PxPvdSceneClient.h>

#define FILE_ROOT "C:\\Users\\tagtje-1\\Documents\\EvolvingCreatures\\ProjectFiles\\"

using namespace Display;
namespace Example
{

//------------------------------------------------------------------------------
/**
*/
ExampleApp::ExampleApp()
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
ExampleApp::~ExampleApp()
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
bool
ExampleApp::Open()
{
	App::Open();
	this->window = new Display::Window;
	window->SetKeyPressFunction([this](int32, int32, int32, int32)
	{
		if (glfwGetKey(this->window->window, GLFW_KEY_ESCAPE))
			this->window->Close();
	});


	if (this->window->Open())
	{
		// set clear color to gray
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
/**
*/
void
ExampleApp::Close()
{
	if (this->window->IsOpen())
		this->window->Close();

	Core::App::Close();
}

//------------------------------------------------------------------------------
/**
*/
void
ExampleApp::Run()
{

	TextureResource fur;
	fur.LoadFromFile(FILE_ROOT "images\\fur.jpg");

	TextureResource grad;
	grad.LoadFromFile(FILE_ROOT "images\\gradientSample3.jpg");

	std::shared_ptr<ShaderResource> shader = std::make_shared<ShaderResource>();
	shader->LoadShaders(FILE_ROOT "Shaders\\materialShader.vert", FILE_ROOT "Shaders\\materialShader.frag");

	std::shared_ptr<ShaderResource> lightingShader = std::make_shared<ShaderResource>();
	lightingShader->LoadShaders(FILE_ROOT "Shaders\\lightingShader.vert", FILE_ROOT "Shaders\\lightingShader.frag");

	MeshResource sphereMesh;
	sphereMesh.LoadOBJ(FILE_ROOT "objs\\sphere.obj");

	GraphicsNode sphere = GraphicsNode(std::make_shared<MeshResource>(std::move(sphereMesh)), std::make_shared<TextureResource>(fur), shader, mat4(), 32);

	//GraphicsNode helmet = LoadGLTF(FILE_ROOT "glTFs\\DamagedHelmetglTF\\", "DamagedHelmet.gltf", shader);
	//GraphicsNode avocado = LoadGLTF(FILE_ROOT "glTFs\\AvocadoglTF\\", "Avocado.gltf", shader);
	GraphicsNode cube = LoadGLTF(FILE_ROOT "glTFs\\CubeglTF\\", "Cube.gltf", lightingShader);
	GraphicsNode armCube = LoadGLTF(FILE_ROOT "glTFs\\CubeglTF\\", "Cube.gltf", lightingShader);
	//GraphicsNode flightHelmet = LoadGLTF(FILE_ROOT "glTFs\\FlightHelmetglTF\\", "FlightHelmet.gltf", shader);
	//GraphicsNode normalTangentMirrorTest = LoadGLTF(FILE_ROOT "glTFs\\NormalTangentMirrorTestglTF\\", "NormalTangentMirrorTest.gltf", shader);
	GraphicsNode Quad(std::make_shared<MeshResource>(CreateQuad(30, 30)), std::make_shared<TextureResource>(fur), lightingShader, rotationx(3.14/2), 1);
	
	mat4 projection = perspective(3.14f / 2, window->GetAspectRatio(), 0.1f, 100);
	
	Camera cam;
	cam.position = vec3(0, 1, 3);

	/// ------------------------------------------
	/// [BEGIN] LIGHT SETUP
	/// ------------------------------------------

	PointLightSource light;
	light.position = vec3(0, 0, -2);
	light.color = vec3(1, 1, 1);
	light.intensity = 1;

	DirectionalLightSource sun;
	sun.direction = vec3(0, -1, -3);
	sun.color = vec3(1, 1, 1);
	Render::Grid grid;

	/// ------------------------------------------
	/// [END] LIGHT SETUP
	/// ------------------------------------------

	auto appStart = std::chrono::high_resolution_clock::now();

	auto start = std::chrono::high_resolution_clock::now();

	/// ------------------------------------------
	/// [BEGIN] INIT PHYSICS
	/// ------------------------------------------

	physx::PxDefaultAllocator mDefaultAllocatorCallback;
	physx::PxDefaultErrorCallback mDefaultErrorCallback;

	physx::PxFoundation* mFoundation = NULL;
	mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
	if (!mFoundation)
	{
		std::cout << "Completely broken!\n";
		return;
	}

	physx::PxPhysics* mPhysics = NULL;
	physx::PxPvd* mPvd = NULL;

	bool bRecordMemoryAllocation = true;
	mPvd = physx::PxCreatePvd(*mFoundation);
	physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("THIS IS WRONG", 5425, 10);
	mPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

	mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, physx::PxTolerancesScale(), bRecordMemoryAllocation, mPvd);
	if (!mPhysics)
	{
		std::cout << "Completely broken!\n";
		return;
	}

	physx::PxDefaultCpuDispatcher* mDispatcher = NULL;

	physx::PxTolerancesScale mToleranceScale;

	mToleranceScale.length = 1;
	mToleranceScale.speed = 981;

	physx::PxSceneDesc mSceneDesc(mToleranceScale);
	mSceneDesc.gravity = { 0, -9.8, 0 };
	mDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	mSceneDesc.cpuDispatcher = mDispatcher;
	mSceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	mSceneDesc.kineKineFilteringMode = physx::PxPairFilteringMode::eKEEP;
	mSceneDesc.staticKineFilteringMode = physx::PxPairFilteringMode::eKEEP;

	physx::PxScene* mScene = mPhysics->createScene(mSceneDesc);
	mScene->setFlag(physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS, true);

	/// ------------------------------------------
	/// [END] INIT PHYSICS
	/// ------------------------------------------

	/// ------------------------------------------
	/// [BEGIN] CREATE ACTORS
	/// ------------------------------------------

	physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE;
	physx::PxMaterial* materialPtr = mPhysics->createMaterial(0.5f, 0.5f, 0.1f);

	physx::PxRigidStatic* rigidStatic = mPhysics->createRigidStatic(physx::PxTransformFromPlaneEquation(physx::PxPlane(physx::PxVec3(0.f, 1.f, 0.f), 0.f)));
	{
		physx::PxShape* shape = mPhysics->createShape(physx::PxPlaneGeometry(), &materialPtr, 1, true, shapeFlags);
		rigidStatic->attachShape(*shape);
		shape->release();
	}

	physx::PxRigidDynamic* rigidDynamic = mPhysics->createRigidDynamic(physx::PxTransform(physx::PxVec3(0.f, 5.5f, 0.f)));
	{
		physx::PxShape* shape = mPhysics->createShape(physx::PxBoxGeometry(0.5f, 2.f, 0.5f), &materialPtr, 1, true, shapeFlags);
		rigidDynamic->attachShape(*shape);
		shape->release();
	}

	physx::PxRigidDynamic* rigidDynamicArm = mPhysics->createRigidDynamic(physx::PxTransform(physx::PxVec3(1.f, 5.5f, 0.f)));
	{
		physx::PxShape* shape = mPhysics->createShape(physx::PxBoxGeometry(0.5f, 0.5f, 0.5f), &materialPtr, 1, true, shapeFlags);
		rigidDynamicArm->attachShape(*shape);
		shape->release();
	}

	mScene->addActor(*rigidStatic);
	mScene->addActor(*rigidDynamic);
	mScene->addActor(*rigidDynamicArm);

	physx::PxRevoluteJoint* joint = physx::PxRevoluteJointCreate(*mPhysics, rigidDynamic, rigidDynamic->getGlobalPose(), rigidDynamicArm, rigidDynamicArm->getGlobalPose());

	joint->setLimit(physx::PxJointAngularLimitPair(-physx::PxPi / 4, physx::PxPi / 4));
	joint->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eLIMIT_ENABLED, true);

	/// ------------------------------------------
	/// [END] CREATE ACTORS
	/// ------------------------------------------

	float mAccumulator = 0.0f;
	float mStepSize = 1.0f / 60.0f;

	while (this->window->IsOpen())
	{
		auto end = std::chrono::high_resolution_clock::now();
		float deltaseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0f;
		start = std::chrono::high_resolution_clock::now();

		//rigidDynamic->addForce({ 0,5,0 }); /// WORKS!!!
		//rigidDynamic->addTorque({ 2,0,0 }); /// WORKS!!!

		mAccumulator += deltaseconds;
		if (mAccumulator > mStepSize)
		{
			mScene->simulate(mStepSize);
			mScene->fetchResults(true);

			mAccumulator -= mStepSize;
		}

		/// ---------------------------------------- 
		/// [BEGIN] GET CUBE POS AND ROT
		/// ---------------------------------------- 

		physx::PxVec3 dynPos = rigidDynamic->getGlobalPose().p;

		/// I couldn't figure out a better way to get the rotation out of PhysX
		mat4 NewDynRotMat;
		{
			physx::PxQuat dynQuat = rigidDynamic->getGlobalPose().q;
			auto xVec = dynQuat.getBasisVector0();
			auto yVec = dynQuat.getBasisVector1();
			auto zVec = dynQuat.getBasisVector2();
			NewDynRotMat = mat4(vec4(xVec.x, xVec.y, xVec.z, 0), vec4(yVec.x, yVec.y, yVec.z, 0), vec4(zVec.x, zVec.y, zVec.z, 0), vec4(0, 0, 0, 1));
		}

		/// ---------------------------------------- 
		/// [END] GET CUBE POS AND ROT
		/// ---------------------------------------- 

		/// ---------------------------------------- 
		/// [BEGIN] GET CUBE ARM POS AND ROT
		/// ---------------------------------------- 

		physx::PxVec3 dynArmPos = rigidDynamicArm->getGlobalPose().p;

		/// I couldn't figure out a better way to get the rotation out of PhysX
		mat4 NewDynArmRotMat;
		{
			physx::PxQuat dynArmQuat = rigidDynamicArm->getGlobalPose().q;
			auto xVec = dynArmQuat.getBasisVector0();
			auto yVec = dynArmQuat.getBasisVector1();
			auto zVec = dynArmQuat.getBasisVector2();
			NewDynArmRotMat = mat4(vec4(xVec.x, xVec.y, xVec.z, 0), vec4(yVec.x, yVec.y, yVec.z, 0), vec4(zVec.x, zVec.y, zVec.z, 0), vec4(0, 0, 0, 1));
		}

		/// ---------------------------------------- 
		/// [END] GET CUBE ARM POS AND ROT
		/// ---------------------------------------- 
		//std::cout << "Dyn Accel Magnitude: " << rigidDynamic->getLinearAcceleration().magnitude() << "\n";
		if (glfwGetKey(window->window, GLFW_KEY_R) == GLFW_PRESS)
			rigidDynamic->addForce({ 0, 15, 0 });

		if (glfwGetKey(window->window, GLFW_KEY_T) == GLFW_PRESS)
			rigidDynamic->addTorque({ 7, 5, 0 });
		
		cam.Update(window->window, deltaseconds);

		auto frameStart = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed_seconds{ frameStart - appStart };
		
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		this->window->Update();

		// do stuff
		light.position = vec3(3*sin(elapsed_seconds.count()), 1, 3*cos(elapsed_seconds.count()));
		light.UpdateShader(&*shader);
		sun.UpdateShader(&*shader);
		light.UpdateShader(&*lightingShader);
		sun.UpdateShader(&*lightingShader);
		
		//cam.position = -vec3(4*sin(-0.25f*elapsed_seconds.count()), -1, 4*cos(-0.25f * elapsed_seconds.count()));
		sphere.transform = translate(light.position) * scale(0.1);
		//flightHelmet.transform = scale(3);
		//helmet.transform = translate(vec3(2,0,0)) * rotationx(3.14 / 2);
		//avocado.transform = translate(vec3(-2,0,0)) * scale(20);
		cube.transform = translate(vec3(dynPos.x, dynPos.y, dynPos.z)) * NewDynRotMat * scale(0.5, 2.0, 0.5);
		armCube.transform = translate(vec3(dynArmPos.x, dynArmPos.y, dynArmPos.z)) * NewDynArmRotMat * scale(0.5, 0.5, 0.5);
		//normalTangentMirrorTest.transform = translate(vec3(0, 0, 1));
		
		mat4 view = cam.GetView();
		mat4 viewProjection = projection * view;
		
		shader->UseProgram();
		//grad.BindTexture(1);
		shader->SetVec3("viewPos", cam.position);

		lightingShader->UseProgram();
		//grad.BindTexture(1);
		lightingShader->SetVec3("viewPos", cam.position);

		//avocado.draw(viewProjection);
		sphere.draw(viewProjection);
		//helmet.draw(viewProjection);
		//flightHelmet.draw(viewProjection);
		cube.draw(viewProjection);
		armCube.draw(viewProjection);
		//normalTangentMirrorTest.draw(viewProjection);

		Quad.draw(viewProjection);

		//grid.Draw((float*)&viewProjection);

		this->window->SwapBuffers();

#ifdef CI_TEST
		// if we're running CI, we want to return and exit the application after one frame
		// break the loop and hopefully exit gracefully
		break;
#endif
	}

	/// ------------------------------------------
	/// [BEGIN] SHUTDOWN PHYSICS
	/// ------------------------------------------

	mPhysics->release();
	mFoundation->release();

	/// ------------------------------------------
	/// [END] SHUTDOWN PHYSICS
	/// ------------------------------------------
}

} // namespace Example