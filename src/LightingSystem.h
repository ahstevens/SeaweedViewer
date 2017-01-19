#ifndef LIGHTING_H
#define LIGHTING_H

#include <vector>

#include "BroadcastSystem.h"
#include "Shader.h"
#include "Icosphere.h"

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
		glm::vec3 position;
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
	void update(glm::mat4 view, Shader *s);

	bool addDirectLight(glm::vec3 position = glm::vec3(1.0f)
		, glm::vec3 ambient = glm::vec3(0.2f)
		, glm::vec3 diffuse = glm::vec3(1.f)
		, glm::vec3 specular = glm::vec3(1.f)
		);

	bool addPointLight(glm::vec3 position = glm::vec3(1.0f)
		, glm::vec3 ambient = glm::vec3(0.05f)
		, glm::vec3 diffuse = glm::vec3(1.f)
		, glm::vec3 specular = glm::vec3(1.f)
		, GLfloat constant = 1.f
		, GLfloat linear = 0.09f
		, GLfloat quadratic = 0.032f
		);

	bool addSpotLight(glm::vec3 position = glm::vec3(1.0f)
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

	Shader* generateLightingShader();

	void draw(Shader s);

	void receiveEvent(Object * obj, const int event, void * data);

	void showPointLights(bool yesno);
	bool toggleShowPointLights();

private:
	GLboolean m_bRefreshShader, m_bDrawLightBulbs;

	Icosphere *m_pLightBulb;
};

#endif