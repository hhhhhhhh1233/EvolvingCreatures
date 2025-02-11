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
	
	std::vector<CreaturePart*> mChildren;

	GraphicsNode mNode;
	vec3 mScale;

	CreaturePart(physx::PxMaterial* PhysicsMaterial, physx::PxShapeFlags ShapeFlags);

	void AddBoxShape(physx::PxPhysics* Physics, vec3 Scale, GraphicsNode Node);

	void AddChild(physx::PxPhysics* Physics, physx::PxArticulationReducedCoordinate* Articulation, physx::PxMaterial* PhysicsMaterial,
		physx::PxShapeFlags ShapeFlags, vec3 Scale, GraphicsNode Node, vec3 RelativePosition, vec3 ParentPosition);

	/// TODO: Add options to this for different styled joints
	/// PosDrive should probably be a parameter
	void ConfigureJoint();
	void Activate(float Force);
	void Update();
	void Draw(mat4 ViewProjection);
};