#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "core/math/mat4.h"
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "render/Material.h"
#include "core/gltf.h"
#include "core/MeshData.h"

// Represents meshes and makes them a bit simpler to use
class MeshResource
{
public:
	unsigned int vertexBuffer;
	unsigned int indexBuffer;
	unsigned int elementCount;
	unsigned int vertexArrayObject;
	BlinnPhongMaterial material;

	MeshResource()
	{
		vertexBuffer = 0;
		indexBuffer = 0;
		elementCount = 0;
		vertexArrayObject = 0;
		material.shader = std::make_shared<ShaderResource>(ShaderResource());
		material.texture = std::make_shared<TextureResource>(TextureResource());
		material.normal = std::make_shared<TextureResource>(TextureResource());
		material.shininess = 32;
	}

	MeshResource(unsigned int vb, unsigned int ib, unsigned int el, unsigned int vao)
	{
		vertexBuffer = vb;
		indexBuffer = ib;
		elementCount = el;
		vertexArrayObject = vao;
		material.shader = std::make_shared<ShaderResource>(ShaderResource());
		material.texture = std::make_shared<TextureResource>(TextureResource());
		material.shininess = 32;
	}

	~MeshResource()
	{
		Destroy();
	}

	void draw()
	{
		if (material.shader != nullptr || material.texture != nullptr)
			material.Apply();
		glBindVertexArray(vertexArrayObject);
		if (indexBuffer)
			glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0);
		else
			glDrawArrays(GL_TRIANGLES, 0, elementCount);
		glBindVertexArray(0);
	}

	std::shared_ptr<MeshResource> MoveToSharedPointer()
	{
		return std::make_shared<MeshResource>(std::move(*this));
	}

	// Takes in four positions and generates any quadrilateral
	void CreateQuadrilateral(vec3 v0, vec3 v1, vec3 v2, vec3 v3)
	{
		GLfloat buf[] =
		{
			v0[0], v0[1], v0[2],			// pos 0
			1,		0,		0,		1,	// color 0
			v1[0], v1[1], v1[2],			// pos 1
			0,		1,		0,		1,	// color 0
			v2[0], v2[1], v2[2],			// pos 2
			0,		0,		1,		1,	// color 0
			v3[0], v3[1], v3[2],			// pos 3
			0,		0,		1,		1	// color 0
		};

		GLuint ib[] =
		{
			0,1,2,
			2,3,0
		};

		elementCount = 6;

		glGenVertexArrays(1, &vertexArrayObject);
		glBindVertexArray(vertexArrayObject);

		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

		// setup vbo
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(buf), buf, GL_STATIC_DRAW);

		// setup ibo
		glGenBuffers(1, &indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ib), ib, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, NULL);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 3));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	// Generates a simple square centered around the origin
	void CreateSquare(float width, float height)
	{
		float depth = -1;
		GLfloat buf[] =
		{
			-width / 2,	-height / 2,	depth,			// pos 0
			1,		0,		0,		1,	// color 0
			-width / 2,	height / 2,	depth,			// pos 1
			0,		1,		0,		1,	// color 0
			width / 2,	height / 2,	depth,			// pos 2
			0,		0,		1,		1,	// color 0
			width / 2,	-height / 2,	depth,			// pos 3
			0,		0,		1,		1	// color 0
		};

		GLuint ib[] =
		{
			0,1,2,
			2,3,0
		};

		elementCount = 6;

		glGenVertexArrays(1, &vertexArrayObject);
		glBindVertexArray(vertexArrayObject);

		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

		// setup vbo
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(buf), buf, GL_STATIC_DRAW);

		// setup ibo
		glGenBuffers(1, &indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ib), ib, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, NULL);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 3));
		
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	}

	void Destroy() noexcept {
		if (vertexBuffer)
		{
			glDeleteBuffers(1, &vertexBuffer);
			vertexBuffer = 0;
		}
		if (vertexArrayObject)
		{
			glDeleteVertexArrays(1, &vertexArrayObject);
			indexBuffer = 0;
		}
		if (indexBuffer)
		{
			glDeleteBuffers(0, &indexBuffer);
			indexBuffer = 0;
		}
	}

	bool LoadOBJ(const char* filepath)
	{
		std::ifstream file(filepath);
		if (!file.is_open())
			std::cout << "[ERROR] FILE DOES NOT EXIST\n";
		std::filesystem::path p = std::filesystem::current_path();

		std::string ps{ p.u8string() };

		assert(file.is_open());

		std::string line;
		
		std::vector<float> positions;
		std::vector<float> uv;
		std::vector<float> normals;

		std::vector<float> buffer;

		int linesRead = 0;

		// Read all the lines in the file
		while (std::getline(file, line))
		{
			//std::cout << "Read " << linesRead << " lines\n";
			linesRead++;
			// If the line starts with 'v ' then add it to the positions
			if (line.find("v ") == 0)
			{
				std::string num;
				int count = 0;
				for (int i = 2; i < line.length(); i++)
				{
					// If the character is not space, add it to the num string
					if (line[i] != ' ')
					{
						num += line[i];
					}
					// If the character is a space, convert the num string to a float and add it to positions, and empty the num string
					if (line[i] == ' ' && num != "")
					{
						if (count < 3)
						{
							positions.push_back(std::stof(num));
							num = "";
							count++;
						}
					}
				}

				// Since I only add to the std::vector when the line ends with a space, I have to have this additional line that adds the last number, I omit it in the uv reading which could screw up some files idk
				if (count == 2 && num != "")
				{
					positions.push_back(std::stof(num));
					count++;
				}
				if (count < 3)
				{
					std::cout << "[ERROR] FILE CORRUPT\n";
					return false;
				}
			}
			// If the line starts with 'vn ' add it to the normals
			if (line.find("vn ") == 0)
			{
				std::string num;
				int count = 0;
				for (int i = 3; i < line.length(); i++)
				{
					if (line[i] != ' ')
					{
						num += line[i];
					}
					if (line[i] == ' ')
					{
						if (count < 3)
						{
							normals.push_back(std::stof(num));
							num = "";
							count++;
						}
					}
				}
				if (count == 2 && num != "")
				{
					normals.push_back(std::stof(num));
					count++;
				}
				if (count < 3)
				{
					std::cout << "[ERROR] FILE CORRUPT\n";
					return false;
				}
			}
			// If the line starts with 'vt ' add it to the uvs
			if (line.find("vt ") == 0)
			{
				std::string num;
				int count = 0;
				for (int i = 3; i < line.length(); i++)
				{
					if (line[i] != ' ')
					{
						num += line[i];
					}
					if (line[i] == ' ')
					{
						if (count < 2)
						{
							uv.push_back(std::stof(num));
							num = "";
						}
						count++;
					}
				}
				if (count < 2)
				{
					std::cout << "[ERROR] FILE CORRUPT\n";
					return false;
				}
			}
			// If the line starts with 'f ', start making the actual buffer
			if (line.find("f ") == 0)
			{
				// Each line starting with an f contains three vertices, so increment elementCount by 3
				elementCount += 3;

				// Create a count variable that starts at 0 and represents which vertex attribute we're dealing with. 0 is position, 1 is uv, and 2 is normal (but it never checks that it's 2)
				int count = 0;

				// Where we collect our number as we iterate through the line
				std::string num;

				// Iterate through the string starting from the third position
				for (int i = 2; i < line.length(); i++)
				{
					// If we hit a / then we want to add either a position or a uv to the buffer
					if (line[i] == '/')
					{
						// Check if it's a position
						if (count == 0)
						{
							int index = std::stoi(num);
							// Check if the index is a positive one or a negative one, and then deal with it properly
							if (index >= 0)
							{
								buffer.push_back(positions[3 * (index - 1)]);
								buffer.push_back(positions[3 * (index - 1) + 1]);
								buffer.push_back(positions[3 * (index - 1) + 2]);
							}
							else if (index < 0)
							{
								buffer.push_back(positions[3 * ((positions.size()/3) + index)]);
								buffer.push_back(positions[3 * ((positions.size()/3) + index) + 1]);
								buffer.push_back(positions[3 * ((positions.size()/3) + index) + 2]);
							}
						}
						// Check if it's a uv
						if (count == 1)
						{
							int index = std::stoi(num);
							if (index >= 0)
							{
								buffer.push_back(uv[2 * (index - 1)]);
								buffer.push_back(uv[2 * (index - 1) + 1]);
							}
							else if (index < 0)
							{
								buffer.push_back(uv[2 * ((uv.size()/2) + index)]);
								buffer.push_back(uv[2 * ((uv.size()/2) + index) + 1]);
							}
						}

						// Clear the num string and increment count so we know if we're dealing with pos, uv, or normals
						num = "";
						count++;
					}
					// If we hit a space, then we want to add a normal to the buffer
					if (line[i] == ' ')
					{
						int index = std::stoi(num);
						if (index >= 0)
						{
							buffer.push_back(normals[3 * (index - 1)]);
							buffer.push_back(normals[3 * (index - 1) + 1]);
							buffer.push_back(normals[3 * (index - 1) + 2]);
						}
						else if (index < 0)
						{
							buffer.push_back(normals[3 * ((normals.size()/3) + index)]);
							buffer.push_back(normals[3 * ((normals.size()/3) + index) + 1]);
							buffer.push_back(normals[3 * ((normals.size()/3) + index) + 2]);
						}

						// Clear the num string and set count to zero since we just added the normals the next number we stumble upon will be a position
						num = "";
						count = 0;
					}
					// And if we hit anything else (probably a number) add it to the num string to later be converted to an int
					else if (line[i] != '/')
					{
						num += line[i];
					}
				}
				// If the line doesn't end with a space then the last number won't get added because we never hit an end point, so we just do a quick check when we're done to see that num is empty, if it isnt add the normals properly (a line will always end with a normal)
				if (num != "")
				{
					int index = std::stoi(num);
					if (index >= 0)
					{
						buffer.push_back(normals[3 * (index - 1)]);
						buffer.push_back(normals[3 * (index - 1) + 1]);
						buffer.push_back(normals[3 * (index - 1) + 2]);
					}
					else if (index < 0)
					{
						buffer.push_back(normals[3 * ((normals.size() / 3) + index)]);
						buffer.push_back(normals[3 * ((normals.size() / 3) + index) + 1]);
						buffer.push_back(normals[3 * ((normals.size() / 3) + index) + 2]);
					}
				}
			}
		}
		file.close();

		glGenVertexArrays(1, &vertexArrayObject);
		glBindVertexArray(vertexArrayObject);

		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), &buffer[0], GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (GLvoid*)(sizeof(float) * 3));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (GLvoid*)(sizeof(float) * 5));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return true;
		//std::map<std::string, int> dict;
	}

	void LoadGLTF(std::string directory, std::string file) {
		fx::gltf::Document obj;
		obj = fx::gltf::LoadFromText((directory + file).c_str());

		int sizeOffset=0;
		
		material.texture->LoadFromFile((directory + (obj.images[obj.materials[0].pbrMetallicRoughness.baseColorTexture.index].uri)).c_str());
		if (obj.materials[0].normalTexture.index != -1)
			material.normal->LoadFromFile((directory + obj.images[obj.materials[0].normalTexture.index].uri).c_str());
		
		std::vector<float> buffer;
		std::vector<GLuint> indices;

		glGenVertexArrays(1, &vertexArrayObject);
		glBindVertexArray(vertexArrayObject);

		for (size_t meshIndex = 0; meshIndex < obj.meshes.size(); meshIndex++) {
			for (size_t primitiveIndex = 0; primitiveIndex < obj.meshes[0].primitives.size(); primitiveIndex++) {
				MeshData meshData(obj, meshIndex, primitiveIndex);

				(void)meshData.VertexBuffer().DataStride;
				
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
						tang[0] = 0;
						tang[1] = 1;
						tang[2] = 0;
						tang[3] = 0;
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
						indices.push_back(((GLuint)((*(GLushort*)(ib.Data + ib.DataStride * i)))) + sizeOffset);
					}

					sizeOffset = buffer.size() / 12;
				}
				else if (ib.DataStride == 4) {
					for (int i = 0; i < (ib.TotalSize / ib.DataStride); i++)
					{
						indices.push_back((*(GLuint*)(ib.Data + ib.DataStride * i)) + sizeOffset);
					}

					sizeOffset = buffer.size() / 12;
				}
			}
		}
		glGenBuffers(1, &indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);

		// setup vbo
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
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

		elementCount = indices.size();
	}

	// Deleted Copy constructor, we don't want this resource copied
	// Also done by compiler when defining a move constructor/assignment 
	MeshResource(const MeshResource&) = delete;

	// Deleted Copy assignment, we don't want this resource copied
	// Also done by compiler when defining a move constructor/assignment 
	MeshResource& operator=(const MeshResource&) = delete;

	// Move constructor
	MeshResource(MeshResource&& other) noexcept {
		vertexBuffer = other.vertexBuffer;
		indexBuffer = other.indexBuffer;
		vertexArrayObject = other.vertexArrayObject;
		elementCount = other.elementCount;
		material = other.material;
		other.vertexBuffer = 0;
		other.indexBuffer = 0;
		other.vertexArrayObject = 0;
		other.elementCount = 0;
		other.material = BlinnPhongMaterial();
	}

	// Move assignment
	MeshResource& operator=(MeshResource&& other) noexcept {
		Destroy();
		vertexBuffer = other.vertexBuffer;
		indexBuffer = other.indexBuffer;
		vertexArrayObject = other.vertexArrayObject;
		elementCount = other.elementCount;
		material = other.material;
		other.vertexBuffer = 0;
		other.indexBuffer = 0;
		other.vertexArrayObject = 0;
		other.elementCount = 0;
		other.material = BlinnPhongMaterial();
		return *this;
	};
};

