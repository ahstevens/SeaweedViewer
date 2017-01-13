#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#endif

#include "Model.h"
#include <list>
#include <glm/gtc/type_ptr.hpp>

Model::Model(std::string objFile)
	: m_vec3DiffColor(glm::vec3(1.f))
	, m_vec3SpecColor(glm::vec3(1.f))
{
	load(objFile);
	initGL();
}

Model::~Model(void)
{
	m_vvec3Vertices.clear();
	m_vuiIndices.clear();
}

bool Model::load(std::string objName)
{	
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, objName.c_str());

	if (!err.empty()) { // `err` may contain warning message.
		std::cerr << err << std::endl;
	}

	if (!ret) {
		exit(1);
	}

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				float vx = attrib.vertices[3 * idx.vertex_index + 0];
				float vy = attrib.vertices[3 * idx.vertex_index + 1];
				float vz = attrib.vertices[3 * idx.vertex_index + 2];
				if (idx.normal_index >= 0)
				{
					float nx = attrib.normals[3 * idx.normal_index + 0];
					float ny = attrib.normals[3 * idx.normal_index + 1];
					float nz = attrib.normals[3 * idx.normal_index + 2];
				}
				if (idx.texcoord_index >= 0)
				{
					float tx = attrib.texcoords[2 * idx.texcoord_index + 0];
					float ty = attrib.texcoords[2 * idx.texcoord_index + 1];
				}
			}
			index_offset += fv;

			// per-face material
			shapes[s].mesh.material_ids[f];
		}
	}
}

void Model::initGL()
{
	// Create buffers/arrays
	if (!this->m_glVAO) glGenVertexArrays(1, &this->m_glVAO);
	if (!this->m_glVBO) glGenBuffers(1, &this->m_glVBO);
	if (!this->m_glEBO) glGenBuffers(1, &this->m_glEBO);

	std::vector<Vertex> buffer;

	for (auto const &v : m_vvec3Vertices)
	{
		Vertex temp;
		temp.pos = v;
		temp.norm = v;
		buffer.push_back(temp);
	}

	glBindVertexArray(this->m_glVAO);
	// Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
	// A great thing about structs is that their memory layout is sequential for all its items.
	// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
	// again translates to 3/2 floats which translates to a byte array.
	glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(Vertex), &buffer[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_vuiIndices.size() * sizeof(GLuint), &m_vuiIndices[0], GL_STATIC_DRAW);

	// Set the vertex attribute pointers
	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	// Vertex Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, norm));

	glBindVertexArray(0);
}

void Model::draw(Shader s)
{
	glUniform3f(glGetUniformLocation(s.m_nProgram, "material.diffuse"), m_vec3DiffColor.r, m_vec3DiffColor.g, m_vec3DiffColor.b);
	glUniform3f(glGetUniformLocation(s.m_nProgram, "material.specular"), m_vec3SpecColor.r, m_vec3SpecColor.g, m_vec3SpecColor.b);
	glUniform1f(glGetUniformLocation(s.m_nProgram, "material.shininess"), 32.0f);

	glUniformMatrix4fv(glGetUniformLocation(s.m_nProgram, "model"), 1, GL_FALSE, glm::value_ptr(m_mat4Model));
	
	// Draw mesh
	glBindVertexArray(this->m_glVAO);
	glDrawElements(GL_TRIANGLES, m_vuiIndices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
