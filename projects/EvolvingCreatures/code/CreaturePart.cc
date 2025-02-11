#include "CreaturePart.h"

CreaturePart::CreaturePart(physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags) : mPhysicsMaterial(PhysicsMaterial), mShapeFlags(ShapeFlags)
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

void CreaturePart::AddChild(physx::PxPhysics* Physics, physx::PxArticulationReducedCoordinate* Articulation, physx::PxMaterial* PhysicsMaterial, 
	physx::PxShapeFlags ShapeFlags, vec3 Scale, GraphicsNode Node, vec3 RelativePosition, vec3 ParentPosition)
{
	CreaturePart* NewPart = new CreaturePart(PhysicsMaterial, ShapeFlags);
	NewPart->mLink = Articulation->createLink(mLink, physx::PxTransform(physx::PxIdentity));
	NewPart->AddBoxShape(Physics, Scale, Node);

	NewPart->mJoint = NewPart->mLink->getInboundJoint();
	//NewPart->mJoint->setParentPose(physx::PxTransform({ParentPosition.x, ParentPosition.y, ParentPosition.z}));
	NewPart->mJoint->setParentPose(physx::PxTransform(physx::PxIdentity));
	NewPart->mJoint->setChildPose(physx::PxTransform({RelativePosition.x, RelativePosition.y, RelativePosition.z}));

	NewPart->ConfigureJoint();
	
	mChildren.push_back(NewPart);
}

/// TODO: Add options to this for different styled joints
/// PosDrive should probably be a parameter
void CreaturePart::ConfigureJoint()
{
	/// Configure the joint type and motion, limited motion
	mJoint->setJointType(physx::PxArticulationJointType::eREVOLUTE);
	mJoint->setMotion(physx::PxArticulationAxis::eSWING2, physx::PxArticulationMotion::eFREE);
	physx::PxArticulationLimit limits;
	limits.low = -physx::PxPiDivFour;
	limits.high = physx::PxPiDivFour;
	mJoint->setLimitParams(physx::PxArticulationAxis::eSWING2, limits);

	/// Add joint drive
	physx::PxArticulationDrive posDrive;
	posDrive.stiffness = 100;
	posDrive.damping = 10;
	posDrive.maxForce = 1000;
	posDrive.driveType = physx::PxArticulationDriveType::eFORCE;

	/// Apply and Set targets (note the consistent axis)
	mJoint->setDriveParams(physx::PxArticulationAxis::eSWING2, posDrive);
	//mJoint->setDriveVelocity(physx::PxArticulationAxis::eSWING2, 0.0f);
	//mJoint->setDriveTarget(physx::PxArticulationAxis::eSWING2, 0);
}

void CreaturePart::Activate(float Force)
{
	/// Torque doesn't get the physicality that I want
	//mLink->addTorque({ 0, 0, Force });

	mJoint->setDriveVelocity(physx::PxArticulationAxis::eSWING2, Force);
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
