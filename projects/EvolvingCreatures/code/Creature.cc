#include "Creature.h"

Creature::Creature(physx::PxPhysics* Physics, physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags, GraphicsNode Node, vec3 Scale)
{
	mArticulation = Physics->createArticulationReducedCoordinate();
	mArticulation->setArticulationFlag(physx::PxArticulationFlag::eDISABLE_SELF_COLLISION, true);

	//mRootPart = new CreaturePart(this, PhysicsMaterial, ShapeFlags);
	mRootPart = new CreaturePart(PhysicsMaterial, ShapeFlags);
	mRootPart->mLink = mArticulation->createLink(NULL, physx::PxTransform(physx::PxIdentity));

	mRootPart->AddBoxShape(Physics, Scale, Node);
}

CreaturePart* Creature::GetChildlessPart() const
{
	CreaturePart* CurrentPart = mRootPart;

	while (CurrentPart->mChildren.size() > 0)
		CurrentPart = CurrentPart->mChildren[0];

	return CurrentPart;
}

CreaturePart* Creature::GetRandomPart()
{
	std::vector<CreaturePart*> Parts = GetAllParts();
	return Parts[rand() % Parts.size()];
}

std::vector<CreaturePart*> Creature::GetAllParts()
{
	return GetAllPartsFrom(mRootPart);
}

std::vector<CreaturePart*> Creature::GetAllPartsFrom(CreaturePart* Part)
{
	std::vector<CreaturePart*> Parts;

	if (Part->mChildren.size() == 0)
	{
		return { Part };
	}

	for (auto iPart : Part->mChildren)
	{
		std::vector<CreaturePart*> Children = GetAllPartsFrom(iPart);

		for (auto ChildPart : Children)
			Parts.push_back(ChildPart);
	}

	Parts.push_back(Part);

	return Parts;
}

void Creature::AddRandomPart(physx::PxPhysics* Physics, physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags, GraphicsNode Node)
{
	//CreaturePart* CurrentPart = mRootPart;

	//const int ODDS_TO_ADD_PART = 20;

	CreaturePart* ParentPart = GetRandomPart();

	int RANDOM_MAX = 3;
	float RandomScaleX = ((rand() % (RANDOM_MAX * 100)) / 100.0f) + 0.1;
	float RandomScaleY = ((rand() % (RANDOM_MAX * 100)) / 100.0f) + 0.1;
	float RandomScaleZ = ((rand() % (RANDOM_MAX * 100)) / 100.0f) + 0.1;
	vec3 RandomScale = vec3(RandomScaleX, RandomScaleY, RandomScaleZ);

	float X = ((rand() % (RANDOM_MAX * 10)) / 10.0f);
	float Y = ((rand() % (RANDOM_MAX * 10)) / 10.0f);
	float Z = (RandomScale.z + ParentPart->mScale.z) * (((rand() % 2) * 2) - 1);

	std::cout << "Z: " << Z << "\n";

	vec3 RandomRelativePosition = vec3(X, Y, Z);

	ParentPart->AddChild(Physics, mArticulation, PhysicsMaterial, ShapeFlags, RandomScale, Node, RandomRelativePosition, vec3(0, 0, 0));

	//if (rand() % 100 < ODDS_TO_ADD_PART)
	//	std::cout << "Lucky! You're in the lucky 20%\n";
	//else
	//	std::cout << "Unlucky... You're in the unlucky 80%\n";
}

void Creature::AddToScene(physx::PxScene* Scene)
{
	Scene->addArticulation(*mArticulation);
}

void Creature::Update()
{
	mRootPart->Update();
}

void Creature::Draw(mat4 ViewProjection)
{
	mRootPart->Draw(ViewProjection);
}
