#pragma once

#include <glm/glm.hpp>


#include <tinyobjloader/tiny_obj_loader.h>

#include "Shader.h"

class Model
{
public:	
	Model(std::string objFile);
	~Model();
	
private:		
	bool load(std::string objName);
	
	std::vector<glm::vec3> m_vvec3Vertices;
	std::vector<unsigned int> m_vuiIndices;
	
public:
	void initGL();
	void draw(Shader s);

private:
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 norm;
	};

	GLuint m_glVAO, m_glVBO, m_glEBO;
	glm::mat4 m_mat4Model;
	glm::vec3 m_vec3DiffColor, m_vec3SpecColor;
};

