#pragma once

#include "Engine.h"

#include <glm/gtc/type_ptr.hpp>

Engine::Engine()
	: m_pWindow(NULL)
	, m_pLightingSystem(NULL)
	, m_fDeltaTime(0.f)
	, m_fLastTime(0.f)
	, m_pCamera(NULL)
	, m_pShaderLighting(NULL)
	, m_pShaderLamps(NULL)
	, m_iViewLocLightingShader(-1)
	, m_iProjLocLightingShader(-1)
	, m_iViewPosLocLightingShader(-1)
	, m_iShininessLightingShader(-1)
{
}

Engine::~Engine()
{
}

void Engine::receiveEvent(Object * obj, const int event, void * data)
{
	if (event == BroadcastSystem::EVENT::KEY_PRESS)
	{
		int key;
		memcpy(&key, data, sizeof(key));

		if (key == GLFW_KEY_L)
			true;
	}

	if (event == BroadcastSystem::EVENT::MOUSE_UNCLICK)
	{
		int button;
		memcpy(&button, data, sizeof(button));

		if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			glm::vec3 rayFrom = m_pCamera->getPosition();
			glm::vec3 rayTo = rayFrom + m_pCamera->getOrientation()[2] * CAST_RAY_LEN;
			glm::vec3 payload[2] = { rayFrom, rayTo };
		}
	}
}

bool Engine::init()
{
	// Load GLFW 
	glfwInit();

	m_pWindow = init_gl_context("OpenGL Seaweed Viewer");

	if (!m_pWindow)
		return false;

	GLFWInputBroadcaster::getInstance().init(m_pWindow);
	GLFWInputBroadcaster::getInstance().attach(this);  // Register self with input broadcaster

	init_shaders();
	init_camera();
	init_lighting();

	return true;
}

void Engine::mainLoop()
{
	m_fLastTime = static_cast<float>(glfwGetTime());

	// Main Rendering Loop
	while (!glfwWindowShouldClose(m_pWindow)) {
		// Calculate deltatime of current frame
		float newTime = static_cast<float>(glfwGetTime());
		m_fDeltaTime = newTime - m_fLastTime;
		m_fLastTime = newTime;

		// Poll input events first
		GLFWInputBroadcaster::getInstance().poll();

		update(m_fStepSize);

		render();

		// Flip buffers and render to screen
		glfwSwapBuffers(m_pWindow);
	}

	glfwTerminate();
}

void Engine::update(float dt)
{
	m_pCamera->update(dt);
}

