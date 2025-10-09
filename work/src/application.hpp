
#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "level_of_detail.h"
#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"
#include "skeleton_model.hpp"
#include "heightmap_generator.hpp"

// Basic model that holds the shader, mesh and transform for drawing.
// Can be copied and modified for adding in extra information for drawing
// including textures for texture mapping etc.
struct basic_model {
	GLuint shader = 0;
	cgra::gl_mesh mesh;
	glm::vec3 color{ 0.38f, 0.2f, 0.1f };
	glm::mat4 modelTransform{ 1.0 };
	GLuint texture;

	GLuint texSnow = 0;
    GLuint texStone = 0;
    GLuint texGrass = 0;

	glm::vec3 tintSnow = glm::vec3(0.8f, 0.8f, 0.898f);
    glm::vec3 tintStone = glm::vec3(0.737f, 0.745f, 0.792f);
    glm::vec3 tintGrass = glm::vec3(0.333f, 0.451f, 0.184f);

	float minHeight = 0.0f;
	float maxHeight = 0.0f;
	float tileX = 18.0f;
	float tileZ = 18.0f;

	void draw(const glm::mat4& view, const glm::mat4 proj);

};


// Main application class
//
class Application {
private:
	// window
	glm::vec2 m_windowsize;
	GLFWwindow* m_window;

	// oribital camera
	float m_pitch = .86;
	float m_yaw = -.86;
	float m_distance = 20;

	// last input
	bool m_leftMouseDown = false;
	glm::vec2 m_mousePosition;

	// drawing flags
	bool m_show_axis = false;
	bool m_show_grid = false;
	bool m_showWireframe = false;

	std::vector<GLuint> m_shaders;
	int m_currentShaderIdx = 0;

	// ..:: Terrain ::..

	const int terrainWidth = 1000;
	const int terrainDepth = 1000;

	HeightmapGenerator m_terrain{ terrainWidth, terrainDepth };

	int ui_octaves = HeightmapGenerator::DefaultParams::OCTAVES;
	float ui_frequency = HeightmapGenerator::DefaultParams::FREQUENCY;
	float ui_amplitude = HeightmapGenerator::DefaultParams::AMPLITUDE;
	float ui_gain = HeightmapGenerator::DefaultParams::GAIN;
	float ui_lacunarity = HeightmapGenerator::DefaultParams::LACUNARITY;

	int ui_erosionIterations = 60;
	float ui_reposeAngle = 50.0f;

	// geometry
	basic_model m_model;

	// Level of Detail
	lod::level_of_detail LOD;

public:
	// setup
	Application(GLFWwindow*);

	// disable copy constructors (for safety)
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	// rendering callbacks (every frame)
	void regenerateTerrain();
	void render();
	void renderGUI();

	// input callbacks
	void cursorPosCallback(double xpos, double ypos);
	void mouseButtonCallback(int button, int action, int mods);
	void scrollCallback(double xoffset, double yoffset);
	void keyCallback(int key, int scancode, int action, int mods);
	void charCallback(unsigned int c);
};