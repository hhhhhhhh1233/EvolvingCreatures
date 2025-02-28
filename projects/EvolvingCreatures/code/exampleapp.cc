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
#include "GenerationManager.h"

#include "imgui.h"

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
	///// Temporarily seed the random so I don't have to worry about that mucking up my testing
	//srand(3);
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

	TextureResource gridTexture;
	gridTexture.LoadFromFile("Assets\\images\\Grid.jpg");

	TextureResource gridArtTexture;
	gridArtTexture.LoadFromFile("Assets\\images\\Grid2.png");

	TextureResource defaultTexture;
	defaultTexture.LoadFromFile("Assets\\images\\default.png");

	std::shared_ptr<ShaderResource> shader = std::make_shared<ShaderResource>();
	shader->LoadShaders("Assets\\Shaders\\materialShader.vert", "Assets\\Shaders\\materialShader.frag");

	std::shared_ptr<ShaderResource> lightingShader = std::make_shared<ShaderResource>();
	lightingShader->LoadShaders("Assets\\Shaders\\lightingShader.vert", "Assets\\Shaders\\lightingShader.frag");

	std::shared_ptr<ShaderResource> simpleDepthShader = std::make_shared<ShaderResource>();
	simpleDepthShader->LoadShaders("Assets\\Shaders\\simpleDepthShader.vert", "Assets\\Shaders\\simpleDepthShader.frag");

	MeshResource sphereMesh;
	sphereMesh.LoadOBJ("Assets\\objs\\sphere.obj");

	GraphicsNode sphere = GraphicsNode(std::make_shared<MeshResource>(std::move(sphereMesh)), std::make_shared<TextureResource>(gridTexture), shader, mat4(), 32);

	GraphicsNode cube = LoadGLTF("Assets\\glTFs\\CubeglTF\\", "Cube.gltf", lightingShader, std::make_shared<TextureResource>(gridTexture));
	GraphicsNode armCube = LoadGLTF("Assets\\glTFs\\CubeglTF\\", "Cube.gltf", lightingShader, std::make_shared<TextureResource>(gridTexture));

	GraphicsNode artCube = LoadGLTF("Assets\\glTFs\\CubeglTF\\", "Cube.gltf", lightingShader, std::make_shared<TextureResource>(gridArtTexture));
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

	physx::PxFoundation* Foundation = NULL;
	Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
	if (!Foundation)
	{
		std::cout << "Completely broken!\n";
		return;
	}

	physx::PxPhysics* Physics = NULL;
	physx::PxPvd* mPvd = NULL;

	bool bRecordMemoryAllocation = true;
	mPvd = physx::PxCreatePvd(*Foundation);
	physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	mPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

	Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *Foundation, physx::PxTolerancesScale(), bRecordMemoryAllocation, mPvd);
	if (!Physics)
	{
		std::cout << "Completely broken!\n";
		return;
	}

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

	physx::PxScene* mScene = Physics->createScene(SceneDesc);
	mScene->setFlag(physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS, true);

	/// ------------------------------------------
	/// [END] INIT PHYSICS
	/// ------------------------------------------

	/// ------------------------------------------
	/// [BEGIN] CREATE ACTORS
	/// ------------------------------------------

	physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE;
	physx::PxMaterial* materialPtr = Physics->createMaterial(0.5f, 0.5f, 0.1f);

	physx::PxRigidStatic* PlaneCollision = Physics->createRigidStatic(physx::PxTransformFromPlaneEquation(physx::PxPlane(physx::PxVec3(0.f, 1.f, 0.f), 0.f)));
	{
		physx::PxShape* shape = Physics->createShape(physx::PxPlaneGeometry(), &materialPtr, 1, true, shapeFlags);
		PlaneCollision->attachShape(*shape);
		shape->release();
	}

	mScene->addActor(*PlaneCollision);

	auto CreatureStart = std::chrono::high_resolution_clock::now();

	vec3 RootScale(1.5f, 2.5f, 1.5f);
	vec3 ChildScale(1.5, 3.5, 1.5);

	Creature* NewCreature = new Creature(Physics, materialPtr, shapeFlags, artCube, RootScale);
	NewCreature->SetPosition(vec3(0, 10, 0));

	/// The positions are different, the way you get that is by taking parent scale plus child scale in the axis want to move it along, 4 = 1.5 + 2.5, and 5 = 2.5 + 2.5
	vec3 FirstChildPosition(0, RootScale.y + ChildScale.y, 0);
	vec3 ChildPosition(0, ChildScale.y + ChildScale.y, 0);

	/// The way you get the joint is by taking just the parent's scale in that axis
	vec3 FirstChildJointPosition(0, RootScale.y, 0);
	vec3 ChildJointPosition(0, ChildScale.y, 0);

	NewCreature->AddRandomPart(Physics, materialPtr, shapeFlags, artCube);
	NewCreature->AddRandomPart(Physics, materialPtr, shapeFlags, artCube);
	NewCreature->AddRandomPart(Physics, materialPtr, shapeFlags, artCube);
	NewCreature->AddRandomPart(Physics, materialPtr, shapeFlags, artCube);
	NewCreature->AddRandomPart(Physics, materialPtr, shapeFlags, artCube);
	NewCreature->AddRandomPart(Physics, materialPtr, shapeFlags, artCube);
	NewCreature->AddRandomPart(Physics, materialPtr, shapeFlags, artCube);
	NewCreature->AddRandomPart(Physics, materialPtr, shapeFlags, artCube);

	//NewCreature->EnableGravity(false);

	//CreaturePart* a = NewCreature->mRootPart->AddChild(mPhysics, NewCreature->mArticulation, materialPtr, shapeFlags, artCube, ChildScale, FirstChildPosition, FirstChildJointPosition, 20, 3);
	//CreaturePart* b = a->AddChild(mPhysics, NewCreature->mArticulation, materialPtr, shapeFlags, artCube, ChildScale, ChildPosition, ChildJointPosition, 20, 3);
	//CreaturePart* c = b->AddChild(mPhysics, NewCreature->mArticulation, materialPtr, shapeFlags, artCube, ChildScale, ChildPosition, ChildJointPosition, 20, 3);
	//CreaturePart* d = c->AddChild(mPhysics, NewCreature->mArticulation, materialPtr, shapeFlags, artCube, ChildScale, ChildPosition, ChildJointPosition, 20, 3);
	//CreaturePart* e = d->AddChild(mPhysics, NewCreature->mArticulation, materialPtr, shapeFlags, artCube, ChildScale, ChildPosition, ChildJointPosition, 20, 3);

	NewCreature->AddToScene(mScene);

	Creature* MutatedCreature = NewCreature->GetMutatedCreature(Physics, 1.5, 0.5);
	MutatedCreature->SetPosition(vec3(5, 10, 0));
	//MutatedCreature->AddToScene(mScene);

	/// ------------------------------------------
	/// [END] CREATE ACTORS
	/// ------------------------------------------

	GenerationManager* GenMan = new GenerationManager(Physics, Dispatcher, artCube);

	float mAccumulator = 0.0f;
	float mStepSize = 1.0f / 60.0f;

	bool bAttachCam = false;
	bool bResetCreature = false;
	bool bActiveCreature = true;
	bool bSimulateGravity = true;
	int BodyPartsNum = 2;
	int CreatureIndexToDraw = 0;
	this->window->SetUiRender([this, &bAttachCam, &bResetCreature, &BodyPartsNum, &bActiveCreature, &bSimulateGravity, &NewCreature, GenMan, &CreatureIndexToDraw]()
	{
		bool show = true;
		// create a new window
		ImGui::Begin("Evolving Creatures Options", &show, ImGuiWindowFlags_NoSavedSettings);

		static bool bGenerationRunning = false;

		static int NumberOfCreatures = 50;
		static int GenerationSurvivors = 15;
		static float MutationChance = 0.3;
		static float MutationSeverity = 0.15;

		static int NumberOfGenerations = 5;
		static float EvaluationTime = 20;

		/// Debug Feature
		char* StateNames[] = { {"Nothing"} , {"Running"}, {"Finished"}, {"Waiting"}};
		ImGui::Text("Current state: %s", StateNames[GenMan->mCurrentState]);

		if (GenMan->mCurrentState == GenerationManagerState::Finished)
		{
			ImGui::Text("Evolution Finished");

			ImGui::DragInt("Which Creature to Draw", &CreatureIndexToDraw, 1, 0, GenMan->mSortedCreatures.size() - 1);
			ImGui::Text("Drawing creature %d/%d", CreatureIndexToDraw, GenMan->mSortedCreatures.size() - 1);
			ImGui::Text("Creature Stats");
			ImGui::Text("Creature Fitness: %f", GenMan->mSortedCreatures[CreatureIndexToDraw].second);

			ImGui::Checkbox("Follow creature", &bAttachCam);


			if (ImGui::Button("Finish"))
			{
				GenMan->mCurrentState = GenerationManagerState::Nothing;
			}
		}
		else if (GenMan->mCurrentState == GenerationManagerState::Nothing)
		{
			ImGui::Text("Evolution Options");
			ImGui::DragInt("How many creatures", &NumberOfCreatures, 1, 5, 500);
			ImGui::DragInt("How many to keep per gen", &GenerationSurvivors, 1, 5, NumberOfCreatures);
			if (GenerationSurvivors > NumberOfCreatures)
				GenerationSurvivors = NumberOfCreatures;
			ImGui::DragFloat("Mutation Chance", &MutationChance, 0.05, 0, 1, "%.2f");
			ImGui::DragFloat("Mutation Severity", &MutationSeverity, 0.05, 0, 1, "%.2f");

			ImGui::Text("Generation Management");
			ImGui::DragInt("Number of Generations", &NumberOfGenerations, 1, 1, 50);
			ImGui::DragFloat("Evaluation Duration", &EvaluationTime, 1, 0, 120);

			if (ImGui::Button("Start"))
			{
				GenMan->Start(NumberOfGenerations, EvaluationTime, GenerationSurvivors, MutationChance, MutationSeverity);
				GenMan->GenerateCreatures(NumberOfCreatures);
			}
		}
		else
		{
			ImGui::Text("Statistics");

			ImGui::Text("Number of creatures: %d", NumberOfCreatures);
			ImGui::Text("Survivors per gen: %d", GenerationSurvivors);
			ImGui::Text("Mutation Chance: %.1f%%", MutationChance * 100);
			ImGui::Text("Mutation Severity: %.1f%%", MutationSeverity * 100);

			ImGui::Text("\nOn Generation: %d/%d", GenMan->mCurrentGeneration, GenMan->mNumberOfGenerations);
			ImGui::Text("Been running for: %.2f/%.2f", GenMan->mCurrentGenerationDuration, GenMan->mGenerationDurationSeconds);

			float CurrentProgress = (((GenMan->mCurrentGeneration * GenMan->mGenerationDurationSeconds) + GenMan->mCurrentGenerationDuration) / (GenMan->mNumberOfGenerations * GenMan->mGenerationDurationSeconds));

			ImGui::Text("Progress: %.2f%%", CurrentProgress * 100);
		}


		// close window
		ImGui::End();
	});

	//float TestAccumulator = 0;
	//float TestBoxUpdateTime = 5;
	//BoundingBox TestBox;
	//CreaturePart* ParentToTestBox = nullptr;

	const auto [ SCR_WIDTH, SCR_HEIGHT ] = window->GetWidthHeight();

	while (this->window->IsOpen())
	{
		auto end = std::chrono::high_resolution_clock::now();
		float deltaseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0f;
		float timesincestart = std::chrono::duration_cast<std::chrono::milliseconds>(end - appStart).count() / 1000.0f;
		start = std::chrono::high_resolution_clock::now();

		//if (bResetCreature)
		//{

		//	std::cout << "\n----------------------------------------\nCreating new Creature!\n";
		//	CreatureStart = std::chrono::high_resolution_clock::now();
		//	NewCreature->RemoveFromScene(mScene);
		//	delete NewCreature;

		//	NewCreature = new Creature(mPhysics, materialPtr, shapeFlags, artCube, vec3(1.5f, 1.5f, 1.5f));
		//	NewCreature->SetPosition(vec3(0, 10, 0));

		//	for (int i = 0; i < BodyPartsNum; i++)
		//		NewCreature->AddRandomPart(mPhysics, materialPtr, shapeFlags, artCube);

		//	NewCreature->AddToScene(mScene);

		//	NewCreature->EnableGravity(bSimulateGravity);


		//	MutatedCreature->RemoveFromScene(mScene);
		//	delete MutatedCreature;
		//	MutatedCreature = NewCreature->GetMutatedCreature(mPhysics, 0.5, 0.3);
		//	MutatedCreature->SetPosition(vec3(10, 10, 0));
		//	MutatedCreature->AddToScene(mScene);

		//	bResetCreature = false;
		//	std::cout << "\n----------------------------------------";
		//}

		GenMan->Update(deltaseconds);

		mAccumulator += deltaseconds;
		if (mAccumulator > mStepSize)
		{
			if (bActiveCreature)
			{
				//NewCreature->Activate(timesincestart);
				//MutatedCreature->Activate(timesincestart);
				if (GenMan->mCurrentState == GenerationManagerState::Running)
					GenMan->Activate(GenMan->mCurrentGenerationDuration);
				else if (GenMan->mCurrentState != GenerationManagerState::Waiting)
					GenMan->Activate(timesincestart);
			}

			mScene->simulate(mStepSize);
			mScene->fetchResults(true);

			GenMan->Simulate(mStepSize);

			mAccumulator -= mStepSize;
		}
		physx::PxVec3 CreatureVel = NewCreature->mRootPart->mLink->getLinearVelocity();
		//std::cout << CreatureVel.x << ", " << CreatureVel.y << ", " << CreatureVel.z << "\n";
		vec3 CreatureHorizontalVel = vec3(CreatureVel.x, 0, CreatureVel.z);
		//std::cout << "Creature Velocity: " << length(CreatureHorizontalVel) << "\n";

		if (glfwGetKey(window->window, GLFW_KEY_F) == GLFW_PRESS)
			NewCreature->mRootPart->mLink->addForce({ 0, 45, 0 });
		
		if (glfwGetKey(window->window, GLFW_KEY_G) == GLFW_PRESS)
			NewCreature->mRootPart->mLink->addTorque({ 50, 0, 0 });
		
		if (GenMan->mCurrentState == GenerationManagerState::Finished && bAttachCam)
		{
			auto PV = GenMan->mSortedCreatures[CreatureIndexToDraw].first->mRootPart->mLink->getGlobalPose().p;
			vec3 v(PV.x, PV.y, PV.z);
			cam.mTarget = v;

			cam.mPosition = cam.mTarget + vec3(10, 10, 0);
		}
		else
			cam.UpdateInput(window->window, deltaseconds);

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
		MutatedCreature->Update();
		GenMan->UpdateCreatures();
		
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

		//NewCreature->Draw(viewProjection);

		//TestAccumulator += deltaseconds;
		//if (TestAccumulator > TestBoxUpdateTime)
		//{
		//	TestAccumulator -= TestBoxUpdateTime;
		//	std::pair<BoundingBox, CreaturePart*> Temp  = NewCreature->GetRandomShape();
		//	TestBox = Temp.first;
		//	ParentToTestBox = Temp.second;
		//}
		//if (NewCreature->IsColliding(TestBox, ParentToTestBox))
		//{
		//	artCube.transform = translate(vec3(0, 20, 0) + TestBox.GetPosition()) * scale(TestBox.GetScale());
		//	artCube.draw(viewProjection);
		//}
		//else
		//{
		//	cube.transform = translate(vec3(0, 20, 0) + TestBox.GetPosition()) * scale(TestBox.GetScale());
		//	cube.draw(viewProjection);
		//}

		//MutatedCreature->Draw(viewProjection);
		//NewCreature->DrawBoundingBoxes(viewProjection, vec3(0, 20, 0), cube);
		if (GenMan->mCurrentState == GenerationManagerState::Running || GenMan->mCurrentState == GenerationManagerState::Waiting)
			GenMan->DrawCreatures(viewProjection);
		else if (GenMan->mCurrentState == GenerationManagerState::Finished)
			GenMan->DrawFinishedCreatures(viewProjection, CreatureIndexToDraw);

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

	Physics->release();
	Foundation->release();

	/// ------------------------------------------
	/// [END] SHUTDOWN PHYSICS
	/// ------------------------------------------
}

} // namespace Example