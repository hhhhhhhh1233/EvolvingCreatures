#pragma once
#include <cassert>
#include <memory>
#include "MeshResource.h"
#include "TextureResource.h"
#include "ShaderResource.h"
#include "core/math/mat4.h"

class GraphicsNode
{
public:
	std::vector<std::shared_ptr<MeshResource>> meshes;
	mat4 transform;

	GraphicsNode()
	{
		meshes.push_back(std::make_shared<MeshResource>());
		meshes[0]->material.texture = std::make_shared<TextureResource>();
		meshes[0]->material.shader = std::make_shared<ShaderResource>();
	}

	GraphicsNode(std::shared_ptr<MeshResource> &&Mesh, std::shared_ptr<TextureResource> &&Texture, std::shared_ptr<ShaderResource> Shader, mat4 Transform, float Shininess)
	{
		meshes.push_back(Mesh);
		meshes[0]->material.texture = Texture;
		meshes[0]->material.shader = Shader;
		meshes[0]->material.shininess = Shininess;
		transform = Transform;
	}

	GraphicsNode(std::shared_ptr<MeshResource>&& Mesh, std::shared_ptr<ShaderResource> Shader, mat4 Transform, float Shininess)
	{
		meshes.push_back(Mesh);
		meshes[0]->material.shader = Shader;
		meshes[0]->material.shininess = Shininess;
		transform = Transform;
	}

	void draw(mat4 &viewProjection, std::shared_ptr<ShaderResource> Shader = nullptr)
	{
		for (auto& mesh : meshes)
		{
			if (Shader == nullptr)
				mesh->material.shader->UseProgram();
			else
				Shader->UseProgram();
			mesh->material.shader->SetMatrix("viewProjection", viewProjection);
			mesh->material.shader->SetMatrix("transform", transform);
			mesh->draw();
		}
	}

};

static GraphicsNode LoadGLTF(std::string directory, std::string file, std::shared_ptr<ShaderResource> Shader, std::shared_ptr<TextureResource> Texture = nullptr) {
	GraphicsNode node;
	
	fx::gltf::Document obj;
	obj = fx::gltf::LoadFromText((directory + file).c_str());

	node.meshes.resize(obj.nodes.size());
	for (auto& mesh : node.meshes)
	{
		mesh = std::shared_ptr<MeshResource>(MeshResource().MoveToSharedPointer());
		mesh->material.shader = Shader;
		if (Texture != nullptr)
			mesh->material.texture = Texture;
	}

	for (size_t meshIndex = 0; meshIndex < obj.meshes.size(); meshIndex++) {
	
		glGenVertexArrays(1, &node.meshes[meshIndex]->vertexArrayObject);
		glBindVertexArray(node.meshes[meshIndex]->vertexArrayObject);
		
		if (node.meshes[meshIndex]->material.texture->texture == 0)
			node.meshes[meshIndex]->material.texture->LoadFromFile((directory + (obj.images[obj.materials[meshIndex].pbrMetallicRoughness.baseColorTexture.index].uri)).c_str());
		if (obj.materials[meshIndex].normalTexture.index != -1)
			node.meshes[meshIndex]->material.normal->LoadFromFile((directory + obj.images[obj.materials[meshIndex].normalTexture.index].uri).c_str());

		std::vector<float> buffer;
		std::vector<GLuint> indices;


		for (size_t primitiveIndex = 0; primitiveIndex < obj.meshes[0].primitives.size(); primitiveIndex++) {
			MeshData meshData(obj, meshIndex, primitiveIndex);

			MeshData::BufferInfo const& positions = meshData.VertexBuffer();
			MeshData::BufferInfo const& uvs = meshData.TexCoord0Buffer();
			MeshData::BufferInfo const& normals = meshData.NormalBuffer();
			MeshData::BufferInfo const& tangents = meshData.TangentBuffer();

			size_t count = positions.TotalSize / positions.DataStride;

			for (size_t i = 0; i < count; i++) {
				float pos[3];
				float uv[2];
				float norm[3];
				float tang[4];

				memcpy(&pos, (float*)(positions.Data + positions.DataStride * i), sizeof(pos));
				memcpy(&uv, (float*)(uvs.Data + uvs.DataStride * i), sizeof(uv));
				memcpy(&norm, (float*)(normals.Data + normals.DataStride * i), sizeof(norm));
				if (tangents.Data != nullptr)
					memcpy(&tang, (float*)(tangents.Data + tangents.DataStride * i), sizeof(tang));
				else
				{
					vec3 tanv = cross(vec3(0, 1, 0), vec3(norm[0], norm[1], norm[2]));
					vec4 tanv2 = vec4(tanv.x,tanv.y, tanv.z, 1);
					tang[0] = tanv2.x;
					tang[1] = tanv2.y;
					tang[2] = tanv2.z;
					tang[3] = tanv2.w;
				}

				buffer.push_back(pos[0]);
				buffer.push_back(pos[1]);
				buffer.push_back(pos[2]);

				buffer.push_back(uv[0]);
				buffer.push_back(uv[1]);

				buffer.push_back(norm[0]);
				buffer.push_back(norm[1]);
				buffer.push_back(norm[2]);

				buffer.push_back(tang[0]);
				buffer.push_back(tang[1]);
				buffer.push_back(tang[2]);
				buffer.push_back(tang[3]);
			}

			MeshData::BufferInfo const& ib = meshData.IndexBuffer();


			if (ib.DataStride == 2) {
				for (int i = 0; i < (ib.TotalSize / ib.DataStride); i++)
				{
					indices.push_back((GLuint)(*(GLushort*)(ib.Data + ib.DataStride * i)));
				}
			}
			else if (ib.DataStride == 4) {
				for (int i = 0; i < (ib.TotalSize / ib.DataStride); i++)
				{
					indices.push_back(*(ib.Data + ib.DataStride * i));
				}
			}
		}
		glGenBuffers(1, &node.meshes[meshIndex]->indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, node.meshes[meshIndex]->indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);

		// setup vbo
		glGenBuffers(1, &node.meshes[meshIndex]->vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, node.meshes[meshIndex]->vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * buffer.size(), &buffer[0], GL_STATIC_DRAW);


		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 12, NULL);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 12, (GLvoid*)(sizeof(float) * 3));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 12, (GLvoid*)(sizeof(float) * 5));
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 12, (GLvoid*)(sizeof(float) * 8));


		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		node.meshes[meshIndex]->elementCount = indices.size();
	}

	return node;
}