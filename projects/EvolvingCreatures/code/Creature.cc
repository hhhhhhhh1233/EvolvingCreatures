#include "Creature.h"
#include "RandomUtils.h"
#include "flatbuffers/flatbuffers.h"
#include "Creature_generated.h"

Creature::Creature(physx::PxPhysics* Physics, physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags, GraphicsNode Node, vec3 Scale)
{
	mArticulation = Physics->createArticulationReducedCoordinate();
	//mArticulation->setArticulationFlag(physx::PxArticulationFlag::eDISABLE_SELF_COLLISION, true);

	//mRootPart = new CreaturePart(this, PhysicsMaterial, ShapeFlags);
	mRootPart = new CreaturePart(PhysicsMaterial, ShapeFlags, 0, 0);
	mRootPart->mLink = mArticulation->createLink(NULL, physx::PxTransform(physx::PxIdentity));

	mRootPart->AddBoxShape(Physics, Scale, Node);

	mShapes.emplace(mRootPart, BoundingBox(vec3(), Scale));
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
		CurrentPart = CurrentPart->mChildren[RandomInt(CurrentPart->mChildren.size())];

	return CurrentPart;
}

void Creature::RemoveChildlessPart()
{
	CreaturePart* Parent = nullptr;
	CreaturePart* CurrentPart = mRootPart;
	int ChildIndex = 0;

	while (CurrentPart->mChildren.size() > 0)
	{
		Parent = CurrentPart;
		ChildIndex = RandomInt(CurrentPart->mChildren.size());
		CurrentPart = CurrentPart->mChildren[ChildIndex];
	}

	if (Parent == nullptr)
	{
		assert(false, "NO PART TO DELETE");
		return;
	}

	mShapes.erase(CurrentPart);
	delete Parent->mChildren[ChildIndex];
	Parent->mChildren.erase(Parent->mChildren.begin() + ChildIndex);
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

void Creature::DrawBoundingBoxes(mat4 ViewProjection, vec3 Position, GraphicsNode Node)
{
	for (auto& [Part, Shape] : mShapes)
	{
		Node.transform = translate(Position + Shape.GetPosition()) * scale(Shape.GetScale());
		Node.draw(ViewProjection);
	}
}

bool Creature::IsColliding(BoundingBox Box, CreaturePart* ToIgnore)
{
	for (auto& [Part, Shape] : mShapes)
	{
		if (Part == ToIgnore)
			continue;

		if (Shape.IsColliding(Box))
			return true;
	}
	return false;
}

std::pair<BoundingBox, CreaturePart*> Creature::GetRandomShape()
{
	CreaturePart* ParentPart = GetRandomPart();

	vec3 RandomPointOnParent;
	vec3 RandomScale;
	vec3 RandomRelativePosition;

	int TEST_TRIES = 0;

	RandomPointOnParent = ParentPart->mScale;

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
	RandomScale = vec3(RandomFloat(MAX_SCALE), RandomFloat(MAX_SCALE), RandomFloat(MAX_SCALE));

	/// Set the position to be the random point we calculated, but also add a random chance to shift it around based on the shape of the child
	RandomRelativePosition = vec3(	RandomPointOnParent.x + RandomFloatInRange(-RandomScale.x, RandomScale.x), 
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

	return { BoundingBox(mShapes[ParentPart].GetPosition() + RandomRelativePosition, RandomScale), ParentPart };
}

void Creature::AddRandomPart(physx::PxPhysics* Physics, physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags, GraphicsNode Node)
{
	CreaturePart* ParentPart;

	vec3 RandomPointOnParent;
	vec3 RandomScale;
	vec3 RandomRelativePosition;

	int TEST_TRIES = 0;

	do
	{
		ParentPart = GetRandomPart();
		RandomPointOnParent = ParentPart->mScale;

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
		RandomScale = vec3(RandomFloat(MAX_SCALE), RandomFloat(MAX_SCALE), RandomFloat(MAX_SCALE));

		/// Set the position to be the random point we calculated, but also add a random chance to shift it around based on the shape of the child
		RandomRelativePosition = vec3(	RandomPointOnParent.x + RandomFloatInRange(-RandomScale.x, RandomScale.x), 
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

		TEST_TRIES++;
	} while (IsColliding(BoundingBox(mShapes[ParentPart].GetPosition() + RandomRelativePosition, RandomScale), ParentPart));
	if (TEST_TRIES > 1)
		std::cout << "It took " << TEST_TRIES << " tries to generate part!\n";

	float MaxJointVel = RandomFloatInRange(-20, 20);
	float JointOscillationSpeed = RandomFloat(10);
	physx::PxArticulationAxis::Enum JointAxis = static_cast<physx::PxArticulationAxis::Enum>(RandomInt(3));

	physx::PxArticulationDrive posDrive;
	posDrive.stiffness = RandomInt(100);
	posDrive.damping = RandomInt(10);
	posDrive.maxForce = RandomInt(10);
	posDrive.driveType = physx::PxArticulationDriveType::eACCELERATION;

	CreaturePart* NewPart = ParentPart->AddChild(Physics, mArticulation, PhysicsMaterial, ShapeFlags, Node, RandomScale, RandomRelativePosition, RandomPointOnParent, MaxJointVel, JointOscillationSpeed, JointAxis, posDrive);
	mShapes.emplace(NewPart, BoundingBox(mShapes[ParentPart].GetPosition() + RandomRelativePosition, RandomScale));
}

void Creature::SetPosition(vec3 Position)
{
	/// NOTE: If the creature is not part of a scene then this cannot be called, this might be a strange fix, Set position might not be a good name if it also clears forces and such anyhow
	if (mArticulation->getScene() != NULL)
		ClearForceAndTorque();
	mArticulation->setRootGlobalPose(physx::PxTransform(physx::PxVec3(Position.x, Position.y, Position.z)));
}

void Creature::ClearForceAndTorque()
{
	/// If the articulation is not part of a scene it doesn't work to call clear force or torque, I guess that doesn't mean much outside of a scene
	assert(mArticulation->getScene() != NULL);

	mArticulation->setRootAngularVelocity({ 0, 0, 0 });
	mArticulation->setRootLinearVelocity({ 0, 0, 0 });

	std::vector<CreaturePart*> Parts = GetAllParts();
	for (auto& Part : Parts)
	{
		Part->mLink->clearForce();
		Part->mLink->clearTorque();
	}
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

/// TODO: Use the bounding boxes to make sure that the mutations don't overlap anything
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
	NewCreature->mShapes.emplace(NewCreature->mRootPart, BoundingBox(vec3(), RootScale));

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
			NewCreature->mShapes.emplace(NewPart, BoundingBox(MutatedRelativePosition, MutatedScale));

			PartsToLookAt.push_back(ChildPart);
			MutatedPartsToLookAt.push_back(NewPart);
		}
	}

	/// Random chance to add new part
	if (RandomFloat() < MutationChance)
	{
		NewCreature->AddRandomPart(Physics, NewCreature->mRootPart->mPhysicsMaterial, NewCreature->mRootPart->mShapeFlags, NewCreature->mRootPart->mNode);
	}

	/// Random chance to remove part part
	if (RandomFloat() < MutationChance)
	{
		NewCreature->RemoveChildlessPart();
	}

	return NewCreature;
}

Creature* Creature::GetCreatureCopy(physx::PxPhysics* Physics)
{
	vec3 RootScale = mRootPart->mScale;

	Creature* NewCreature = new Creature(Physics, mRootPart->mPhysicsMaterial, mRootPart->mShapeFlags, mRootPart->mNode, RootScale);
	NewCreature->mShapes.emplace(NewCreature->mRootPart, BoundingBox(vec3(), RootScale));

	std::vector<CreaturePart*> PartsToLookAt = { mRootPart };
	std::vector<CreaturePart*> CopyPartsToLookAt = { NewCreature->mRootPart };


	while (PartsToLookAt.size() > 0)
	{
		CreaturePart* CurrentSelfPart = PartsToLookAt[0];
		PartsToLookAt.erase(PartsToLookAt.begin());

		CreaturePart* CurrentCopyPart = CopyPartsToLookAt[0];
		CopyPartsToLookAt.erase(CopyPartsToLookAt.begin());

		for (auto ChildPart : CurrentSelfPart->mChildren)
		{
			vec3 CopyScale = ChildPart->mScale;
			vec3 CopyRelativePosition = ChildPart->mRelativePosition;
			vec3 CopyJointPosition = ChildPart->mJointPosition;
			float CopyMaxJointVel = ChildPart->mMaxJointVel;
			float CopyJointOscillationSpeed = ChildPart->mJointOscillationSpeed;
			physx::PxArticulationAxis::Enum CopyJointAxis = ChildPart->mJointAxis;

			CreaturePart* NewPart = CurrentCopyPart->AddChild(Physics, NewCreature->mArticulation, ChildPart->mPhysicsMaterial, ChildPart->mShapeFlags, ChildPart->mNode, CopyScale, 
																CopyRelativePosition, CopyJointPosition, CopyMaxJointVel, CopyJointOscillationSpeed, CopyJointAxis, 
																ChildPart->mJoint->getDriveParams(ChildPart->mJointAxis));
			NewCreature->mShapes.emplace(NewPart, BoundingBox(CopyRelativePosition, CopyScale));

			PartsToLookAt.push_back(ChildPart);
			CopyPartsToLookAt.push_back(NewPart);
		}
	}

	return NewCreature;
}

Creature* LoadCreatureFromFile(std::string FileName, physx::PxPhysics* Physics, physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags, GraphicsNode Node)
{
	std::ifstream infile(FileName, std::ios::binary | std::ios::in);
	infile.seekg(0, std::ios::end);
	int length = infile.tellg();
	infile.seekg(0, std::ios::beg);
	char* data = new char[length];
	infile.read(data, length);
	infile.close();

	auto InCreature = EvolvingCreature::GetCreature(data);

	auto RootScaleV = InCreature->root_part()->scale();
	vec3 RootScale(RootScaleV->x(), RootScaleV->y(), RootScaleV->z());

	Creature* NewCreature = new Creature(Physics, PhysicsMaterial, ShapeFlags, Node, RootScale);
	NewCreature->mShapes.emplace(NewCreature->mRootPart, BoundingBox(vec3(), RootScale));

	std::vector<const EvolvingCreature::CreaturePart*> PartsToLookAt = { InCreature->root_part() };
	std::vector<CreaturePart*> NewPartsToLookAt = { NewCreature->mRootPart };

	while (PartsToLookAt.size() > 0)
	{
		const EvolvingCreature::CreaturePart* CurrentPart = PartsToLookAt[0];
		PartsToLookAt.erase(PartsToLookAt.begin());

		CreaturePart* NewCurrentPart = NewPartsToLookAt[0];
		NewPartsToLookAt.erase(NewPartsToLookAt.begin());

		for (int i = 0; i < CurrentPart->children()->size(); i++)
		{
			auto scale = CurrentPart->children()->Get(i)->scale();
			auto relative_position = CurrentPart->children()->Get(i)->relative_position();
			auto joint_position = CurrentPart->children()->Get(i)->joint_position();

			float max_joint_vel = CurrentPart->children()->Get(i)->max_joint_vel();
			float joint_oscillation_speed = CurrentPart->children()->Get(i)->joint_oscillation_speed();

			auto joint_axis = CurrentPart->children()->Get(i)->joint_axis();

			vec3 Scale(scale->x(), scale->y(), scale->z());
			vec3 RelativePosition(relative_position->x(), relative_position->y(), relative_position->z());
			vec3 JointPosition(joint_position->x(), joint_position->y(), joint_position->z());

			physx::PxArticulationDrive posDrive;
			posDrive.stiffness = CurrentPart->children()->Get(i)->joint_drive_stiffness();
			posDrive.damping = CurrentPart->children()->Get(i)->joint_drive_damping();
			posDrive.maxForce = CurrentPart->children()->Get(i)->joint_drive_max_force();
			posDrive.driveType = physx::PxArticulationDriveType::eACCELERATION;

			CreaturePart* NewPart = NewCurrentPart->AddChild(Physics, NewCreature->mArticulation, PhysicsMaterial, ShapeFlags, Node, Scale, 
																RelativePosition, JointPosition, max_joint_vel, joint_oscillation_speed, (physx::PxArticulationAxis::Enum)joint_axis, 
																posDrive);

			NewCreature->mShapes.emplace(NewPart, BoundingBox(RelativePosition, Scale));

			PartsToLookAt.push_back(CurrentPart->children()->Get(i));
			NewPartsToLookAt.push_back(NewPart);
		}
	}

	delete[] data;

	return NewCreature;
}

static flatbuffers::Offset<EvolvingCreature::CreaturePart> CreateFlatbufferCreaturePart(flatbuffers::FlatBufferBuilder &Builder, CreaturePart* Part)
{
	std::vector<flatbuffers::Offset<EvolvingCreature::CreaturePart>> CreatureParts;

	for (auto Child : Part->mChildren)
	{
		CreatureParts.push_back(CreateFlatbufferCreaturePart(Builder, Child));
	}

	auto BuPartScale = EvolvingCreature::Vec3(Part->mScale.x, Part->mScale.y, Part->mScale.z);
	auto BuPartRelativePosition = EvolvingCreature::Vec3(Part->mRelativePosition.x, Part->mRelativePosition.y, Part->mRelativePosition.z);
	auto BuPartJointPosition = EvolvingCreature::Vec3(Part->mJointPosition.x, Part->mJointPosition.y, Part->mJointPosition.z);

	float BuDriveStiffness = 0;
	float BuDriveDamping = 0;
	float BuDriveMaxForce = 0;

	/// The root node doesn't have a joint so I have to ignore that ones values
	if (Part->mJoint != nullptr)
	{
		BuDriveStiffness = Part->mJoint->getDriveParams(Part->mJointAxis).stiffness;
		BuDriveDamping = Part->mJoint->getDriveParams(Part->mJointAxis).damping;
		BuDriveMaxForce = Part->mJoint->getDriveParams(Part->mJointAxis).maxForce;
	}

	auto Parts = Builder.CreateVector(CreatureParts);
	auto BuPart = EvolvingCreature::CreateCreaturePart(Builder, &BuPartScale, &BuPartRelativePosition, &BuPartJointPosition, 
			Part->mMaxJointVel, Part->mJointOscillationSpeed, (EvolvingCreature::ArticulationAxis)Part->mJointAxis, BuDriveStiffness,
			BuDriveDamping, BuDriveMaxForce, Parts);

	return BuPart;
}

void SaveCreatureToFile(Creature* CreatureToSave, std::string FileName)
{
	flatbuffers::FlatBufferBuilder builder(1024);

	auto RootPart = CreateFlatbufferCreaturePart(builder, CreatureToSave->mRootPart);
	auto Creature = EvolvingCreature::CreateCreature(builder, RootPart);

	builder.Finish(Creature);

	/// Write the buffer to a file
	std::ofstream ofile(FileName, std::ios::binary);
	assert(ofile.is_open());
	ofile.write((char*)builder.GetBufferPointer(), builder.GetSize());
	ofile.close();
}
