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

class CreaturePart
{
public:
	physx::PxArticulationLink* mLink;
	physx::PxMaterial* mPhysicsMaterial;
	physx::PxShapeFlags mShapeFlags;
	physx::PxArticulationJointReducedCoordinate* mJoint;
	
	std::vector<CreaturePart*> mChildren;

	GraphicsNode mNode;
	vec3 mScale;

	CreaturePart(physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags) : mPhysicsMaterial(PhysicsMaterial), mShapeFlags(ShapeFlags)
	{

	}

	void AddBoxShape(physx::PxPhysics* Physics, vec3 Scale, GraphicsNode Node)
	{
		physx::PxShape* shape = Physics->createShape(physx::PxBoxGeometry({Scale.x, Scale.y, Scale.z}), &mPhysicsMaterial, 1, true, mShapeFlags);
		mLink->attachShape(*shape);
		shape->release();
		mScale = Scale;
		mNode = Node;
	}

	void AddChild(physx::PxPhysics* Physics, physx::PxArticulationReducedCoordinate* Articulation, physx::PxMaterial* PhysicsMaterial, 
		physx::PxShapeFlags ShapeFlags, vec3 Scale, GraphicsNode Node, vec3 RelativePosition)
	{
		CreaturePart* NewPart = new CreaturePart(PhysicsMaterial, ShapeFlags);
		NewPart->mLink = Articulation->createLink(mLink, physx::PxTransform(physx::PxIdentity));
		NewPart->AddBoxShape(Physics, Scale, Node);

		NewPart->mJoint = NewPart->mLink->getInboundJoint();
		NewPart->mJoint->setParentPose(physx::PxTransform(physx::PxIdentity));
		NewPart->mJoint->setChildPose(physx::PxTransform({RelativePosition.x, RelativePosition.y, RelativePosition.z}));

		NewPart->ConfigureJoint();
		
		mChildren.push_back(NewPart);
	}

	/// TODO: Add options to this for different styled joints
	/// PosDrive should probably be a parameter
	void ConfigureJoint()
	{
		/// Configure the joint type and motion, limited motion
		mJoint->setJointType(physx::PxArticulationJointType::eREVOLUTE);
		mJoint->setMotion(physx::PxArticulationAxis::eTWIST, physx::PxArticulationMotion::eFREE);
		physx::PxArticulationLimit limits;
		limits.low = -physx::PxPiDivFour;
		limits.high = physx::PxPiDivFour;
		mJoint->setLimitParams(physx::PxArticulationAxis::eTWIST, limits);

		/// Add joint drive
		physx::PxArticulationDrive posDrive;
		posDrive.stiffness = 0;
		posDrive.damping = 0;
		posDrive.maxForce = 0;
		posDrive.driveType = physx::PxArticulationDriveType::eFORCE;

		/// Apply and Set targets (note the consistent axis)
		mJoint->setDriveParams(physx::PxArticulationAxis::eTWIST, posDrive);
		mJoint->setDriveVelocity(physx::PxArticulationAxis::eTWIST, 0.0f);
		mJoint->setDriveTarget(physx::PxArticulationAxis::eTWIST, 0);
	}

	void Update()
	{
		physx::PxVec3 Pos = mLink->getGlobalPose().p;
		vec3 Position(Pos.x, Pos.y, Pos.z);

		mat4 RotMat;
		{
			physx::PxQuat dynQuat = mLink->getGlobalPose().q;
			auto xVec = dynQuat.getBasisVector0();
			auto yVec = dynQuat.getBasisVector1();
			auto zVec = dynQuat.getBasisVector2();
			RotMat = mat4(vec4(xVec.x, xVec.y, xVec.z, 0), vec4(yVec.x, yVec.y, yVec.z, 0), vec4(zVec.x, zVec.y, zVec.z, 0), vec4(0, 0, 0, 1));
		}

		mNode.transform = translate(Position) * RotMat * scale(mScale.x, mScale.y, mScale.z);

		for (auto Child : mChildren)
			Child->Update();
	}

	void Draw(mat4 ViewProjection)
	{
		mNode.draw(ViewProjection);

		for (auto Child : mChildren)
			Child->Draw(ViewProjection);
	}
};