// Generates a simple quad centered around the origin
/// UV_SCALE is for scaling the texture, since I use a quad for the big ground plane I want the textured grid to repeat more often
static MeshResource CreateQuad(float width, float height, float UV_SCALE = 1)
{
	unsigned int vertexBuffer = 0; 
	unsigned int indexBuffer = 0;
	unsigned int elementCount = 0;
	unsigned int vertexArrayObject = 0;


	float depth = 0;
	GLfloat buf[] =
	{
		-width / 2,	-height / 2,	depth,			// pos 0
		0, 0,										// UV
		0, 0, -1,									// NORMAL
		-width / 2,	height / 2,	depth,				// pos 1
		0, 1 * UV_SCALE,										// UV
		0, 0, -1,									// NORMAL
		width / 2,	height / 2,	depth,				// pos 2
		1 * UV_SCALE, 1 * UV_SCALE,										// UV
		0, 0, -1,									// NORMAL
		width / 2,	-height / 2,	depth,			// pos 3
		1 * UV_SCALE, 0,										// UV
		0, 0, -1,									// NORMAL
	};

	GLuint ib[] =
	{
		0,1,2,
		2,3,0
	};

	elementCount = 6;

	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	// setup vbo
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(buf), buf, GL_STATIC_DRAW);

	// setup ibo
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ib), ib, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (GLvoid*)(sizeof(float) * 3));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (GLvoid*)(sizeof(float) * 5));
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return MeshResource(vertexBuffer, indexBuffer, elementCount, vertexArrayObject);
}

