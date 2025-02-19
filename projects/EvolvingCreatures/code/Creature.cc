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
		RandomPointOnParent.x *= RandomFloat(2) - 1; 
		RandomPointOnParent.y *= RandomFloat(2) - 1;

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

	vec3 RandomRelativePosition = vec3(	RandomPointOnParent.x + (RandomFloat(2 * RandomScale.x) - RandomScale.x), 
										RandomPointOnParent.y + (RandomFloat(2 * RandomScale.y) - RandomScale.y), 
										RandomPointOnParent.z + (RandomFloat(2 * RandomScale.z) - RandomScale.z));

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

	//std::cout << "\nCreating new part\n";
	//if (ParentPart == mRootPart)
	//	std::cout << "Attach to root: TRUE!\n";
	//else
	//	std::cout << "Attach to root: False\n";

	//int MAX_SCALE = 5;
	//float RandomScaleX = RandomFloat(MAX_SCALE);
	//float RandomScaleY = RandomFloat(MAX_SCALE);
	//float RandomScaleZ = RandomFloat(MAX_SCALE);
	//vec3 RandomScale(RandomScaleX, RandomScaleY, RandomScaleZ);

	//vec3 RandomRelativePosition;

	//float MAX_RANGE = 3;
	//int POS_MULT = (((rand() % 2) * 2) - 1);
	//int PLANE = rand() % 3;
	//vec3 JointPos;
	//switch (PLANE)
	//{
	//case(0):
	//{
	//	float X = RandomFloat(RandomScaleX * 2) - RandomScaleX;
	//	float Y = RandomFloat(RandomScaleY * 2) - RandomScaleY;
	//	float Z = (RandomScale.z + ParentPart->mScale.z) * POS_MULT;
	//	std::cout << "Generating on the XY plane!\n";
	//	std::cout << "New pos is: " << X << ", " << Y << ", " << Z << "\n";
	//	RandomRelativePosition = vec3(X, Y, Z);
	//	JointPos = vec3(((RandomRelativePosition.x - RandomScaleX) + ParentPart->mScale.x) / 2, ((RandomRelativePosition.y - RandomScaleY) + ParentPart->mScale.y) / 2, ParentPart->mScale.z * POS_MULT);
	//	break;
	//}
	//case(1):
	//{
	//	float X = RandomFloat(RandomScaleX * 2) - RandomScaleX;
	//	float Y = (RandomScale.y + ParentPart->mScale.y) * POS_MULT;
	//	float Z = RandomFloat(RandomScaleZ * 2) - RandomScaleZ;
	//	std::cout << "Generating on the XZ plane!\n";
	//	std::cout << "New pos is: " << X << ", " << Y << ", " << Z << "\n";
	//	RandomRelativePosition = vec3(X, Y, Z);
	//	JointPos = vec3(((RandomRelativePosition.x - RandomScaleX) + ParentPart->mScale.x) / 2, ParentPart->mScale.y * POS_MULT, ((RandomRelativePosition.y - RandomScaleY) + ParentPart->mScale.y) / 2);
	//	break;
	//}
	//case(2):
	//{
	//	float X = (RandomScale.x + ParentPart->mScale.x) * POS_MULT;
	//	float Y = RandomFloat(RandomScaleY * 2) - RandomScaleY;
	//	float Z = RandomFloat(RandomScaleZ * 2) - RandomScaleZ;
	//	std::cout << "Generating on the YZ plane!\n";
	//	std::cout << "New pos is: " << X << ", " << Y << ", " << Z << "\n";
	//	RandomRelativePosition = vec3(X, Y, Z);
	//	JointPos = vec3(ParentPart->mScale.x * POS_MULT, ((RandomRelativePosition.x - RandomScaleX) + ParentPart->mScale.x) / 2, ((RandomRelativePosition.y - RandomScaleY) + ParentPart->mScale.y) / 2);
	//	break;
	//}
	//}

	//CreaturePart* NewPart = ParentPart->AddChild(Physics, mArticulation, PhysicsMaterial, ShapeFlags, Node, RandomScale, RandomRelativePosition, JointPos);
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
