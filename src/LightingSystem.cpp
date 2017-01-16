#ifndef LIGHTINGSYSTEM_H
#define LIGHTINGSYSTEM_H

#include "LightingSystem.h"

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif // !GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

LightingSystem::LightingSystem() 
	: m_bMeshInitialized(false)
	, m_bRefreshShader(true)
{
}

LightingSystem::~LightingSystem()
{
	dLights.clear();
	pLights.clear();
	sLights.clear();
}

// Uses the current shader
void LightingSystem::setupLighting(Shader *s)
{
	if (m_bRefreshShader)
	{
		if (s)
			delete s;
		s = getShader();
		m_bRefreshShader = false;
	}

	// Directional light
	for (int i = 0; i < dLights.size(); ++i)
	{
		std::string name = "dirLights[" + std::to_string(i);
		name += "]";

		if (dLights[i].on)
		{
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".direction").c_str()), dLights[i].direction.x, dLights[i].direction.y, dLights[i].direction.z);
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".ambient").c_str()), dLights[i].ambient.x, dLights[i].ambient.y, dLights[i].ambient.z);
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".diffuse").c_str()), dLights[i].diffuse.x, dLights[i].diffuse.y, dLights[i].diffuse.z);
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".specular").c_str()), dLights[i].specular.x, dLights[i].specular.y, dLights[i].specular.z);
		}
		else
		{
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".ambient").c_str()), 0.f, 0.f, 0.f);
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".diffuse").c_str()), 0.f, 0.f, 0.f);
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".specular").c_str()), 0.f, 0.f, 0.f);
		}
	}

	// Point light
	for (int i = 0; i < pLights.size(); ++i)
	{
		std::string name = "pointLights[" + std::to_string(i);
		name += "]";

		if (pLights[i].on)
		{
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".position").c_str()), pLights[i].position.x, pLights[i].position.y, pLights[i].position.z);
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".ambient").c_str()), pLights[i].ambient.r, pLights[i].ambient.g, pLights[i].ambient.b);
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".diffuse").c_str()), pLights[i].diffuse.r, pLights[i].diffuse.g, pLights[i].diffuse.b);
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".specular").c_str()), pLights[i].specular.r, pLights[i].specular.g, pLights[i].specular.b);
			glUniform1f(glGetUniformLocation(s->m_nProgram, (name + ".constant").c_str()), pLights[i].constant);
			glUniform1f(glGetUniformLocation(s->m_nProgram, (name + ".linear").c_str()), pLights[i].linear);
			glUniform1f(glGetUniformLocation(s->m_nProgram, (name + ".quadratic").c_str()), pLights[i].quadratic);
		}
		else
		{
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".ambient").c_str()), 0.f, 0.f, 0.f);
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".diffuse").c_str()), 0.f, 0.f, 0.f);
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".specular").c_str()), 0.f, 0.f, 0.f);
		}
	}

	// SpotLight
	for (int i = 0; i < sLights.size(); ++i)
	{
		std::string name = "spotLights[" + std::to_string(i);
		name += "]";

		if (sLights[i].on)
		{
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".position").c_str()), sLights[i].position.x, sLights[i].position.y, sLights[i].position.z);
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".direction").c_str()), sLights[i].direction.x, sLights[i].direction.y, sLights[i].direction.z);
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".ambient").c_str()), sLights[i].ambient.r, sLights[i].ambient.g, sLights[i].ambient.b);
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".diffuse").c_str()), sLights[i].diffuse.r, sLights[i].diffuse.g, sLights[i].diffuse.b);
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".specular").c_str()), sLights[i].specular.r, sLights[i].specular.g, sLights[i].specular.b);
			glUniform1f(glGetUniformLocation(s->m_nProgram, (name + ".constant").c_str()), sLights[i].constant);
			glUniform1f(glGetUniformLocation(s->m_nProgram, (name + ".linear").c_str()), sLights[i].linear);
			glUniform1f(glGetUniformLocation(s->m_nProgram, (name + ".quadratic").c_str()), sLights[i].quadratic);
			glUniform1f(glGetUniformLocation(s->m_nProgram, (name + ".cutOff").c_str()), sLights[i].cutOff);
			glUniform1f(glGetUniformLocation(s->m_nProgram, (name + ".outerCutOff").c_str()), sLights[i].outerCutOff);
		}
		else
		{
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".ambient").c_str()), 0.f, 0.f, 0.f);
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".diffuse").c_str()), 0.f, 0.f, 0.f);
			glUniform3f(glGetUniformLocation(s->m_nProgram, (name + ".specular").c_str()), 0.f, 0.f, 0.f);
		}
	}
}

