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

#include "Creature.h"

#include "imgui.h"

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
	srand(time(NULL));
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

	TextureResource gridArtTexture;
	gridArtTexture.LoadFromFile(FILE_ROOT "images\\Grid2.png");

	TextureResource defaultTexture;
	defaultTexture.LoadFromFile(FILE_ROOT "images\\default.png");

	std::shared_ptr<ShaderResource> shader = std::make_shared<ShaderResource>();
	shader->LoadShaders(FILE_ROOT "Shaders\\materialShader.vert", FILE_ROOT "Shaders\\materialShader.frag");

	std::shared_ptr<ShaderResource> lightingShader = std::make_shared<ShaderResource>();
	lightingShader->LoadShaders(FILE_ROOT "Shaders\\lightingShader.vert", FILE_ROOT "Shaders\\lightingShader.frag");

	std::shared_ptr<ShaderResource> simpleDepthShader = std::make_shared<ShaderResource>();
	simpleDepthShader->LoadShaders(FILE_ROOT "Shaders\\simpleDepthShader.vert", FILE_ROOT "Shaders\\simpleDepthShader.frag");

	MeshResource sphereMesh;
	sphereMesh.LoadOBJ(FILE_ROOT "objs\\sphere.obj");

	GraphicsNode sphere = GraphicsNode(std::make_shared<MeshResource>(std::move(sphereMesh)), std::make_shared<TextureResource>(gridTexture), shader, mat4(), 32);

	GraphicsNode cube = LoadGLTF(FILE_ROOT "glTFs\\CubeglTF\\", "Cube.gltf", lightingShader, std::make_shared<TextureResource>(gridTexture));
	GraphicsNode armCube = LoadGLTF(FILE_ROOT "glTFs\\CubeglTF\\", "Cube.gltf", lightingShader, std::make_shared<TextureResource>(gridTexture));

	GraphicsNode artCube = LoadGLTF(FILE_ROOT "glTFs\\CubeglTF\\", "Cube.gltf", lightingShader, std::make_shared<TextureResource>(gridArtTexture));
	GraphicsNode Quad(std::make_shared<MeshResource>(CreateQuad(300, 300, 50)), std::make_shared<TextureResource>(defaultTexture), lightingShader, rotationx(3.14/2), 1);
	
	mat4 projection = perspective(3.14f / 2, window->GetAspectRatio(), 0.1f, 1000);
	mat4 lightProjection = ortho(-10, 10, -10, 10, 0.1f, 1000);
	
	Camera cam;
	cam.mPosition = vec3(0, 3, 8);

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
	physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
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

	physx::PxRigidStatic* PlaneCollision = mPhysics->createRigidStatic(physx::PxTransformFromPlaneEquation(physx::PxPlane(physx::PxVec3(0.f, 1.f, 0.f), 0.f)));
	{
		physx::PxShape* shape = mPhysics->createShape(physx::PxPlaneGeometry(), &materialPtr, 1, true, shapeFlags);
		PlaneCollision->attachShape(*shape);
		shape->release();
	}

	mScene->addActor(*PlaneCollision);

	auto CreatureStart = std::chrono::high_resolution_clock::now();
	Creature* NewCreature = new Creature(mPhysics, materialPtr, shapeFlags, artCube, vec3(1.5f, 1.5f, 1.5f));
	NewCreature->SetPosition(vec3(0, 10, 0));
	CreaturePart* a = NewCreature->mRootPart->AddChild(mPhysics, NewCreature->mArticulation, materialPtr, shapeFlags, artCube, vec3(1.5, 2.5, 1.5), vec3(0, 4, 0), vec3(0, 1.5, 0));
	CreaturePart* b = a->AddChild(mPhysics, NewCreature->mArticulation, materialPtr, shapeFlags, artCube, vec3(1.5, 2.5, 1.5), vec3(0, 5, 0), vec3(0, 2.5, 0));
	CreaturePart* c = b->AddChild(mPhysics, NewCreature->mArticulation, materialPtr, shapeFlags, artCube, vec3(1.5, 2.5, 1.5), vec3(0, 5, 0), vec3(0, 2.5, 0));
	CreaturePart* d = c->AddChild(mPhysics, NewCreature->mArticulation, materialPtr, shapeFlags, artCube, vec3(1.5, 2.5, 1.5), vec3(0, 5, 0), vec3(0, 2.5, 0));
	CreaturePart* e = d->AddChild(mPhysics, NewCreature->mArticulation, materialPtr, shapeFlags, artCube, vec3(1.5, 2.5, 1.5), vec3(0, 5, 0), vec3(0, 2.5, 0));
	/// WHETHER THE LAST PARAMETER IS 1.5 ON X OR 0 IT MAKES NO DIFFERENCE
	/// JUSTIFY WHY AND WHAT IS HAPPENING
	/// THAT WILL ILLUMINATE HOW IT WORKS
	/// I WAS WRONG; IT IS DIFFERENT
	/// THE ATTACHMENT CHANGES FROM BEING EITHER AT THE EDGE OR IN THE CENTER

	//NewCreature->AddRandomPart(mPhysics, materialPtr, shapeFlags, artCube);
	//NewCreature->AddRandomPart(mPhysics, materialPtr, shapeFlags, artCube);
	NewCreature->AddToScene(mScene);

	/// ------------------------------------------
	/// [END] CREATE ACTORS
	/// ------------------------------------------

	float mAccumulator = 0.0f;
	float mStepSize = 1.0f / 60.0f;

	bool bAttachCam = false;
	bool bResetCreature = false;
	bool bActiveCreature = true;
	bool bSimulateGravity = true;
	int BodyPartsNum = 2;
	this->window->SetUiRender([this, &bAttachCam, &bResetCreature, &BodyPartsNum, &bActiveCreature, &bSimulateGravity, &NewCreature]()
	{
		bool show = true;
		// create a new window
		ImGui::Begin("Evolving Creatures Options", &show, ImGuiWindowFlags_NoSavedSettings);

		ImGui::Checkbox("Attach Camera to Creature", &bAttachCam);

		ImGui::Checkbox("Simulate Creature", &bActiveCreature);

		if (ImGui::Button("Toggle Gravity"))
		{
			NewCreature->EnableGravity(!bSimulateGravity);
			bSimulateGravity = !bSimulateGravity;
		}

		ImGui::InputInt("Number of limbs", &BodyPartsNum);
		BodyPartsNum = BodyPartsNum < 0 ? 0 : BodyPartsNum;

		if (ImGui::Button("Regenerate Creature"))
		{
			bResetCreature = true;
		}

		// close window
		ImGui::End();
	});

	const auto [ SCR_WIDTH, SCR_HEIGHT ] = window->GetWidthHeight();

	while (this->window->IsOpen())
	{
		auto end = std::chrono::high_resolution_clock::now();
		float deltaseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0f;
		float timesincestart = std::chrono::duration_cast<std::chrono::milliseconds>(end - appStart).count() / 1000.0f;
		start = std::chrono::high_resolution_clock::now();

		if (bResetCreature)
		{

			CreatureStart = std::chrono::high_resolution_clock::now();
			/// This would probably be good to do but I can't do it while the simulation is running apparently so I don't know how to tweak it
			NewCreature->RemoveFromScene(mScene);
			delete NewCreature;

			NewCreature = new Creature(mPhysics, materialPtr, shapeFlags, artCube, vec3(1.5f, 1.5f, 1.5f));
			NewCreature->SetPosition(vec3(0, 10, 0));
			for (int i = 0; i < BodyPartsNum; i++)
			NewCreature->AddRandomPart(mPhysics, materialPtr, shapeFlags, artCube);
			//NewCreature->AddRandomPart(mPhysics, materialPtr, shapeFlags, artCube);
			NewCreature->AddToScene(mScene);

			NewCreature->EnableGravity(bSimulateGravity);

			bResetCreature = false;
		}

		mAccumulator += deltaseconds;
		if (mAccumulator > mStepSize)
		{
			float MaxVel = 10;
			float OscillationSpeed = 2;

			if (bActiveCreature)
			NewCreature->Activate(MaxVel * sin(OscillationSpeed * timesincestart));

			mScene->simulate(mStepSize);
			mScene->fetchResults(true);

			mAccumulator -= mStepSize;
		}
		physx::PxVec3 CreatureVel = NewCreature->mRootPart->mLink->getLinearVelocity();
		//std::cout << CreatureVel.x << ", " << CreatureVel.y << ", " << CreatureVel.z << "\n";
		vec3 CreatureHorizontalVel = vec3(CreatureVel.x, 0, CreatureVel.z);
		std::cout << "Creature Velocity: " << length(CreatureHorizontalVel) << "\n";

		if (glfwGetKey(window->window, GLFW_KEY_F) == GLFW_PRESS)
			NewCreature->mRootPart->mLink->addForce({ 0, 45, 0 });
		
		if (glfwGetKey(window->window, GLFW_KEY_G) == GLFW_PRESS)
			NewCreature->mRootPart->mLink->addTorque({ 50, 0, 0 });
		
		if (!bAttachCam)
			cam.UpdateInput(window->window, deltaseconds);
		else
		{
			auto PV = NewCreature->mRootPart->mLink->getGlobalPose().p;
			vec3 v(PV.x, PV.y, PV.z);
			cam.mTarget = v;

			cam.mPosition = cam.mTarget + vec3(5, 5, 0);
		}

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

		NewCreature->Update();
		
		mat4 view = cam.GetView();
		mat4 viewProjection = projection * view;

		/// For Shadow Mapping
		mat4 lightView = lookat(vec3(-2, 4, -1), vec3(0, 0, 0), vec3(0, 1, 0));
		mat4 lightSpaceMatrix = lightProjection * lightView;
		
		shader->UseProgram();
		shader->SetVec3("viewPos", cam.mPosition);

		lightingShader->UseProgram();
		lightingShader->SetVec3("viewPos", cam.mPosition);

		sphere.draw(viewProjection);

		/// ----------------------------------------
		/// [BEGIN] MORE SHADOW MAPPING STUFF
		/// ----------------------------------------

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			NewCreature->Draw(lightSpaceMatrix);
			Quad.draw(lightSpaceMatrix);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		/// ----------------------------------------
		/// [END] MORE SHADOW MAPPING STUFF
		/// ----------------------------------------

		sphere.draw(viewProjection);

		NewCreature->Draw(viewProjection);

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