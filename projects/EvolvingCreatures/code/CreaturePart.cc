#include "CreaturePart.h"
#include "RandomUtils.h"

CreaturePart::CreaturePart(physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags, float MaxJointVel, float JointOscillationSpeed) : 
	mPhysicsMaterial(PhysicsMaterial), 
	mShapeFlags(ShapeFlags), 
	mMaxJointVel(MaxJointVel), 
	mJointOscillationSpeed(JointOscillationSpeed)
{
	/// Intentionally left blank
}

void CreaturePart::AddBoxShape(physx::PxPhysics* Physics, vec3 Scale, GraphicsNode Node)
{
	physx::PxShape* shape = Physics->createShape(physx::PxBoxGeometry({Scale.x, Scale.y, Scale.z}), &mPhysicsMaterial, 1, true, mShapeFlags);
	mLink->attachShape(*shape);
	shape->release();
	mScale = Scale;
	mNode = Node;
}

CreaturePart* CreaturePart::AddChild(physx::PxPhysics* Physics, physx::PxArticulationReducedCoordinate* Articulation, physx::PxMaterial* PhysicsMaterial, 
	physx::PxShapeFlags ShapeFlags, GraphicsNode Node, vec3 Scale, vec3 RelativePosition, vec3 JointPosition, float MaxJointVel, float JointOscillationSpeed, 
	physx::PxArticulationAxis::Enum JointAxis)
{
	CreaturePart* NewPart = new CreaturePart(PhysicsMaterial, ShapeFlags, MaxJointVel, JointOscillationSpeed);
	NewPart->mLink = Articulation->createLink(mLink, physx::PxTransform(physx::PxIdentity));
	NewPart->AddBoxShape(Physics, Scale, Node);

	NewPart->mJoint = NewPart->mLink->getInboundJoint();
	NewPart->mJoint->setParentPose(physx::PxTransform({JointPosition.x, JointPosition.y, JointPosition.z}));
	NewPart->mJoint->setChildPose(physx::PxTransform({JointPosition.x - RelativePosition.x, JointPosition.y - RelativePosition.y, JointPosition.z - RelativePosition.z}));

	/// Set these values so we can fetch them when we mutate the creature
	NewPart->mJointPosition = JointPosition;
	NewPart->mRelativePosition = RelativePosition;

	NewPart->ConfigureJoint(JointAxis);
	
	mChildren.push_back(NewPart);

	return NewPart;
}

/// TODO: Add options to this for different styled joints
/// PosDrive should probably be a parameter
void CreaturePart::ConfigureJoint(physx::PxArticulationAxis::Enum JointAxis)
{
	/// This sets the joint axis to either eTWIST, eSWING1, or eSWING2
	//mJointAxis = static_cast<physx::PxArticulationAxis::Enum>(RandomInt(3));
	mJointAxis = JointAxis;

	/// Configure the joint type and motion, limited motion
	mJoint->setJointType(physx::PxArticulationJointType::eREVOLUTE);
	mJoint->setMotion(mJointAxis, physx::PxArticulationMotion::eFREE);
	physx::PxArticulationLimit limits;
	limits.low = -physx::PxPiDivFour;
	limits.high = physx::PxPiDivFour;
	mJoint->setLimitParams(mJointAxis, limits);

	/// Add joint drive
	physx::PxArticulationDrive posDrive;
	posDrive.stiffness = 100;
	posDrive.damping = 10;
	posDrive.maxForce = 1000;
	posDrive.driveType = physx::PxArticulationDriveType::eFORCE;

	/// Apply and Set targets (note the consistent axis)
	mJoint->setDriveParams(mJointAxis, posDrive);
	//mJoint->setDriveVelocity(physx::PxArticulationAxis::eSWING2, 0.0f);
	//mJoint->setDriveTarget(physx::PxArticulationAxis::eSWING2, 0);
}

void CreaturePart::Activate(float TimePassed)
{
	mJoint->setDriveVelocity(mJointAxis, mMaxJointVel * sin(mJointOscillationSpeed * TimePassed));
}

void CreaturePart::Update()
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

void CreaturePart::Draw(mat4 ViewProjection)
{
	mNode.draw(ViewProjection);

	for (auto Child : mChildren)
		Child->Draw(ViewProjection);
}