bool LightingSystem::addDLight(glm::vec3 direction,	glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
{
	DLight dl;

	dl.direction = direction;
	dl.ambient = ambient;
	dl.diffuse = diffuse;
	dl.specular = specular;

	dl.on = true;

	dLights.push_back(dl);

	m_bRefreshShader = true;

	return true;
}

bool LightingSystem::addPLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, GLfloat constant, GLfloat linear, GLfloat quadratic)
{
	PLight pl;
	pl.position = position;
	pl.ambient = ambient;
	pl.diffuse = diffuse;
	pl.specular = specular;
	pl.constant = constant;
	pl.linear = linear;
	pl.quadratic = quadratic;

	pl.on = true;

	pLights.push_back(pl);

	m_bRefreshShader = true;

	return true;
}

bool LightingSystem::addSLight(glm::vec3 position, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, GLfloat constant, GLfloat linear, GLfloat quadratic, GLfloat cutOffDeg, GLfloat outerCutOffDeg, bool attachToCamera)
{
	SLight sl;
	sl.position = position;
	sl.direction = direction;
	sl.ambient = ambient;
	sl.diffuse = diffuse;
	sl.specular = specular;
	sl.constant = constant;
	sl.linear = linear;
	sl.quadratic = quadratic;
	sl.cutOff = glm::cos(glm::radians(cutOffDeg));
	sl.outerCutOff = glm::cos(glm::radians(outerCutOffDeg));
	sl.attachedToCamera = attachToCamera;

	sl.on = true;

	sLights.push_back(sl);

	m_bRefreshShader = true;

	return true;
}

