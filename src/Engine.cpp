#pragma once

#include "Engine.h"

#include <glm/gtc/type_ptr.hpp>

glm::vec3 g_vec3Ambient(0.1f, 0.1f, 0.1f);
glm::vec3 g_vec3Diffuse(0.f, 0.7f, 0.f);
glm::vec3 g_vec3Specular(0.f, 0.f, 0.f);
glm::vec3 g_vec3Emissive(0.f, 0.f, 0.f);
float g_fShininess(32.f);

float *g_pfCurrentEditValue = NULL;
float g_fEditValueDelta = 0.1f;


Engine::Engine(int argc, char* argv[])
	: m_pWindow(NULL)
	, m_pLightingSystem(NULL)
	, m_fDeltaTime(0.f)
	, m_fLastTime(0.f)
	, m_pCamera(NULL)
	, m_pShaderLighting(NULL)
	, m_pShaderNormals(NULL)
	, m_pSphere(NULL)
{
	for (int i = 0; i < argc; ++i)
		m_vstrArgs.push_back(std::string(argv[i]));
}

Engine::~Engine()
{
}

void Engine::receiveEvent(Object * obj, const int event, void * data)
{
	if (event == BroadcastSystem::EVENT::KEY_PRESS || event == BroadcastSystem::EVENT::KEY_REPEAT)
	{
		int key;
		memcpy(&key, data, sizeof(key));

		if (key == GLFW_KEY_P)
		{
			float boxSize = 50.f; // cm
			float totalAreaInside = 0.f;
			for (auto const &obj : m_vpModels)
			{
				float modelAreaInside = 0.f;
				std::vector<unsigned int> inds = obj->getIndices();
				std::vector<glm::vec3> verts = obj->getVertices();
				std::vector<unsigned int> indsRet;
				for (std::vector<unsigned int>::iterator it = inds.begin(); it != inds.end(); it += 3)
				{
					float res = getTriangleSurfaceAreaInAABB(
						verts[*(it + 0)],
						verts[*(it + 1)],
						verts[*(it + 2)],
						glm::vec3(0.f),
						glm::vec3(boxSize, boxSize, -boxSize)
					);
					
					if (res > 0.f)
					{
						indsRet.push_back(*(it + 0));
						indsRet.push_back(*(it + 1));
						indsRet.push_back(*(it + 2));
					}

					modelAreaInside += res;
				}
				std::cout << "\tModel " << obj->getName() << std::endl;
				std::cout << "\t\tSurface area inside " << boxSize << "-cm bounding box = " << modelAreaInside * 2.f << " cm^2" << std::endl;
				totalAreaInside += modelAreaInside;
				obj->setIndices(indsRet);
			}
			std::cout << "Total area inside " << boxSize << "-cm bounding box = " << totalAreaInside * 2.f << " cm^2" << std::endl;
		}

		if (key == GLFW_KEY_RIGHT)
			m_mat4WorldRotation = glm::rotate(m_mat4WorldRotation, glm::radians(1.f), glm::vec3(0.f, 1.f, 0.f));
		if (key == GLFW_KEY_LEFT)
			m_mat4WorldRotation = glm::rotate(m_mat4WorldRotation, glm::radians(-1.f), glm::vec3(0.f, 1.f, 0.f));

		if (key == GLFW_KEY_KP_7)
			g_pfCurrentEditValue = &g_vec3Ambient.r;
		if (key == GLFW_KEY_KP_8)
			g_pfCurrentEditValue = &g_vec3Ambient.g;
		if (key == GLFW_KEY_KP_9)
			g_pfCurrentEditValue = &g_vec3Ambient.b;

		if (key == GLFW_KEY_KP_4)
			g_pfCurrentEditValue = &g_vec3Diffuse.r;
		if (key == GLFW_KEY_KP_5)
			g_pfCurrentEditValue = &g_vec3Diffuse.g;
		if (key == GLFW_KEY_KP_6)
			g_pfCurrentEditValue = &g_vec3Diffuse.b;

		if (key == GLFW_KEY_KP_1)
			g_pfCurrentEditValue = &g_vec3Specular.r;
		if (key == GLFW_KEY_KP_2)
			g_pfCurrentEditValue = &g_vec3Specular.g;
		if (key == GLFW_KEY_KP_3)
			g_pfCurrentEditValue = &g_vec3Specular.b;

		if (key == GLFW_KEY_KP_0)
			g_pfCurrentEditValue = &g_fShininess;

		if (key == GLFW_KEY_KP_ADD)
		{
			if (g_pfCurrentEditValue)
			{
				*g_pfCurrentEditValue += g_fEditValueDelta;
				if (*g_pfCurrentEditValue > 1.f)
					*g_pfCurrentEditValue = 1.f;
			}
		}
		if (key == GLFW_KEY_KP_SUBTRACT)
		{
			if (g_pfCurrentEditValue)
			{
				*g_pfCurrentEditValue -= g_fEditValueDelta;
				if (*g_pfCurrentEditValue < 0.f)
					*g_pfCurrentEditValue = 0.f;
			}
		}
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

	m_pSphere = new Icosphere(4, glm::vec3(0.f, 0.f, 1.f), glm::vec3(1.f));

	for (int i = 1; i < m_vstrArgs.size(); ++i)
		m_vpModels.push_back(new ObjModel(m_vstrArgs[i]));

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

		update(m_fDeltaTime);

		render();

		// Flip buffers and render to screen
		glfwSwapBuffers(m_pWindow);
	}

	glfwTerminate();
}