class Creature
{
public:
	physx::PxArticulationReducedCoordinate* mArticulation;
	CreaturePart* mRootPart;
	
	Creature(physx::PxPhysics* Physics, physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags, GraphicsNode Node, vec3 Scale)
	{
		mArticulation = Physics->createArticulationReducedCoordinate();
		mArticulation->setArticulationFlag(physx::PxArticulationFlag::eDISABLE_SELF_COLLISION, true);

		//mRootPart = new CreaturePart(this, PhysicsMaterial, ShapeFlags);
		mRootPart = new CreaturePart(PhysicsMaterial, ShapeFlags);
		mRootPart->mLink = mArticulation->createLink(NULL, physx::PxTransform(physx::PxIdentity));

		mRootPart->AddBoxShape(Physics, Scale, Node);
	}

	void AddToScene(physx::PxScene* Scene)
	{
		Scene->addArticulation(*mArticulation);
	}

	void Update()
	{
		mRootPart->Update();
	}

	void Draw(mat4 ViewProjection)
	{
		mRootPart->Draw(ViewProjection);
	}
};


void
ExampleApp::Run()
{
	/// ---------------------------------------- 
	/// [BEGIN] SHADOW MAPPING
	/// ---------------------------------------- 

	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	/// ---------------------------------------- 
	/// [END] SHADOW MAPPING
	/// ---------------------------------------- 

	TextureResource furTexture;
	furTexture.LoadFromFile(FILE_ROOT "images\\fur.jpg");

	TextureResource gridTexture;
	gridTexture.LoadFromFile(FILE_ROOT "images\\Grid.jpg");

	TextureResource defaultTexture;
	defaultTexture.LoadFromFile(FILE_ROOT "images\\default.png");

	std::shared_ptr<ShaderResource> shader = std::make_shared<ShaderResource>();
	shader->LoadShaders(FILE_ROOT "Shaders\\materialShader.vert", FILE_ROOT "Shaders\\materialShader.frag");

	std::shared_ptr<ShaderResource> lightingShader = std::make_shared<ShaderResource>();
	lightingShader->LoadShaders(FILE_ROOT "Shaders\\lightingShader.vert", FILE_ROOT "Shaders\\lightingShader.frag");

	MeshResource sphereMesh;
	sphereMesh.LoadOBJ(FILE_ROOT "objs\\sphere.obj");

	GraphicsNode sphere = GraphicsNode(std::make_shared<MeshResource>(std::move(sphereMesh)), std::make_shared<TextureResource>(gridTexture), shader, mat4(), 32);

	//GraphicsNode helmet = LoadGLTF(FILE_ROOT "glTFs\\DamagedHelmetglTF\\", "DamagedHelmet.gltf", shader);
	//GraphicsNode avocado = LoadGLTF(FILE_ROOT "glTFs\\AvocadoglTF\\", "Avocado.gltf", shader);
	GraphicsNode cube = LoadGLTF(FILE_ROOT "glTFs\\CubeglTF\\", "Cube.gltf", lightingShader, std::make_shared<TextureResource>(gridTexture));
	GraphicsNode armCube = LoadGLTF(FILE_ROOT "glTFs\\CubeglTF\\", "Cube.gltf", lightingShader, std::make_shared<TextureResource>(gridTexture));

	GraphicsNode artCube = LoadGLTF(FILE_ROOT "glTFs\\CubeglTF\\", "Cube.gltf", lightingShader);
	GraphicsNode artArmCube = LoadGLTF(FILE_ROOT "glTFs\\CubeglTF\\", "Cube.gltf", lightingShader);
	//GraphicsNode normalTangentMirrorTest = LoadGLTF(FILE_ROOT "glTFs\\NormalTangentMirrorTestglTF\\", "NormalTangentMirrorTest.gltf", shader);
	GraphicsNode Quad(std::make_shared<MeshResource>(CreateQuad(30, 30)), std::make_shared<TextureResource>(defaultTexture), lightingShader, rotationx(3.14/2), 1);
	
	mat4 projection = perspective(3.14f / 2, window->GetAspectRatio(), 0.1f, 100);
	
	Camera cam;
	cam.position = vec3(0, 1, 3);

	/// ------------------------------------------
	/// [BEGIN] LIGHT SETUP
	/// ------------------------------------------

	PointLightSource light;
	light.position = vec3(0, 0, -2);
	light.color = vec3(1, 1, 1);
	light.intensity = 0.2;

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

	physx::PxRevoluteJoint* joint = physx::PxRevoluteJointCreate(*mPhysics, rigidDynamic, physx::PxTransform(physx::PxIdentity), rigidDynamicArm,physx::PxTransform(physx::PxVec3(1.f, 0.0f, 0.f)));

	//joint->setLimit(physx::PxJointAngularLimitPair(-physx::PxPi / 4, physx::PxPi / 4));
	joint->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eDRIVE_FREESPIN, true);

	/// ----------------------------------------
	/// [BEGIN] ARTICULATIONS
	/// ----------------------------------------

	Creature* NewCreature = new Creature(mPhysics, materialPtr, shapeFlags, artCube, vec3(0.5f, 2.5f, 0.5f));
	NewCreature->mRootPart->AddChild(mPhysics, NewCreature->mArticulation, materialPtr, shapeFlags, vec3(0.5f, 0.5f, 0.5f), artArmCube, vec3(1,0,0));
	NewCreature->mRootPart->AddChild(mPhysics, NewCreature->mArticulation, materialPtr, shapeFlags, vec3(0.5f, 0.5f, 0.5f), artArmCube, vec3(-1,0,0));
	NewCreature->mRootPart->mChildren[1]->AddChild(mPhysics, NewCreature->mArticulation, materialPtr, shapeFlags, vec3(0.5f, 0.5f, 0.5f), artArmCube, vec3(-1, 0, 0));
	NewCreature->mRootPart->mChildren[1]->mChildren[0]->AddChild(mPhysics, NewCreature->mArticulation, materialPtr, shapeFlags, vec3(0.5f, 0.5f, 0.5f), artArmCube, vec3(-1, 0, 0));

	NewCreature->AddToScene(mScene);

	/// ------------------------------------------
	/// [END] ARTICULATIONS
	/// ------------------------------------------

	/// ------------------------------------------
	/// [END] CREATE ACTORS
	/// ------------------------------------------

	float mAccumulator = 0.0f;
	float mStepSize = 1.0f / 60.0f;

	const auto [ SCR_WIDTH, SCR_HEIGHT ] = window->GetWidthHeight();

	while (this->window->IsOpen())
	{
		auto end = std::chrono::high_resolution_clock::now();
		float deltaseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0f;
		start = std::chrono::high_resolution_clock::now();

		mAccumulator += deltaseconds;
		if (mAccumulator > mStepSize)
		{
			mScene->simulate(mStepSize);
			mScene->fetchResults(true);

			mAccumulator -= mStepSize;
		}

		/// ---------------------------------------- 
		/// [BEGIN] GET CUBE AND CUBE ARM POS AND ROT
		/// ---------------------------------------- 

		/// I couldn't figure out a better way to get the rotation out of PhysX
		physx::PxVec3 dynPos = rigidDynamic->getGlobalPose().p;
		mat4 NewDynRotMat;
		{
			physx::PxQuat dynQuat = rigidDynamic->getGlobalPose().q;
			auto xVec = dynQuat.getBasisVector0();
			auto yVec = dynQuat.getBasisVector1();
			auto zVec = dynQuat.getBasisVector2();
			NewDynRotMat = mat4(vec4(xVec.x, xVec.y, xVec.z, 0), vec4(yVec.x, yVec.y, yVec.z, 0), vec4(zVec.x, zVec.y, zVec.z, 0), vec4(0, 0, 0, 1));
		}

		/// I couldn't figure out a better way to get the rotation out of PhysX
		physx::PxVec3 dynArmPos = rigidDynamicArm->getGlobalPose().p;
		mat4 NewDynArmRotMat;
		{
			physx::PxQuat dynArmQuat = rigidDynamicArm->getGlobalPose().q;
			auto xVec = dynArmQuat.getBasisVector0();
			auto yVec = dynArmQuat.getBasisVector1();
			auto zVec = dynArmQuat.getBasisVector2();
			NewDynArmRotMat = mat4(vec4(xVec.x, xVec.y, xVec.z, 0), vec4(yVec.x, yVec.y, yVec.z, 0), vec4(zVec.x, zVec.y, zVec.z, 0), vec4(0, 0, 0, 1));
		}

		/// ---------------------------------------- 
		/// [END] GET CUBE AND CUBE ARM POS AND ROT
		/// ---------------------------------------- 

		if (glfwGetKey(window->window, GLFW_KEY_R) == GLFW_PRESS)
			rigidDynamic->addForce({ 0, 25, 0 });

		if (glfwGetKey(window->window, GLFW_KEY_T) == GLFW_PRESS)
			rigidDynamic->addTorque({ 7, 5, 0 });

		if (glfwGetKey(window->window, GLFW_KEY_Y) == GLFW_PRESS)
			rigidDynamic->addTorque({ -7, -5, 0 });

		if (glfwGetKey(window->window, GLFW_KEY_F) == GLFW_PRESS)
			NewCreature->mRootPart->mLink->addForce({ 0, 45, 0 });

		if (glfwGetKey(window->window, GLFW_KEY_G) == GLFW_PRESS)
			NewCreature->mRootPart->mLink->addTorque({ 7, 5, 0 });

		if (glfwGetKey(window->window, GLFW_KEY_H) == GLFW_PRESS)
			NewCreature->mRootPart->mLink->addTorque({ -7, -5, 0 });

		
		cam.Update(window->window, deltaseconds);

		auto frameStart = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed_seconds{ frameStart - appStart };
		
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		this->window->Update();

		// do stuff
		float lightRadius = 8, lightHeight = 4, lightSpeedMultiplier = 0.7;
		light.position = vec3(lightRadius * sin(elapsed_seconds.count() * lightSpeedMultiplier), lightHeight, lightRadius * cos(elapsed_seconds.count() * lightSpeedMultiplier));
		light.UpdateShader(&*shader);
		sun.UpdateShader(&*shader);
		light.UpdateShader(&*lightingShader);
		sun.UpdateShader(&*lightingShader);
		
		sphere.transform = translate(light.position) * scale(0.1);
		cube.transform = translate(vec3(dynPos.x, dynPos.y, dynPos.z)) * NewDynRotMat * scale(0.5, 2.0, 0.5);
		armCube.transform = translate(vec3(dynArmPos.x, dynArmPos.y, dynArmPos.z)) * NewDynArmRotMat * scale(0.5, 0.5, 0.5);
		//artCube.transform = translate(vec3(artDynPos.x, artDynPos.y, artDynPos.z)) * artNewDynRotMat * scale(0.5, 2.0, 0.5);
		//artArmCube.transform = translate(vec3(artDynArmPos.x, artDynArmPos.y, artDynArmPos.z)) * artNewDynArmRotMat * scale(0.5, 0.5, 0.5);

		NewCreature->Update();
		
		mat4 view = cam.GetView();
		mat4 viewProjection = projection * view;
		
		shader->UseProgram();
		shader->SetVec3("viewPos", cam.position);

		lightingShader->UseProgram();
		lightingShader->SetVec3("viewPos", cam.position);

		sphere.draw(viewProjection);

		/// ----------------------------------------
		/// [BEGIN] MORE SHADOW MAPPING STUFF
		/// ----------------------------------------

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			cube.draw(viewProjection);
			armCube.draw(viewProjection);
			artCube.draw(viewProjection);
			artArmCube.draw(viewProjection);
			Quad.draw(viewProjection);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		/// ----------------------------------------
		/// [END] MORE SHADOW MAPPING STUFF
		/// ----------------------------------------

		sphere.draw(viewProjection);
		cube.draw(viewProjection);
		armCube.draw(viewProjection);

		NewCreature->Draw(viewProjection);
		//NewCreature->mRootPart->Draw(viewProjection);
		//NewCreature->mRootPart->mChildren[0]->Draw(viewProjection);

		Quad.draw(viewProjection);

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