void Engine::render()
{
	// OpenGL options
	glEnable(GL_DEPTH_TEST);
	glLineWidth(5.f);

	// Background Fill Color
	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use corresponding shader when setting uniforms/drawing objects
	m_pShaderLighting->use();
	glUniform3f(m_iViewPosLocLightingShader, m_pCamera->getPosition().x, m_pCamera->getPosition().y, m_pCamera->getPosition().z);

	m_pLightingSystem->sLight.position = m_pCamera->getPosition();
	m_pLightingSystem->sLight.direction = glm::vec3(m_pCamera->getOrientation()[2]);

	m_pLightingSystem->setupLighting(*m_pShaderLighting);

	// Create camera transformations
	glm::mat4 view = m_pCamera->getViewMatrix();
	glm::mat4 projection = glm::perspective(
		glm::radians(m_pCamera->getZoom()),
		static_cast<float>(m_iWidth) / static_cast<float>(m_iHeight),
		0.01f,
		1000.0f
		);

	// Pass the matrices to the shader
	glUniformMatrix4fv(m_iViewLocLightingShader, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(m_iProjLocLightingShader, 1, GL_FALSE, glm::value_ptr(projection));

	// Set material properties
	glUniform1f(m_iShininessLightingShader, 32.0f);

	for (auto& shader : m_vpShaders)
	{
		if (shader->status())
		{
			shader->use();
			glUniformMatrix4fv(glGetUniformLocation(shader->m_nProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(shader->m_nProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

			if (shader == m_pShaderLamps)
			{
				m_pLightingSystem->draw(*shader);
			}
			else
			{

				// DRAW HERE

			}
		}
	}

	Shader::off();
}

GLFWwindow* Engine::init_gl_context(std::string winName)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow* mWindow = glfwCreateWindow(m_iWidth, m_iHeight, winName.c_str(), nullptr, nullptr);

	// Check for Valid Context
	if (mWindow == nullptr)
		return nullptr;

	// Create Context and Load OpenGL Functions
	glfwMakeContextCurrent(mWindow);

	// GLFW Options
	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	glewInit();
	fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

	// Define the viewport dimensions
	glViewport(0, 0, m_iWidth, m_iHeight);

	return mWindow;
}

// Initialize the lighting system
void Engine::init_lighting()
{
	m_pLightingSystem = new LightingSystem();
	GLFWInputBroadcaster::getInstance().attach(m_pLightingSystem);

	// Directional light
	m_pLightingSystem->addDLight(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(0.1f), glm::vec3(0.25f), glm::vec3(0.5f));
	m_pLightingSystem->addDLight(glm::vec3(1.f, 1.f, 1.f), glm::vec3(0.1f), glm::vec3(0.25f), glm::vec3(0.5f));

	// Positions of the point lights
	m_pLightingSystem->addPLight(glm::vec3(0.f, 50.f, 0.f));
	m_pLightingSystem->addPLight(glm::vec3(0.f, 50.f, -50.f));
	m_pLightingSystem->addPLight(glm::vec3(50.f, 50.f, 0.f));
	m_pLightingSystem->addPLight(glm::vec3(50.f, 50.f, -50.f));

	// Spotlight
	m_pLightingSystem->addSLight();
}

void Engine::init_camera()
{
	m_pCamera = new Camera(glm::vec3(0.0f, 50.0f, 50.0f));
	GLFWInputBroadcaster::getInstance().attach(m_pCamera);
}

void Engine::init_shaders()
{
	// Build and compile our shader program
	m_pShaderLighting = new Shader(
		// VERTEX SHADER
		"#version 330 core\n"
		"layout(location = 0) in vec3 position;\n"
		"layout(location = 1) in vec3 normal;\n"
		"layout(location = 2) in vec2 texCoords;\n"

		"out vec3 Normal;\n"
		"out vec3 FragPos;\n"
		"out vec2 TexCoords;\n"

		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		
		"void main()\n"
		"{\n"
		"	gl_Position = projection * view *  model * vec4(position, 1.0f);\n"
		"	FragPos = vec3(model * vec4(position, 1.0f));\n"
		"	Normal = mat3(transpose(inverse(model))) * normal;\n"
		"	TexCoords = texCoords;\n"
		"}\n",

		// FRAGMENT SHADER
		"#version 330 core\n"
		"#define NR_POINT_LIGHTS 4\n"

		"struct Material {\n"
		"    sampler2D diffuse;\n"
		"    sampler2D specular;\n"
		"    float shininess;\n"
		"};\n"

		"struct DirLight {\n"
		"    vec3 direction;\n"
		"    vec3 ambient;\n"
		"    vec3 diffuse;\n"
		"    vec3 specular;\n"
		"};\n"

		"struct PointLight {\n"
		"    vec3 position;\n"
		"    float constant;\n"
		"    float linear;\n"
		"    float quadratic;\n"
		"    vec3 ambient;\n"
		"    vec3 diffuse;\n"
		"    vec3 specular;\n"
		"};\n"

		"struct SpotLight {\n"
		"    vec3 position;\n"
		"    vec3 direction;\n"
		"    float cutOff;\n"
		"    float outerCutOff;\n"
		"    float constant;\n"
		"    float linear;\n"
		"    float quadratic;\n"
		"    vec3 ambient;\n"
		"    vec3 diffuse;\n"
		"    vec3 specular;\n"
		"};\n"

		"in vec3 FragPos;\n"
		"in vec3 Normal;\n"
		"in vec2 TexCoords;\n"

		"out vec4 color;\n"

		"uniform vec3 viewPos;\n"
		"uniform DirLight dirLight;\n"
		"uniform PointLight pointLights[NR_POINT_LIGHTS];\n"
		"uniform SpotLight spotLight;\n"
		"uniform Material material;\n"

		// Function prototypes
		"vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);\n"
		"vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);\n"
		"vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);\n"

		"void main()\n"
		"{\n"
		     // Properties
		"    vec3 norm = normalize(Normal);\n"
		"    vec3 viewDir = normalize(viewPos - FragPos);\n"

		     // == ======================================
		     // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
		     // For each phase, a calculate function is defined that calculates the corresponding color
		     // per lamp. In the main() function we take all the calculated colors and sum them up for
		     // this fragment's final color.
		     // == ======================================
		     // Phase 1: Directional lighting
		"    vec3 result = CalcDirLight(dirLight, norm, viewDir);\n"
		     // Phase 2: Point lights\n"
		"    for(int i = 0; i < NR_POINT_LIGHTS; i++)\n"
		"        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);\n"
		     // Phase 3: Spot light\n"
		"    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);\n"

		"    color = vec4(result, 1.0);\n"
		"}\n"

		// Calculates the color when using a directional light.
		"vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)\n"
		"{\n"
		"    vec3 lightDir = normalize(-light.direction);\n"
		     // Diffuse shading
		"    float diff = max(dot(normal, lightDir), 0.0);\n"
		     // Specular shading
		"    vec3 reflectDir = reflect(-lightDir, normal);\n"
		"    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
		     // Combine results
		"    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));\n"
		"    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));\n"
		"    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));\n"
		"    return (ambient + diffuse + specular);\n"
		"}\n"

		// Calculates the color when using a point light.
		"vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)\n"
		"{\n"
		"    vec3 lightDir = normalize(light.position - fragPos);\n"
		     // Diffuse shading
		"    float diff = max(dot(normal, lightDir), 0.0);\n"
		     // Specular shading
		"    vec3 reflectDir = reflect(-lightDir, normal);\n"
		"    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
		     // Attenuation
		"    float distance = length(light.position - fragPos);\n"
		"    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n"
		     // Combine results
		"    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));\n"
		"    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));\n"
		"    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));\n"
		"    ambient *= attenuation;\n"
		"    diffuse *= attenuation;\n"
		"    specular *= attenuation;\n"
		"    return (ambient + diffuse + specular);\n"
		"}\n"

		// Calculates the color when using a spot light.
		"vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)\n"
		"{\n"
		"    vec3 lightDir = normalize(light.position - fragPos);\n"
		     // Diffuse shading
		"    float diff = max(dot(normal, lightDir), 0.0);\n"
		     // Specular shading
		"    vec3 reflectDir = reflect(-lightDir, normal);\n"
		"    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
		     // Attenuation
		"    float distance = length(light.position - fragPos);\n"
		"    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n"
		     // Spotlight intensity
		"    float theta = dot(lightDir, normalize(-light.direction));\n"
		"    float epsilon = light.cutOff - light.outerCutOff;\n"
		"    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);\n"
		     // Combine results
		"    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));\n"
		"    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));\n"
		"    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));\n"
		"    ambient *= attenuation * intensity;\n"
		"    diffuse *= attenuation * intensity;\n"
		"    specular *= attenuation * intensity;\n"
		"    return (ambient + diffuse + specular);\n"
		"}\n"
		);
	m_pShaderLighting->enable();
	m_vpShaders.push_back(m_pShaderLighting);

	m_pShaderLamps = new Shader(
		// Vertex shader
		"#version 330 core\n"
		"layout(location = 0) in vec3 position;\n"

		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"

		"void main()\n"
		"{\n"
		"	gl_Position = projection * view * model * vec4(position, 1.0f);\n"
		"}\n",

		// Fragment shader
		"#version 330 core\n"
		"out vec4 color;\n"

		"uniform vec3 col;\n"

		"void main()\n"
		"{\n"
		"	color = vec4(col.r, col.g, col.b, 1.0f); \n"
		"}\n"
		);
	m_pShaderLamps->enable();
	m_vpShaders.push_back(m_pShaderLamps);

	// Get the uniform locations
	m_iViewLocLightingShader = glGetUniformLocation(m_pShaderLighting->m_nProgram, "view");
	m_iProjLocLightingShader = glGetUniformLocation(m_pShaderLighting->m_nProgram, "projection");
	m_iViewPosLocLightingShader = glGetUniformLocation(m_pShaderLighting->m_nProgram, "viewPos");
	m_iShininessLightingShader = glGetUniformLocation(m_pShaderLighting->m_nProgram, "material.shininess");
}