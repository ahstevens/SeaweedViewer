#pragma once

#include <glm/glm.hpp>

#include <tinyobjloader/tiny_obj_loader.h>

#include "Shader.h"

class ObjModel
{
public:	
	ObjModel(std::string objFile);
	~ObjModel();
	
private:		
	bool load(std::string objName);
	
	std::vector<glm::vec3> m_vvec3Vertices;
	std::vector<glm::vec3> m_vvec3Normals;
	std::vector<unsigned int> m_vuiIndices;
	
public:
	void initGL();
	void draw(Shader s);

	std::vector<unsigned int> getIndices();
	void setIndices(std::vector<unsigned int> inds);
	std::vector<glm::vec3> getVertices();

	std::string getName();

private:
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 norm;
	};

	std::string m_strModelName;

	GLuint m_glVAO, m_glVBO, m_glEBO;
	glm::mat4 m_mat4Model;
	glm::vec3 m_vec3DiffColor, m_vec3SpecColor, m_vec3EmisColor;
};