void Engine::update(float dt)
{
	m_pCamera->update(dt);

	for (auto &pl : m_pLightingSystem->pLights)
	{
		float step = 180.f * dt;
		pl.position = glm::vec3(glm::rotate(glm::mat4(), glm::radians(step), glm::vec3(0.f, 1.f, 0.f)) * glm::vec4(pl.position, 1.f));
		pl.diffuse = (glm::normalize(pl.position) + glm::vec3(1.f)) / glm::vec3(2.f);
		pl.specular = (glm::normalize(pl.position) + glm::vec3(1.f)) / glm::vec3(2.f);
	}

	// Create camera transformations
	glm::mat4 view = m_pCamera->getViewMatrix();
	glm::mat4 projection = glm::perspective(
		glm::radians(m_pCamera->getZoom()),
		static_cast<float>(m_iWidth) / static_cast<float>(m_iHeight),
		0.1f,
		1000.0f
		);

	for (auto &shader : m_vpShaders)
	{
		shader->use();

		glUniformMatrix4fv(glGetUniformLocation(shader->m_nProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader->m_nProgram, "worldRotation"), 1, GL_FALSE, glm::value_ptr(m_mat4WorldRotation));
		glUniformMatrix4fv(glGetUniformLocation(shader->m_nProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		
		if (shader == m_pShaderLighting)
			m_pLightingSystem->update(view, shader);
	}

	Shader::off();
}

void Engine::render()
{
	// OpenGL options
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	// Background Fill Color
	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto& shader : m_vpShaders)
	{
		shader->use();

		//m_pLightingSystem->draw(*shader);

		//m_pSphere->draw(*shader);


		glUniform3fv(glGetUniformLocation(shader->m_nProgram, "material.ambient"), 1, glm::value_ptr(g_vec3Ambient));
		glUniform3fv(glGetUniformLocation(shader->m_nProgram, "material.diffuse"), 1, glm::value_ptr(g_vec3Diffuse));
		glUniform3fv(glGetUniformLocation(shader->m_nProgram, "material.specular"), 1, glm::value_ptr(g_vec3Specular));
		glUniform3fv(glGetUniformLocation(shader->m_nProgram, "material.emissive"), 1, glm::value_ptr(g_vec3Emissive));
		glUniform1f(glGetUniformLocation(shader->m_nProgram, "material.shininess"), g_fShininess);

		for (auto const &m : m_vpModels)
			m->draw(*shader);
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
	glfwWindowHint(GLFW_SAMPLES, 16);
	//glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow* mWindow = glfwCreateWindow(m_iWidth, m_iHeight, winName.c_str(), nullptr, nullptr);
	// Check for Valid Context
	if (mWindow == nullptr)
		return nullptr;

	// Create Context and Load OpenGL Functions
	glfwMakeContextCurrent(mWindow);

	// GLFW Options
	//glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // capture cursor and disable it

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	glewInit();
	fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));
	GLenum err = glGetError(); // clear GL_INVALID_ENUM error from glewInit

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
	m_pLightingSystem->addDirectLight(glm::vec3(1.f)
		, glm::vec3(1.f)
		, glm::vec3(1.f)
		, glm::vec3(1.f)
	);

	// Positions of the point lights
	//m_pLightingSystem->addPointLight(glm::vec3(5.f, 0.f, 5.f));
	//m_pLightingSystem->addPointLight(glm::vec3(5.f, 0.f, -5.f));
	//m_pLightingSystem->addPointLight(glm::vec3(-5.f, 0.f, 5.f));
	//m_pLightingSystem->addPointLight(glm::vec3(-5.f, 0.f, -5.f));

	// Spotlight
	//m_pLightingSystem->addSpotLight();
}

