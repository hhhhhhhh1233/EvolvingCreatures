#include "CreaturePart.h"

#define FILE_ROOT "C:\\Users\\tagtje-1\\Documents\\EvolvingCreatures\\ProjectFiles\\"

CreaturePart::CreaturePart(physx::PxPhysics* Physics, physx::PxArticulationReducedCoordinate* Articulation, vec3 Position, vec3 Scale, std::shared_ptr<ShaderResource> lightingShader, physx::PxArticulationLink* Parent)
{
	shapeFlags = physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE;
	materialPtr = Physics->createMaterial(0.5f, 0.5f, 0.1f);

	mLink = Articulation->createLink(Parent, physx::PxTransform(physx::PxVec3(Position.x, Position.y, Position.z)));
	{
		physx::PxShape* shape = Physics->createShape(physx::PxBoxGeometry(Scale.x, Scale.y, Scale.z), &materialPtr, 1, true, shapeFlags);
		mLink->attachShape(*shape);
		shape->release();
	}

	mScale = Scale;
	gridTexture.LoadFromFile(FILE_ROOT "images\\Grid.jpg");

	mNode = LoadGLTF(FILE_ROOT "glTFs\\CubeglTF\\", "Cube.gltf", lightingShader, std::make_shared<TextureResource>(gridTexture));
}

void CreaturePart::AddPart(physx::PxPhysics* Physics, physx::PxArticulationReducedCoordinate* Articulation, vec3 PartScale, vec3 PartPosition)
{
	/// Hacky way of getting the shader, DO BETTER
	CreaturePart* NewPart = new CreaturePart(Physics, Articulation, PartPosition, PartScale, mNode.meshes[0]->material.shader, mLink);

	/// Connect the links
	joint = NewPart->mLink->getInboundJoint();
	joint->setParentPose(physx::PxTransform(physx::PxIdentity));
	joint->setChildPose(physx::PxTransform(physx::PxVec3(PartPosition.x, PartPosition.y, PartPosition.z)));

	mConnectedParts.push_back(NewPart);
}

void CreaturePart::Update()
{
	/// Take the position from the mLink
	physx::PxVec3 Pos = mLink->getGlobalPose().p;
	vec3 Position(Pos.x, Pos.y, Pos.z);

	/// Take the rotation from the mLink
	physx::PxQuat Quat = mLink->getGlobalPose().q;
	auto xVec = Quat.getBasisVector0();
	auto yVec = Quat.getBasisVector1();
	auto zVec = Quat.getBasisVector2();
	mat4 RotMat = mat4(vec4(xVec.x, xVec.y, xVec.z, 0), vec4(yVec.x, yVec.y, yVec.z, 0), vec4(zVec.x, zVec.y, zVec.z, 0), vec4(0, 0, 0, 1));
	
	mNode.transform = translate(Position) * RotMat * scale(mScale);

	for (auto Part : mConnectedParts)
	{
		Part->Update();
	}
}

void CreaturePart::Draw(mat4 ViewProjection)
{
	mNode.draw(ViewProjection);

	for (auto Part : mConnectedParts)
	{
		Part->Draw(ViewProjection);
	}
}