static MeshResource CreateCube(float width, float height, float depth)
{
	unsigned int vertexBuffer = 0; 
	unsigned int indexBuffer = 0;
	unsigned int elementCount = 0;
	unsigned int vertexArrayObject = 0;

	GLfloat buf[] =
	{
		// FIRST SET OF VERTICES
		// FRONT FACE
		-width / 2,		-height / 2,	depth / 2,		// pos 0
		1,		0,		0,		1,						// color 0
		0.0f,	0.0f,									// tex 0
		
		-width / 2,		height / 2,		depth / 2,		// pos 1
		0,		1,		0,		1,						// color 0
		0.0f,	1.0f,									// tex 1
		
		width / 2,		height / 2,		depth / 2,		// pos 2
		0,		0,		1,		1,						// color 0
		1.0f,	1.0f,									// tex 2
		
		width / 2,		-height / 2,	depth / 2,		// pos 3
		0,		0,		1,		1,						// color 0
		1.0f,	0.0f,									// tex 3

		// BACK FACE
		- width / 2,	-height / 2,	-depth / 2,		// pos 4
		1,		0,		0,		1,						// color 0
		1.0f,	0.0f,									// tex 4
		
		-width / 2,		height / 2,		-depth / 2,		// pos 5
		0,		1,		0,		1,						// color 0
		1.0f,	1.0f,									// tex 5
		
		width / 2,		height / 2,		-depth / 2,		// pos 6
		0,		0,		1,		1,						// color 0
		0.0f,	1.0f,									// tex 6
		
		width / 2,		-height / 2,	-depth / 2,		// pos 7
		0,		0,		1,		1,						// color 0
		0.0f,	0.0f,									// tex 7

		
		// SECOND SET OF VERTICES
		// LEFT FACE
		-width / 2,		-height / 2,	depth / 2,		// pos 8
		1,		0,		0,		1,						// color 0
		1.0f,	0.0f,									// tex 0
		
		-width / 2,		height / 2,		depth / 2,		// pos 9
		0,		1,		0,		1,						// color 0
		1.0f,	1.0f,									// tex 1
		
		- width / 2,	-height / 2,	-depth / 2,		// pos 10
		1,		0,		0,		1,						// color 0
		0.0f,	0.0f,									// tex 4
		
		-width / 2,		height / 2,		-depth / 2,		// pos 11
		0,		1,		0,		1,						// color 0
		0.0f,	1.0f,									// tex 5

		// RIGHT FACE
		width / 2,		height / 2,		depth / 2,		// pos 12
		0,		0,		1,		1,						// color 0
		0.0f,	1.0f,									// tex 2
		
		width / 2,		-height / 2,	depth / 2,		// pos 13
		0,		0,		1,		1,						// color 0
		0.0f,	0.0f,									// tex 3
		
		width / 2,		height / 2,		-depth / 2,		// pos 14
		0,		0,		1,		1,						// color 0
		1.0f,	1.0f,									// tex 6
		
		width / 2,		-height / 2,	-depth / 2,		// pos 15
		0,		0,		1,		1,						// color 0
		1.0f,	0.0f,									// tex 7

		
		
		// THIRD SET OF VERTICES
		// TOP FACE
		-width / 2,		height / 2,		depth / 2,		// pos 16
		0,		1,		0,		1,						// color 0
		0.0f,	0.0f,									// tex 1
		
		-width / 2,		height / 2,		-depth / 2,		// pos 17
		0,		1,		0,		1,						// color 0
		0.0f,	1.0f,									// tex 5

		width / 2,		height / 2,		depth / 2,		// pos 18
		0,		0,		1,		1,						// color 0
		1.0f,	0.0f,									// tex 2
		
		width / 2,		height / 2,		-depth / 2,		// pos 19
		0,		0,		1,		1,						// color 0
		1.0f,	1.0f,									// tex 6

		// BOTTOM FACE
		-width / 2,		-height / 2,	depth / 2,		// pos 20
		1,		0,		0,		1,						// color 0
		1.0f,	0.0f,									// tex 0
		
		- width / 2,	-height / 2,	-depth / 2,		// pos 21
		1,		0,		0,		1,						// color 0
		1.0f,	1.0f,									// tex 4
		
		width / 2,		-height / 2,	depth / 2,		// pos 22
		0,		0,		1,		1,						// color 0
		0.0f,	0.0f,									// tex 3
		
		width / 2,		-height / 2,	-depth / 2,		// pos 23
		0,		0,		1,		1,						// color 0
		0.0f,	1.0f									// tex 7


	};

	unsigned int ib[] =
	{
		0,3,2,
		2,1,0,

		7, 4, 5,
		5, 6, 7,

		10,8,9,
		9,11,10,

		13,15,14,
		14,12,13,

		16, 18, 19,
		19, 17, 16,

		22, 20, 21,
		21, 23, 22

	};

	elementCount = 36;

	// Create and bind vao
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	// setup vbo
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(buf), buf, GL_STATIC_DRAW);

	// setup ibo
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ib), ib, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, NULL);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(7 * sizeof(float)));

	// Unbind everything
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	return MeshResource(vertexBuffer, indexBuffer, elementCount, vertexArrayObject);
}