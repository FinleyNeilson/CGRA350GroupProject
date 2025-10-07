
#pragma once

#include "cgra_mesh.hpp"
#include <heightmap_generator.hpp>
 
namespace cgra {
	
	// creates a mesh for a unit sphere (radius of 1)
	// immediately draws the sphere mesh, assuming the shader is set up
	void drawSphere();

	gl_mesh plane_terrain(int width, int depth, HeightmapGenerator& terrain, glm::vec3 offset = { -10,0,-10 }, float scaleX = 0.02f, float scaleZ = 0.02f);

	// creates a mesh for a unit cylinder (radius and hieght of 1) along the z-axis
	// immediately draws the sphere mesh, assuming the shader is set up
	void drawCylinder();
	
	// creates a mesh for a unit cone (radius and hieght of 1) along the z-axis
	// immediately draws the sphere mesh, assuming the shader is set up
	void drawCone();

	// sets up a shader and draws an axis straight to the current framebuffer
	void drawAxis(const glm::mat4 &view, const glm::mat4 &proj);

	// sets up a shader and draws a grid straight to the current framebuffer
	void drawGrid(const glm::mat4 &view, const glm::mat4 &proj);
}