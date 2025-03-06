#pragma once

#include "config.h"
#include "render/GraphicsNode.h"
#include <PxPhysicsAPI.h>

class CreaturePart
{
public:
	physx::PxArticulationLink* mLink = nullptr;
	physx::PxMaterial* mPhysicsMaterial = nullptr;
	physx::PxShapeFlags mShapeFlags;
	physx::PxArticulationJointReducedCoordinate* mJoint = nullptr;
	physx::PxArticulationAxis::Enum mJointAxis;
	
	std::vector<CreaturePart*> mChildren;

	GraphicsNode mNode;
	vec3 mScale;
	vec3 mJointPosition;
	vec3 mRelativePosition;
	vec3 mParentNormal;

	/// These values are for the activation
	float mMaxJointVel = 10;
	float mJointOscillationSpeed = 2;

	CreaturePart(physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags, float MaxJointVel, float JointOscillationSpeed);
	~CreaturePart();

	void AddBoxShape(physx::PxPhysics* Physics, vec3 Scale, GraphicsNode Node);

	CreaturePart* AddChild(physx::PxPhysics* Physics, physx::PxArticulationReducedCoordinate* Articulation, physx::PxMaterial* PhysicsMaterial, 
				physx::PxShapeFlags ShapeFlags, GraphicsNode Node, vec3 Scale, vec3 RelativePosition, vec3 JointPosition, float MaxJointVel, float JointOscillationSpeed, 
				physx::PxArticulationAxis::Enum JointAxis, physx::PxArticulationDrive PosDrive, physx::PxArticulationMotion::Enum JointMotion, physx::PxArticulationLimit JointLimit);

	/// TODO: Add options to this for different styled joints
	/// PosDrive should probably be a parameter
	void ConfigureJoint(physx::PxArticulationAxis::Enum JointAxis, physx::PxArticulationMotion::Enum JointMotion, physx::PxArticulationLimit JointLimit, physx::PxArticulationDrive PosDrive);
	void Activate(float TimePassed);
	void Update();
	void Draw(mat4 ViewProjection);
};