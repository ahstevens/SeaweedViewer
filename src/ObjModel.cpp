#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cpp
#endif

#include "ObjModel.h"
#include <list>
#include <map>
#include <glm/gtc/type_ptr.hpp>

ObjModel::ObjModel(std::string objFile)
	: m_vec3DiffColor(glm::vec3(0.f, 0.8f, 0.f))
	, m_vec3SpecColor(glm::vec3(0.f))
{
	load(objFile);
	initGL();
}

ObjModel::~ObjModel(void)
{
	m_vvec3Vertices.clear();
	m_vuiIndices.clear();
}

bool ObjModel::load(std::string objName)
{	
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::map<int, std::vector<glm::vec3>> vertNorms;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, objName.c_str());

	if (!err.empty()) { // `err` may contain warning message.
		std::cerr << err << std::endl;
	}

	if (!ret) {
		return false;
	}

	// Loop over shapes
	int index = 0;
	for (size_t s = 0; s < shapes.size(); s++)
	{
		for (int i = 0; i < attrib.vertices.size(); i += 3)
		{
			m_vvec3Vertices.push_back(glm::vec3(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]));
			m_vvec3Normals.push_back(glm::vec3(attrib.normals[i], attrib.normals[i + 1], attrib.normals[i + 2]));
		}

		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) 
		{
			int fv = shapes[s].mesh.num_face_vertices[f];

			// access to vertex
			tinyobj::index_t idxA = shapes[s].mesh.indices[index_offset + 0];
			tinyobj::index_t idxB = shapes[s].mesh.indices[index_offset + 1];
			tinyobj::index_t idxC = shapes[s].mesh.indices[index_offset + 2];

			glm::vec3 a, b, c;
				
			a.x = attrib.vertices[3 * idxA.vertex_index + 0];
			a.y = attrib.vertices[3 * idxA.vertex_index + 1];
			a.b = attrib.vertices[3 * idxA.vertex_index + 2];

			b.x = attrib.vertices[3 * idxB.vertex_index + 0];
			b.y = attrib.vertices[3 * idxB.vertex_index + 1];
			b.b = attrib.vertices[3 * idxB.vertex_index + 2];

			c.x = attrib.vertices[3 * idxC.vertex_index + 0];
			c.y = attrib.vertices[3 * idxC.vertex_index + 1];
			c.b = attrib.vertices[3 * idxC.vertex_index + 2];

			glm::vec3 norm(glm::cross(b - a, c - a));

			vertNorms[idxA.vertex_index].push_back(norm);
			vertNorms[idxB.vertex_index].push_back(norm);
			vertNorms[idxC.vertex_index].push_back(norm);

			m_vuiIndices.push_back(idxA.vertex_index);
			m_vuiIndices.push_back(idxB.vertex_index);
			m_vuiIndices.push_back(idxC.vertex_index);

			index_offset += fv;

			// per-face material
			shapes[s].mesh.material_ids[f];
		}
	}
	return true;

	for (int i = 0; i < vertNorms.size(); ++i)
	{
		glm::vec3 norm(0.f);

		for (int j = 0; j < vertNorms[i].size(); ++j)
			norm += vertNorms[i][j];

		m_vvec3Normals[i] = glm::normalize(norm);
	}
}

void ObjModel::initGL()
{
	// Create buffers/arrays
	if (!this->m_glVAO) glGenVertexArrays(1, &this->m_glVAO);
	if (!this->m_glVBO) glGenBuffers(1, &this->m_glVBO);
	if (!this->m_glEBO) glGenBuffers(1, &this->m_glEBO);

	std::vector<Vertex> buffer;

	for (int i = 0; i < m_vvec3Vertices.size(); ++i)
	{
		Vertex temp;
		temp.pos = m_vvec3Vertices[i];
		temp.norm = m_vvec3Normals[i];
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

void ObjModel::draw(Shader s)
{
	glUniform3f(glGetUniformLocation(s.m_nProgram, "material.diffuse"), m_vec3DiffColor.r, m_vec3DiffColor.g, m_vec3DiffColor.b);
	glUniform3f(glGetUniformLocation(s.m_nProgram, "material.specular"), m_vec3SpecColor.r, m_vec3SpecColor.g, m_vec3SpecColor.b);
	glUniform1f(glGetUniformLocation(s.m_nProgram, "material.shininess"), 32.f);

	glUniformMatrix4fv(glGetUniformLocation(s.m_nProgram, "model"), 1, GL_FALSE, glm::value_ptr(m_mat4Model));
	
	// Draw mesh
	glBindVertexArray(this->m_glVAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_vuiIndices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
