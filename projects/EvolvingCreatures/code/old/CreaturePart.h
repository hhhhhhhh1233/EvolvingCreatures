#pragma once

#include "config.h"
#include "render/GraphicsNode.h"
#include <PxPhysicsAPI.h>

#define FILE_ROOT "C:\\Users\\tagtje-1\\Documents\\EvolvingCreatures\\ProjectFiles\\"

class CreaturePart
{
private:
	TextureResource gridTexture;

	/// Physics stuff we keep around
	physx::PxShapeFlags shapeFlags;
	physx::PxMaterial* materialPtr;

public:
	GraphicsNode mNode;
	physx::PxArticulationLink* mLink;
	vec3 mScale;
	std::vector<CreaturePart*> mConnectedParts;
	physx::PxArticulationJointReducedCoordinate* joint;

	CreaturePart(physx::PxPhysics* Physics, physx::PxArticulationReducedCoordinate* Articulation, vec3 Position, vec3 Scale, std::shared_ptr<ShaderResource> lightingShader, physx::PxArticulationLink* Parent = NULL);

	void AddPart(physx::PxPhysics* Physics, physx::PxArticulationReducedCoordinate* Articulation, vec3 PartScale, vec3 PartPosition);
	void Update();
	void Draw(mat4 ViewProjection);
};