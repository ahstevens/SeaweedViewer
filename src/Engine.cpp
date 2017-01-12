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

	init_lighting();
	init_camera();
	init_shaders();

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

	m_pLightingSystem->sLights[0].position = m_pCamera->getPosition();
	m_pLightingSystem->sLights[0].direction = glm::vec3(m_pCamera->getOrientation()[2]);

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
	m_pShaderLighting = m_pLightingSystem->getShader();
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