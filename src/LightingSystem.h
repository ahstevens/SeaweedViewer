#ifndef LIGHTING_H
#define LIGHTING_H

#include <vector>

#include "BroadcastSystem.h"
#include "Shader.h"

#include <glm/glm.hpp>

class LightingSystem : public BroadcastSystem::Listener
{
public:
	struct BasicLight {
		GLboolean on;
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
	};

	struct DLight : BasicLight {
		glm::vec3 direction;
	};

	struct PLight : BasicLight {
		glm::vec3 position;
		GLfloat constant;
		GLfloat linear;
		GLfloat quadratic;
	};

	struct SLight : PLight {
		glm::vec3 direction;
		GLfloat cutOff;
		GLfloat outerCutOff;
		bool attachedToCamera;
	};

public:
	std::vector<DLight> dLights;
	std::vector<PLight> pLights;
	std::vector<SLight> sLights;

public:
	LightingSystem();
	~LightingSystem();

    // Uses the current shader
	void setupLighting(Shader *s);

	bool addDLight(glm::vec3 direction = glm::vec3(-1.0f)
		, glm::vec3 ambient = glm::vec3(0.2f)
		, glm::vec3 diffuse = glm::vec3(1.f)
		, glm::vec3 specular = glm::vec3(1.f)
		);

	bool addPLight(glm::vec3 position = glm::vec3(1.0f)
		, glm::vec3 ambient = glm::vec3(0.05f)
		, glm::vec3 diffuse = glm::vec3(0.8f)
		, glm::vec3 specular = glm::vec3(1.0f)
		, GLfloat constant = 1.0f
		, GLfloat linear = 0.09f
		, GLfloat quadratic = 0.032f
		);

	bool addSLight(glm::vec3 position = glm::vec3(1.0f)
		, glm::vec3 direction = glm::vec3(0.0f)
		, glm::vec3 ambient = glm::vec3(0.0f)
		, glm::vec3 diffuse = glm::vec3(1.0f)
		, glm::vec3 specular = glm::vec3(1.0f)
		, GLfloat constant = 1.0f
		, GLfloat linear = 0.09f
		, GLfloat quadratic = 0.032f
		, GLfloat cutOffDeg = 12.5f
		, GLfloat outerCutOffDeg = 15.0f
		, bool attachToCamera = true
		);

	Shader* getShader();

	void draw(Shader s);

	void receiveEvent(Object * obj, const int event, void * data);

private:
	GLuint m_uiVBO, m_uiVAO, m_uiEBO;
	GLsizei m_nIndices;
	GLboolean m_bMeshInitialized;
	GLboolean m_bRefreshShader;

	void setupLightMesh();
};

#endif