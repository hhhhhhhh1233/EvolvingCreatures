#include "Creature.h"

Creature::Creature(physx::PxPhysics* Physics, physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags, GraphicsNode Node, vec3 Scale)
{
	mArticulation = Physics->createArticulationReducedCoordinate();
	//mArticulation->setArticulationFlag(physx::PxArticulationFlag::eDISABLE_SELF_COLLISION, true);

	//mRootPart = new CreaturePart(this, PhysicsMaterial, ShapeFlags);
	mRootPart = new CreaturePart(PhysicsMaterial, ShapeFlags);
	mRootPart->mLink = mArticulation->createLink(NULL, physx::PxTransform(physx::PxIdentity));

	mRootPart->AddBoxShape(Physics, Scale, Node);
}

Creature::~Creature()
{
	std::vector<CreaturePart*> Parts = GetAllParts();
	for (auto Part : Parts)
	{
		delete Part;
	}
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

inline static float RandomFloat(float Mult = 1)
{
	return Mult * (float(rand()) / float(RAND_MAX));
}

void Creature::AddRandomPart(physx::PxPhysics* Physics, physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags, GraphicsNode Node)
{
	CreaturePart* ParentPart = GetRandomPart();

	vec3 RandomPointOnParent = ParentPart->mScale;

	/// Pick an axis to place the new shape on
	int RandAxis = rand() % 3;
	switch (RandAxis)
	{
	case(0):
		/// Times these values by [-1 , 1] to get a random point on the parent shape
		RandomPointOnParent.x *= RandomFloat(2) - 1; 
		RandomPointOnParent.y *= RandomFloat(2) - 1;

		/// Times this value by either -1 or 1 to get the maximum or minimum point
		RandomPointOnParent.z *= ((rand() % 2) * 2) - 1;
		break;
	case(1):
		RandomPointOnParent.z *= RandomFloat(2) - 1; 
		RandomPointOnParent.x *= RandomFloat(2) - 1;

		RandomPointOnParent.y *= ((rand() % 2) * 2) - 1;
		break;
	case(2):
		RandomPointOnParent.y *= RandomFloat(2) - 1; 
		RandomPointOnParent.z *= RandomFloat(2) - 1;

		RandomPointOnParent.x *= ((rand() % 2) * 2) - 1;
		break;
	}

	float MAX_SCALE = 3;
	vec3 RandomScale(RandomFloat(MAX_SCALE), RandomFloat(MAX_SCALE), RandomFloat(MAX_SCALE));

	/// Set the position to be the random point we calculated, but also add a random chance to shift it around based on the shape of the child
	vec3 RandomRelativePosition = vec3(	RandomPointOnParent.x + (RandomFloat(2 * RandomScale.x) - RandomScale.x), 
										RandomPointOnParent.y + (RandomFloat(2 * RandomScale.y) - RandomScale.y), 
										RandomPointOnParent.z + (RandomFloat(2 * RandomScale.z) - RandomScale.z));

	/// Set the axis that we determined as the normal to be maxed out
	switch (RandAxis)
	{
	case(0):
		RandomRelativePosition.z = (ParentPart->mScale.z + RandomScale.z) * RandomPointOnParent.z/abs(RandomPointOnParent.z);
		break;
	case(1):
		RandomRelativePosition.y = (ParentPart->mScale.y + RandomScale.y) * RandomPointOnParent.y/abs(RandomPointOnParent.y);
		break;
	case(2):
		RandomRelativePosition.x = (ParentPart->mScale.x + RandomScale.x) * RandomPointOnParent.x/abs(RandomPointOnParent.x);
		break;
	}

	CreaturePart* NewPart = ParentPart->AddChild(Physics, mArticulation, PhysicsMaterial, ShapeFlags, Node, RandomScale, RandomRelativePosition, RandomPointOnParent);
}

void Creature::SetPosition(vec3 Position)
{
	mArticulation->setRootGlobalPose(physx::PxTransform(physx::PxVec3(Position.x, Position.y, Position.z)));
}

void Creature::AddToScene(physx::PxScene* Scene)
{
	Scene->addArticulation(*mArticulation);
}

void Creature::RemoveFromScene(physx::PxScene* Scene)
{
	Scene->removeArticulation(*mArticulation);
}

void Creature::Update()
{
	mRootPart->Update();
}

void Creature::Activate(float NewVel)
{
	std::vector<CreaturePart*> Parts = GetAllParts();
	for (auto& Part : Parts)
	{
		/// NOTE: This check exists since the root node does not have a joint, all other parts should absolutely have joints though
		if (Part->mJoint != nullptr)
			Part->Activate(NewVel);
	}
}

void Creature::Draw(mat4 ViewProjection)
{
	mRootPart->Draw(ViewProjection);
}

void Creature::EnableGravity(bool NewState)
{
	std::vector<CreaturePart*> Parts = GetAllParts();
	for (auto Part : Parts)
	{
		Part->mLink->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !NewState);
	}
}