void LightingSystem::draw(Shader s)
{
	if(!m_bMeshInitialized)
		this->setupLightMesh();

	glm::mat4 model;

	glBindVertexArray(this->m_uiVAO);
	for (GLuint i = 0; i < pLights.size(); ++i)
	{
		if(pLights[i].on)
			glUniform3f(glGetUniformLocation(s.m_nProgram, "col"), pLights[i].specular.r, pLights[i].specular.g, pLights[i].specular.b);
		else
			glUniform3f(glGetUniformLocation(s.m_nProgram, "col"), 0.f, 0.f, 0.f);

		model = glm::mat4();
		model = glm::translate(model, pLights[i].position);
		model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
		glUniformMatrix4fv(glGetUniformLocation(s.m_nProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, this->m_nIndices, GL_UNSIGNED_INT, 0);
	}
	glBindVertexArray(0);

}

void LightingSystem::receiveEvent(Object * obj, const int event, void * data)
{
	if (event == BroadcastSystem::EVENT::KEY_PRESS)
	{
		int key;
		memcpy(&key, data, sizeof(key));

		if (key == GLFW_KEY_1)
			for (auto &l : dLights) l.on = !l.on;
		if (key == GLFW_KEY_2)
			for (auto &l : pLights) l.on = !l.on;
		if (key == GLFW_KEY_3)
			for (auto &l : sLights) l.on = !l.on;
	}
}

void LightingSystem::setupLightMesh()
{
	// Construct light geometry
	std::vector<glm::vec3> vertices;
	std::vector<GLuint> indices;

	glm::vec3 frontBotLeft = glm::vec3(-0.5f, -0.5f, 0.5f); // 0
	glm::vec3 frontBotRight = glm::vec3(0.5f, -0.5f, 0.5f); // 1
	glm::vec3 frontTopRight = glm::vec3(0.5f, 0.5f, 0.5f);  // 2
	glm::vec3 frontTopLeft = glm::vec3(-0.5f, 0.5f, 0.5f);  // 3
	glm::vec3 backBotRight = glm::vec3(0.5f, -0.5f, -0.5f); // 4
	glm::vec3 backBotLeft = glm::vec3(-0.5f, -0.5f, -0.5f); // 5
	glm::vec3 backTopLeft = glm::vec3(-0.5f, 0.5f, -0.5f);  // 6
	glm::vec3 backTopRight = glm::vec3(0.5f, 0.5f, -0.5f);  // 7

	vertices = { frontBotLeft, frontBotRight, frontTopRight, frontTopLeft,
					backBotRight, backBotLeft, backTopLeft, backTopRight };
		
	indices = { 0, 1, 2, 2, 3, 0,   // Face 1
				4, 5, 6, 6, 7, 4,   // Face 2
				1, 4, 7, 7, 2, 1,   // Face 3
				5, 0, 3, 3, 6, 5,   // Face 4
				3, 2, 7, 7, 6, 3,   // Face 5
				1, 0, 5, 5, 4, 1 }; // Face 6

	// Send data and its description to GPU
	glGenVertexArrays(1, &m_uiVAO);
	glGenBuffers(1, &m_uiVBO);
	glGenBuffers(1, &m_uiEBO);

	glBindVertexArray(m_uiVAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_uiVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	m_nIndices = static_cast<GLsizei>( indices.size() );

	m_bMeshInitialized = true;
}

Shader* LightingSystem::getShader()
{
	std::string vBuffer, fBuffer;

	// VERTEX SHADER
	vBuffer.append("#version 330 core\n");
	vBuffer.append("layout(location = 0) in vec3 position;\n");
	vBuffer.append("layout(location = 1) in vec3 normal;\n");
	vBuffer.append("out vec3 Normal;\n");
	vBuffer.append("out vec3 FragPos;\n");
	vBuffer.append("uniform mat4 model;\n");
	vBuffer.append("uniform mat4 view;\n");
	vBuffer.append("uniform mat4 projection;\n");
	vBuffer.append("void main()\n");
	vBuffer.append("{\n");
	vBuffer.append("	gl_Position = projection * view *  model * vec4(position, 1.0f);\n");
	vBuffer.append("	FragPos = vec3(model * vec4(position, 1.0f));\n");
	vBuffer.append("	Normal = mat3(transpose(inverse(model))) * normal;\n"); // this preserves correct normals under nonuniform scaling by using the normal matrix
	vBuffer.append("}\n");


	// FRAGMENT SHADER
	fBuffer.append("#version 330 core\n");
	fBuffer.append("#define N_DIR_LIGHTS "); fBuffer.append(std::to_string(dLights.size())); fBuffer.append("\n");
	fBuffer.append("#define N_POINT_LIGHTS "); fBuffer.append(std::to_string(pLights.size())); fBuffer.append("\n");
	fBuffer.append("#define N_SPOT_LIGHTS "); fBuffer.append(std::to_string(sLights.size())); fBuffer.append("\n");

	fBuffer.append("struct Material {\n");
	fBuffer.append("    vec3 diffuse;\n");
	fBuffer.append("    vec3 specular;\n");
	fBuffer.append("    float shininess;\n");
	fBuffer.append("};\n");

	fBuffer.append("struct DirLight {\n");
	fBuffer.append("    vec3 direction;\n");
	fBuffer.append("    vec3 ambient;\n");
	fBuffer.append("    vec3 diffuse;\n");
	fBuffer.append("    vec3 specular;\n");
	fBuffer.append("};\n");

	fBuffer.append("struct PointLight {\n");
	fBuffer.append("    vec3 position;\n");
	fBuffer.append("    float constant;\n");
	fBuffer.append("    float linear;\n");
	fBuffer.append("    float quadratic;\n");
	fBuffer.append("    vec3 ambient;\n");
	fBuffer.append("    vec3 diffuse;\n");
	fBuffer.append("    vec3 specular;\n");
	fBuffer.append("};\n");

	fBuffer.append("struct SpotLight {\n");
	fBuffer.append("    vec3 position;\n");
	fBuffer.append("    vec3 direction;\n");
	fBuffer.append("    float cutOff;\n");
	fBuffer.append("    float outerCutOff;\n");
	fBuffer.append("    float constant;\n");
	fBuffer.append("    float linear;\n");
	fBuffer.append("    float quadratic;\n");
	fBuffer.append("    vec3 ambient;\n");
	fBuffer.append("    vec3 diffuse;\n");
	fBuffer.append("    vec3 specular;\n");
	fBuffer.append("};\n");

	fBuffer.append("in vec3 FragPos;\n");
	fBuffer.append("in vec3 Normal;\n");

	fBuffer.append("out vec4 color;\n");

	fBuffer.append("uniform vec3 viewPos;\n");
	if (dLights.size() > 0)
		fBuffer.append("uniform DirLight dirLights[N_DIR_LIGHTS];\n");
	if (pLights.size() > 0)
		fBuffer.append("uniform PointLight pointLights[N_POINT_LIGHTS];\n");
	if (sLights.size() > 0)
		fBuffer.append("uniform SpotLight spotLights[N_SPOT_LIGHTS];\n");
	fBuffer.append("uniform Material material;\n");
	
	fBuffer.append("vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);\n");
	fBuffer.append("vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);\n");
	fBuffer.append("vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);\n");

	fBuffer.append("void main()\n");
	fBuffer.append("{\n");
	fBuffer.append("    vec3 viewDir = normalize(viewPos - FragPos);\n");
	fBuffer.append("    vec3 norm = normalize(Normal);\n");
	fBuffer.append("    if(!gl_FrontFacing)\n");
	fBuffer.append("		norm = -norm;\n");
	fBuffer.append("    vec3 result = vec3(0.f);\n");
	if (dLights.size() > 0)
	{
		fBuffer.append("    for(int i = 0; i < N_DIR_LIGHTS; i++)\n");
		fBuffer.append("        result += CalcDirLight(dirLights[i], norm, viewDir);\n");
	}
	if (pLights.size() > 0)
	{
		fBuffer.append("    for(int i = 0; i < N_POINT_LIGHTS; i++)\n");
		fBuffer.append("        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);\n");
	}
	if (sLights.size() > 0)
	{
		fBuffer.append("    for(int i = 0; i < N_SPOT_LIGHTS; i++)\n");
		fBuffer.append("        result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);\n");
	}
	fBuffer.append("    color = vec4(result, 1.0);\n");
	//fBuffer.append("    if(!gl_FrontFacing)\n");
	//fBuffer.append("		color.x = 1.f - color.x;\n");
	fBuffer.append("}\n");	

	if (dLights.size() > 0)
	{
		fBuffer.append("vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)\n");
		fBuffer.append("{\n");
		fBuffer.append("    vec3 lightDir = normalize(-light.direction);\n");
		fBuffer.append("    float diff = max(dot(normal, lightDir), 0.0);\n");
		fBuffer.append("    vec3 reflectDir = reflect(-lightDir, normal);\n");
		fBuffer.append("    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n");
		fBuffer.append("    vec3 ambient = light.ambient * material.diffuse;\n");
		fBuffer.append("    vec3 diffuse = light.diffuse * diff * material.diffuse;\n");
		fBuffer.append("    vec3 specular = light.specular * spec * material.specular;\n");
		fBuffer.append("    return (ambient + diffuse + specular);\n");
		fBuffer.append("}\n");
	}

	if (pLights.size() > 0)
	{
		fBuffer.append("vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)\n");
		fBuffer.append("{\n");
		fBuffer.append("    vec3 lightDir = normalize(light.position - fragPos);\n");
		fBuffer.append("    float diff = max(dot(normal, lightDir), 0.0);\n");
		fBuffer.append("    vec3 reflectDir = reflect(-lightDir, normal);\n");
		fBuffer.append("    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n");
		fBuffer.append("    float distance = length(light.position - fragPos);\n");
		fBuffer.append("    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n");
		fBuffer.append("    vec3 ambient = light.ambient * material.diffuse;\n");
		fBuffer.append("    vec3 diffuse = light.diffuse * diff * material.diffuse;\n");
		fBuffer.append("    vec3 specular = light.specular * spec * material.specular;\n");
		fBuffer.append("    ambient *= attenuation;\n");
		fBuffer.append("    diffuse *= attenuation;\n");
		fBuffer.append("    specular *= attenuation;\n");
		fBuffer.append("    return (ambient + diffuse + specular);\n");
		fBuffer.append("}\n");
	}

	if (sLights.size() > 0)
	{
		fBuffer.append("vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)\n");
		fBuffer.append("{\n");
		fBuffer.append("    vec3 lightDir = normalize(light.position - fragPos);\n");
		fBuffer.append("    float diff = max(dot(normal, lightDir), 0.0);\n");
		fBuffer.append("    vec3 reflectDir = reflect(-lightDir, normal);\n");
		fBuffer.append("    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n");
		fBuffer.append("    float distance = length(light.position - fragPos);\n");
		fBuffer.append("    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n");
		fBuffer.append("    float theta = dot(lightDir, normalize(-light.direction));\n");
		fBuffer.append("    float epsilon = light.cutOff - light.outerCutOff;\n");
		fBuffer.append("    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);\n");
		fBuffer.append("    vec3 ambient = light.ambient * material.diffuse;\n");
		fBuffer.append("    vec3 diffuse = light.diffuse * diff * material.diffuse;\n");
		fBuffer.append("    vec3 specular = light.specular * spec * material.specular;\n");
		fBuffer.append("    ambient *= attenuation * intensity;\n");
		fBuffer.append("    diffuse *= attenuation * intensity;\n");
		fBuffer.append("    specular *= attenuation * intensity;\n");
		fBuffer.append("    return (ambient + diffuse + specular);\n");
		fBuffer.append("}\n");
	}

	//std::cout << fBuffer << std::endl;

	return new Shader(vBuffer.c_str(), fBuffer.c_str());
}

#endif