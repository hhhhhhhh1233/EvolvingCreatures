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
#include "RandomUtils.h"

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
static void FindCreatureFiles(std::string path, std::vector<char*>& Entries)
{
	for (int i = 0; i < Entries.size(); i++)
	{
		delete Entries[i];
	}
	Entries.erase(Entries.begin(), Entries.end());

	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		if (entry.path().string().find(".creature") != std::string::npos)
		{
			char* file = new char[entry.path().u8string().size() + 1];
			std::strcpy(file, entry.path().u8string().c_str());
			Entries.push_back(file);
		}
	}
}

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
	light.intensity = 0.0;

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

	physx::PxDefaultCpuDispatcher* Dispatcher = physx::PxDefaultCpuDispatcherCreate(2);

	/// ------------------------------------------
	/// [END] INIT PHYSICS
	/// ------------------------------------------

	GenerationManager* GenMan = new GenerationManager(Physics, Dispatcher, artCube);

	float mAccumulator = 0.0f;
	float mStepSize = 1.0f / 60.0f;

	bool bAttachCam = false;
	int CreatureIndexToDraw = 0;
	bool bDrawBoundingBox = false;

	std::vector<char*> Entries;
	FindCreatureFiles("Creatures", Entries);

	char* SavedCreatureName = new char[30];
	strcpy(SavedCreatureName, "NewCreature");

	this->window->SetUiRender([this, &bAttachCam, GenMan, &CreatureIndexToDraw, &bDrawBoundingBox, &Entries, &SavedCreatureName]()
	{
		bool show = true;
		// create a new window
		ImGui::Begin("Evolving Creatures Options", &show);

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

			ImGui::Checkbox("Draw creature bounding box", &bDrawBoundingBox);

			if (ImGui::Button("Prev"))
			{
				CreatureIndexToDraw--;
				if (CreatureIndexToDraw < 0)
					CreatureIndexToDraw = GenMan->mSortedCreatures.size() - 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("Next"))
			{
				CreatureIndexToDraw++;
				if (CreatureIndexToDraw >= GenMan->mSortedCreatures.size())
					CreatureIndexToDraw = 0;
			}

			ImGui::InputText("Creature Name", SavedCreatureName, 30);
			if (ImGui::Button("Save Creature"))
			{
				SaveCreatureToFile(GenMan->mSortedCreatures[CreatureIndexToDraw].first, "Creatures/" + std::string(SavedCreatureName) + ".creature");
				strcpy(SavedCreatureName, "NewCreature");
			}
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
			ImGui::Columns(2);
			ImGui::Text("Saved Creatures");
			if (ImGui::Button("Refresh"))
			{
				FindCreatureFiles("Creatures", Entries);
			}

			if (Entries.size() > 0)
			{
				static int CurrentItem = 0;
				ImGui::ListBox("", &CurrentItem, Entries.data(), Entries.size(), 5);
				if (ImGui::Button("Load Creature"))
				{
					GenMan->LoadCreature(Entries[CurrentItem]);
				}
			}

			if (GenMan->mLoadedCreatureNames.size() > 0)
			{
				static int CurrentItem = 0;
				ImGui::Text("");
				ImGui::Text("Loaded Creatures");
				ImGui::ListBox(" ", &CurrentItem, GenMan->mLoadedCreatureNames.data(), GenMan->mLoadedCreatureNames.size(), 5);

				if (CurrentItem >= GenMan->mLoadedCreatures.size())
				{
					CurrentItem = GenMan->mLoadedCreatures.size() - 1;
				}

				bool bToRemove = ImGui::Button("Remove");

				ImGui::SameLine();

				if (ImGui::Button("Move to spawn"))
				{
					GenMan->SetLoadedCreaturePosition(CurrentItem, vec3(0, 20, 0));
				}

				ImGui::SameLine();

				ImGui::Checkbox("Active", &GenMan->mLoadedCreatures[CurrentItem]->bActive);

				ImGui::SameLine();

				ImGui::Checkbox("Draw Bounding Boxes", &GenMan->mLoadedCreatures[CurrentItem]->bDrawBoundingBox);

				if (bToRemove)
				{
					GenMan->RemoveLoadedCreature(CurrentItem);
				}
			}

			ImGui::NextColumn();

			ImGui::Text("Evolution Options");

			static bool bUseLoadedCreatures;
			ImGui::Checkbox("Use loaded creatures in population", &bUseLoadedCreatures);

			ImGui::DragInt("Population Size", &NumberOfCreatures, 1, 5, 500);
			ImGui::DragInt("Generation Survivors", &GenerationSurvivors, 1, 5, NumberOfCreatures);
			if (GenerationSurvivors > NumberOfCreatures)
				GenerationSurvivors = NumberOfCreatures;
			ImGui::DragFloat("Mutation Chance", &MutationChance, 0.05, 0, 1, "%.2f");
			ImGui::DragFloat("Mutation Severity", &MutationSeverity, 0.05, 0, 1, "%.2f");

			ImGui::Text("Generation Management");
			ImGui::DragInt("Number of Generations", &NumberOfGenerations, 1, 1, 200);
			ImGui::DragFloat("Evaluation Duration", &EvaluationTime, 1, 0, 120);

			if (ImGui::Button("Start"))
			{
				GenMan->Start(NumberOfGenerations, EvaluationTime, GenerationSurvivors, MutationChance, MutationSeverity, NumberOfCreatures, bUseLoadedCreatures);
			}
			ImGui::Columns(1);
		}
		else
		{
			ImGui::Columns(2);
			ImGui::Text("Statistics");

			ImGui::Text("Number of creatures: %d", NumberOfCreatures);
			ImGui::Text("Survivors per gen: %d", GenerationSurvivors);
			ImGui::Text("Mutation Chance: %.1f%%", MutationChance * 100);
			ImGui::Text("Mutation Severity: %.1f%%", MutationSeverity * 100);

			ImGui::NextColumn();
			ImGui::Text("On Generation: %d/%d", GenMan->mCurrentGeneration, GenMan->mNumberOfGenerations);
			ImGui::Text("Been running for: %.2f/%.2f", GenMan->mCurrentGenerationDuration, GenMan->mGenerationDurationSeconds);

			float CurrentProgress = (((GenMan->mCurrentGeneration * GenMan->mGenerationDurationSeconds) + GenMan->mCurrentGenerationDuration) / (GenMan->mNumberOfGenerations * GenMan->mGenerationDurationSeconds));

			ImGui::Text("Progress: %.2f%%", CurrentProgress * 100);
			ImGui::Columns(1);
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

		GenMan->Update(deltaseconds);

		mAccumulator += deltaseconds;
		if (mAccumulator > mStepSize)
		{
			if (GenMan->mCurrentState != GenerationManagerState::Waiting)
				GenMan->Activate();

			if (GenMan->mCurrentState == GenerationManagerState::Nothing)
				GenMan->ActivateLoadedCreatures();

			GenMan->Simulate(mStepSize);

			mAccumulator -= mStepSize;
		}
		
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
		sun.UpdateShader(&*shader);
		sun.UpdateShader(&*lightingShader);
		
		GenMan->UpdateCreatures(deltaseconds);
		
		mat4 view = cam.GetView();
		mat4 viewProjection = projection * view;

		/// For Shadow Mapping
		mat4 lightView = lookat(vec3(-2, 4, -1), vec3(0, 0, 0), vec3(0, 1, 0));
		mat4 lightSpaceMatrix = lightProjection * lightView;
		
		shader->UseProgram();
		shader->SetVec3("viewPos", cam.mPosition);

		lightingShader->UseProgram();
		lightingShader->SetVec3("viewPos", cam.mPosition);

		/// ----------------------------------------
		/// [BEGIN] MORE SHADOW MAPPING STUFF
		/// ----------------------------------------

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			GenMan->DrawCreatures(viewProjection, simpleDepthShader);
			Quad.draw(lightSpaceMatrix, simpleDepthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		/// ----------------------------------------
		/// [END] MORE SHADOW MAPPING STUFF
		/// ----------------------------------------

		if (GenMan->mCurrentState == GenerationManagerState::Running || GenMan->mCurrentState == GenerationManagerState::Waiting)
		{
			GenMan->DrawCreatures(viewProjection);
		}
		else if (GenMan->mCurrentState == GenerationManagerState::Finished)
		{
			GenMan->DrawFinishedCreatures(viewProjection, CreatureIndexToDraw);
			if (bDrawBoundingBox)
				GenMan->mSortedCreatures[CreatureIndexToDraw].first->DrawBoundingBoxes(viewProjection, vec3(0, 20, 0), cube);
		}

		GenMan->UpdateAndDrawLoadedCreatures(viewProjection, deltaseconds);
		for (auto Thing : GenMan->mLoadedCreatures)
		{
			if (Thing->bDrawBoundingBox)
				Thing->mCreature->DrawBoundingBoxes(viewProjection, vec3(0, 20, 0), cube);
		}

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

	/// Deleting the char* we created for storing the name of the creature
	delete[] SavedCreatureName;
}

} // namespace Example