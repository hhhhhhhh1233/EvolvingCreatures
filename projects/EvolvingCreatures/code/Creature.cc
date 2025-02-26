#include "Creature.h"
#include "RandomUtils.h"

Creature::Creature(physx::PxPhysics* Physics, physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags, GraphicsNode Node, vec3 Scale)
{
	mArticulation = Physics->createArticulationReducedCoordinate();
	//mArticulation->setArticulationFlag(physx::PxArticulationFlag::eDISABLE_SELF_COLLISION, true);

	//mRootPart = new CreaturePart(this, PhysicsMaterial, ShapeFlags);
	mRootPart = new CreaturePart(PhysicsMaterial, ShapeFlags, 0, 0);
	mRootPart->mLink = mArticulation->createLink(NULL, physx::PxTransform(physx::PxIdentity));

	mRootPart->AddBoxShape(Physics, Scale, Node);
}

Creature::~Creature()
{
	delete mRootPart;
	mArticulation->release();
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
	CreaturePart* ParentPart = GetRandomPart();

	vec3 RandomPointOnParent = ParentPart->mScale;

	/// Pick an axis to place the new shape on
	int RandAxis = rand() % 3;
	switch (RandAxis)
	{
	case(0):
		/// Times these values by [-1 , 1] to get a random point on the parent shape
		RandomPointOnParent.x *= RandomFloatInRange(-1, 1); 
		RandomPointOnParent.y *= RandomFloatInRange(-1, 1);

		/// Times this value by either -1 or 1 to get the maximum or minimum point
		RandomPointOnParent.z *= (RandomInt(2) * 2) - 1;
		break;
	case(1):
		RandomPointOnParent.z *= RandomFloatInRange(-1, 1); 
		RandomPointOnParent.x *= RandomFloatInRange(-1, 1);

		RandomPointOnParent.y *= (RandomInt(2) * 2) - 1;
		break;
	case(2):
		RandomPointOnParent.y *= RandomFloatInRange(-1, 1); 
		RandomPointOnParent.z *= RandomFloatInRange(-1, 1);

		RandomPointOnParent.x *= (RandomInt(2) * 2) - 1;
		break;
	}

	float MAX_SCALE = 3;
	vec3 RandomScale(RandomFloat(MAX_SCALE), RandomFloat(MAX_SCALE), RandomFloat(MAX_SCALE));

	/// Set the position to be the random point we calculated, but also add a random chance to shift it around based on the shape of the child
	vec3 RandomRelativePosition = vec3(	RandomPointOnParent.x + RandomFloatInRange(-RandomScale.x, RandomScale.x), 
										RandomPointOnParent.y + RandomFloatInRange(-RandomScale.y, RandomScale.y), 
										RandomPointOnParent.z + RandomFloatInRange(-RandomScale.z, RandomScale.z));
	

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

	float MaxJointVel = RandomFloatInRange(-20, 20);
	float JointOscillationSpeed = RandomFloat(10);
	physx::PxArticulationAxis::Enum JointAxis = static_cast<physx::PxArticulationAxis::Enum>(RandomInt(3));

	physx::PxArticulationDrive posDrive;
	posDrive.stiffness = RandomInt(100);
	posDrive.damping = RandomInt(10);
	posDrive.maxForce = RandomInt(1000);
	posDrive.driveType = physx::PxArticulationDriveType::eFORCE;

	CreaturePart* NewPart = ParentPart->AddChild(Physics, mArticulation, PhysicsMaterial, ShapeFlags, Node, RandomScale, RandomRelativePosition, RandomPointOnParent, MaxJointVel, JointOscillationSpeed, JointAxis, posDrive);
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

void Creature::Activate(float TimePassed)
{
	std::vector<CreaturePart*> Parts = GetAllParts();
	for (auto& Part : Parts)
	{
		/// NOTE: This check exists since the root node does not have a joint, all other parts should absolutely have joints though
		if (Part->mJoint != nullptr)
			Part->Activate(TimePassed);
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

Creature* Creature::GetMutatedCreature(physx::PxPhysics* Physics, float MutationChance, float MutationSeverity)
{
	float MinMutationAmount = 1 - MutationSeverity;
	float MaxMutationAmount = 1 + MutationSeverity;

	vec3 RootScale = mRootPart->mScale;
	/// Randomize scale
	if (RandomFloat() < MutationChance)
	{
		RootScale.x *= RandomFloatInRange(MinMutationAmount, MaxMutationAmount);
		RootScale.y *= RandomFloatInRange(MinMutationAmount, MaxMutationAmount);
		RootScale.z *= RandomFloatInRange(MinMutationAmount, MaxMutationAmount);
	}

	Creature* NewCreature = new Creature(Physics, mRootPart->mPhysicsMaterial, mRootPart->mShapeFlags, mRootPart->mNode, RootScale);



	std::vector<CreaturePart*> PartsToLookAt = { mRootPart };
	std::vector<CreaturePart*> MutatedPartsToLookAt = { NewCreature->mRootPart };


	while (PartsToLookAt.size() > 0)
	{
		CreaturePart* CurrentSelfPart = PartsToLookAt[0];
		PartsToLookAt.erase(PartsToLookAt.begin());

		CreaturePart* CurrentMutatedPart = MutatedPartsToLookAt[0];
		MutatedPartsToLookAt.erase(MutatedPartsToLookAt.begin());

		for (auto ChildPart : CurrentSelfPart->mChildren)
		{
			/// TODO: Add in random mutations for the scale, position, and jointposition
			/// and whatever else you'd like

			vec3 MutatedScale = ChildPart->mScale;
			vec3 MutatedRelativePosition = ChildPart->mRelativePosition;
			vec3 MutatedJointPosition = ChildPart->mJointPosition;
			float MutatedMaxJointVel = ChildPart->mMaxJointVel;
			float MutatedJointOscillationSpeed = ChildPart->mJointOscillationSpeed;
			physx::PxArticulationAxis::Enum MutatedJointAxis = ChildPart->mJointAxis;

			/// Randomize scale
			if (RandomFloat() < MutationChance)
			{
				MutatedScale.x *= RandomFloatInRange(MinMutationAmount, MaxMutationAmount);
				MutatedScale.y *= RandomFloatInRange(MinMutationAmount, MaxMutationAmount);
				MutatedScale.z *= RandomFloatInRange(MinMutationAmount, MaxMutationAmount);

				/// This part is adjust the position of the child and the joint based on how the creature mutated scale
				{
					if (ChildPart->mParentNormal.x != 0)
					{
						MutatedRelativePosition.x = ChildPart->mParentNormal.x * (MutatedScale.x + CurrentMutatedPart->mScale.x);
						MutatedJointPosition.x = ChildPart->mParentNormal.x * CurrentMutatedPart->mScale.x;
					}
					if (ChildPart->mParentNormal.y != 0)
					{
						MutatedRelativePosition.y = ChildPart->mParentNormal.y * (MutatedScale.y + CurrentMutatedPart->mScale.y);
						MutatedJointPosition.y = ChildPart->mParentNormal.y * CurrentMutatedPart->mScale.y;
					}
					if (ChildPart->mParentNormal.z != 0)
					{
						MutatedRelativePosition.z = ChildPart->mParentNormal.z * (MutatedScale.z + CurrentMutatedPart->mScale.z);
						MutatedJointPosition.z = ChildPart->mParentNormal.z * CurrentMutatedPart->mScale.z;
					}
				}
			}

			///// Randomize position
			//if (RandomFloat() < MutationChance)
			//{
			//	MutatedRelativePosition.x *= RandomFloatInRange(MinMutationAmount, MaxMutationAmount);
			//	MutatedRelativePosition.y *= RandomFloatInRange(MinMutationAmount, MaxMutationAmount);
			//	MutatedRelativePosition.z *= RandomFloatInRange(MinMutationAmount, MaxMutationAmount);
			//}

			///// Randomize joint position
			//if (RandomFloat() < MutationChance)
			//{
			//	MutatedJointPosition.x *= RandomFloatInRange(MinMutationAmount, MaxMutationAmount);
			//	MutatedJointPosition.y *= RandomFloatInRange(MinMutationAmount, MaxMutationAmount);
			//	MutatedJointPosition.z *= RandomFloatInRange(MinMutationAmount, MaxMutationAmount);
			//}

			/// Randomize joint type
			if (RandomFloat() < MutationChance)
			{
				MutatedJointAxis = static_cast<physx::PxArticulationAxis::Enum>(RandomInt(3));
			}

			/// Randomize joint velocity
			if (RandomFloat() < MutationChance)
			{
				MutatedMaxJointVel *= RandomFloatInRange(MinMutationAmount, MaxMutationAmount);
			}

			/// Randomize joint oscillation
			if (RandomFloat() < MutationChance)
			{
				MutatedJointOscillationSpeed *= RandomFloatInRange(MinMutationAmount, MaxMutationAmount);
			}

			CreaturePart* NewPart = CurrentMutatedPart->AddChild(Physics, NewCreature->mArticulation, ChildPart->mPhysicsMaterial, ChildPart->mShapeFlags, ChildPart->mNode, MutatedScale, 
																MutatedRelativePosition, MutatedJointPosition, MutatedMaxJointVel, MutatedJointOscillationSpeed, MutatedJointAxis, 
																ChildPart->mJoint->getDriveParams(ChildPart->mJointAxis));

			PartsToLookAt.push_back(ChildPart);
			MutatedPartsToLookAt.push_back(NewPart);
		}
	}

	/// Random chance to add new part
	if (RandomFloat() < MutationChance)
	{
		NewCreature->AddRandomPart(Physics, NewCreature->mRootPart->mPhysicsMaterial, NewCreature->mRootPart->mShapeFlags, NewCreature->mRootPart->mNode);
	}

	return NewCreature;
}