void Engine::init_camera()
{
	m_pCamera = new Camera(glm::vec3(0.f, 0.f, 15.f));
	GLFWInputBroadcaster::getInstance().attach(m_pCamera);
}

void Engine::init_shaders()
{
	// Build and compile our shader program
	m_pShaderLighting = m_pLightingSystem->generateLightingShader();
	m_vpShaders.push_back(m_pShaderLighting);

	std::string vBuffer, fBuffer, gBuffer;

	vBuffer.append("#version 330 core\n");
	vBuffer.append("layout(location = 0) in vec3 position;\n");
	vBuffer.append("layout(location = 1) in vec3 normal;\n");
	vBuffer.append("out VS_OUT{\n");
	vBuffer.append("	vec3 normal;\n");
	vBuffer.append("} vs_out;\n");
	vBuffer.append("uniform mat4 projection;\n");
	vBuffer.append("uniform mat4 view;\n");
	vBuffer.append("uniform mat4 model;\n");
	vBuffer.append("void main()\n");
	vBuffer.append("{\n");
	vBuffer.append("	gl_Position = projection * view * model * vec4(position, 1.0f);\n");
	vBuffer.append("	mat3 normalMatrix = mat3(transpose(inverse(view * model)));\n");
	vBuffer.append("	vs_out.normal = normalize(vec3(projection * vec4(normalMatrix * normal, 1.0)));\n");
	vBuffer.append("}");

	gBuffer.append("#version 330 core\n");
	gBuffer.append("layout(triangles) in;\n");
	gBuffer.append("layout(line_strip, max_vertices = 6) out;\n");
	gBuffer.append("in VS_OUT{\n");
	gBuffer.append("	vec3 normal;\n");
	gBuffer.append("} gs_in[];\n");
	gBuffer.append("const float MAGNITUDE = 1.f;\n");
	gBuffer.append("void GenerateLine(int index)\n");
	gBuffer.append("{\n");
	gBuffer.append("	gl_Position = gl_in[index].gl_Position;\n");
	gBuffer.append("	EmitVertex();\n");
	gBuffer.append("	gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0f) * MAGNITUDE;\n");
	gBuffer.append("	EmitVertex();\n");
	gBuffer.append("	EndPrimitive();\n");
	gBuffer.append("}\n");
	gBuffer.append("void main()\n");
	gBuffer.append("{\n");
	gBuffer.append("	GenerateLine(0); // First vertex normal\n");
	gBuffer.append("	GenerateLine(1); // Second vertex normal\n");
	gBuffer.append("	GenerateLine(2); // Third vertex normal\n");
	gBuffer.append("}");	
		
	fBuffer.append("#version 330 core\n");
	fBuffer.append("out vec4 color;\n");
	fBuffer.append("void main()\n");
	fBuffer.append("{\n");
	fBuffer.append("	color = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n");
	fBuffer.append("}");

	m_pShaderNormals = new Shader(vBuffer.c_str(), fBuffer.c_str(), gBuffer.c_str());
	//m_vpShaders.push_back(m_pShaderNormals);
}

float Engine::getTriangleSurfaceAreaInAABB(glm::vec3 triVert1, glm::vec3 triVert2, glm::vec3 triVert3, glm::vec3 bbMin, glm::vec3 bbMax)
{
	glm::vec3 boxCenter((bbMax - bbMin) * 0.5f);
	glm::vec3 boxHalfExtents(abs(boxCenter));
	float verts[3][3] = { 
		{ triVert1.x, triVert1.y, triVert1.z },
		{ triVert2.x, triVert2.y, triVert2.z },
		{ triVert3.x, triVert3.y, triVert3.z } 
	};

	// No overlap
	if (triBoxOverlap(glm::value_ptr(boxCenter), glm::value_ptr(boxHalfExtents), verts) == 0)
		return 0.0f;

	// Process overlap
	return glm::length(glm::cross(triVert2 - triVert1, triVert3 - triVert1)) * 0.5;
}
