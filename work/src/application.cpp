
// std
#include <iostream>
#include <string>
#include <chrono>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "application.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"


using namespace std;
using namespace cgra;
using namespace glm;


void basic_model::draw(const glm::mat4& view, const glm::mat4 proj) {
    mat4 modelview = view * modelTransform;
    mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelview)));

    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, GL_FALSE, value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, GL_FALSE, value_ptr(modelview));
    glUniformMatrix3fv(glGetUniformLocation(shader, "uNormalMatrix"), 1, GL_FALSE, value_ptr(normalMatrix));
    glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

    glm::vec3 lightDir_world = glm::normalize(glm::vec3(0.5f, 1.0f, 0.3f));
    glm::vec3 lightDir_view = glm::normalize(glm::mat3(view) * lightDir_world);
    glm::vec3 lightColor = glm::vec3(1.3f);
    glm::vec3 ambientColor = glm::vec3(0.35f);

    glUniform3fv(glGetUniformLocation(shader, "uLightDir"), 1, value_ptr(lightDir_view));
    glUniform3fv(glGetUniformLocation(shader, "uLightColor"), 1, value_ptr(lightColor));
    glUniform3fv(glGetUniformLocation(shader, "uAmbientColor"), 1, value_ptr(ambientColor));

    mesh.draw();
}


Application::Application(GLFWwindow* window) : m_window(window) {

	std::vector<std::pair<std::string, std::string>> shaderPaths = {
	{ std::string(CGRA_SRCDIR) + "//res//shaders//lambert_vert.glsl", std::string(CGRA_SRCDIR) + "//res//shaders//lambert_frag.glsl" }
	};

	for (auto& paths : shaderPaths) {
		shader_builder sb;
		sb.set_shader(GL_VERTEX_SHADER, paths.first);
		sb.set_shader(GL_FRAGMENT_SHADER, paths.second);
		m_shaders.push_back(sb.build());
	}

	m_currentShaderIdx = 0;
	m_model.shader = m_shaders[m_currentShaderIdx];

	const int terrainWidth = 1000;
	const int terrainDepth = 1000;

	HeightmapGenerator terrain(terrainWidth, terrainDepth);

	terrain.setParameters(
		12.0f,      // octaves
		0.0002f,  // frequency
		6.0f,  // amplitude
		0.5f,   // gain
		1.0f    // lacunarity
	);

	terrain.regenerate();
	terrain.addCoarseNoise(0.0004f, 3);
	terrain.addCoarseNoise(0.008f, 2);
	//terrain.addCoarseNoise(0.01f, 0.5);
	terrain.addCoarseNoise(0.08f, 0.04);
	terrain.addCoarseNoise(0.8f, 0.008);
	//terrain.addCoarseNoise(0.3f, 0.5);

	m_model.mesh = plane_terrain(terrain.getWidth(), terrain.getDepth(), terrain);
}


void Application::render() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// retrieve the window hieght
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);

	m_windowsize = vec2(width, height); // update window size
	glViewport(0, 0, width, height); // set the viewport to draw to the entire window

	// clear the back-buffer
	glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// enable flags for normal/forward rendering
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// projection matrix
	mat4 proj = perspective(1.f, float(width) / height, 0.1f, 1000.f);

	// view matrix
	mat4 view = translate(mat4(1), vec3(0, 0, -m_distance))
		* rotate(mat4(1), m_pitch, vec3(1, 0, 0))
		* rotate(mat4(1), m_yaw, vec3(0, 1, 0));


	// helpful draw options
	if (m_show_grid) drawGrid(view, proj);
	if (m_show_axis) drawAxis(view, proj);
	glPolygonMode(GL_FRONT_AND_BACK, (m_showWireframe) ? GL_LINE : GL_FILL);

	// draw the model
	m_model.draw(view, proj);
}

void Application::renderGUI() {

	// setup window
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiSetCond_Once);
	ImGui::Begin("Options", 0);

	// display current camera parameters
	ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::SliderFloat("Pitch", &m_pitch, -pi<float>() / 2, pi<float>() / 2, "%.2f");
	ImGui::SliderFloat("Yaw", &m_yaw, -pi<float>(), pi<float>(), "%.2f");
	ImGui::SliderFloat("Distance", &m_distance, 0, 100, "%.2f", 2.0f);

	// helpful drawing options
	ImGui::Checkbox("Show axis", &m_show_axis);
	ImGui::SameLine();
	ImGui::Checkbox("Show grid", &m_show_grid);
	ImGui::Checkbox("Wireframe", &m_showWireframe);
	ImGui::SameLine();
	if (ImGui::Button("Screenshot")) rgba_image::screenshot(true);

	ImGui::Separator();

	 if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Model color");
        if (ImGui::ColorEdit3("##ModelColor", &m_model.color[0])) {
            // color changed — optional debug print
            std::cout << "model color = ("
                      << m_model.color.r << ", "
                      << m_model.color.g << ", "
                      << m_model.color.b << ")\n";
        }

        // quick preset buttons
        ImGui::SameLine();
        if (ImGui::Button("Red"))    m_model.color = glm::vec3(1.0f, 0.0f, 0.0f);
        ImGui::SameLine();
        if (ImGui::Button("Green"))  m_model.color = glm::vec3(0.0f, 1.0f, 0.0f);
        ImGui::SameLine();
        if (ImGui::Button("Blue"))   m_model.color = glm::vec3(0.0f, 0.0f, 1.0f);
        ImGui::SameLine();
        if (ImGui::Button("White"))  m_model.color = glm::vec3(1.0f);

        // small value tweakers if you prefer sliders
        float col[3] = { m_model.color.r, m_model.color.g, m_model.color.b };
        if (ImGui::SliderFloat3("RGB sliders", col, 0.0f, 1.0f)) {
            m_model.color = glm::vec3(col[0], col[1], col[2]);
        }
    }

	// finish creating window
	ImGui::End();
}


void Application::cursorPosCallback(double xpos, double ypos) {
	if (m_leftMouseDown) {
		vec2 whsize = m_windowsize / 2.0f;

		// clamp the pitch to [-pi/2, pi/2]
		m_pitch += float(acos(glm::clamp((m_mousePosition.y - whsize.y) / whsize.y, -1.0f, 1.0f))
			- acos(glm::clamp((float(ypos) - whsize.y) / whsize.y, -1.0f, 1.0f)));
		m_pitch = float(glm::clamp(m_pitch, -pi<float>() / 2, pi<float>() / 2));

		// wrap the yaw to [-pi, pi]
		m_yaw += float(acos(glm::clamp((m_mousePosition.x - whsize.x) / whsize.x, -1.0f, 1.0f))
			- acos(glm::clamp((float(xpos) - whsize.x) / whsize.x, -1.0f, 1.0f)));
		if (m_yaw > pi<float>()) m_yaw -= float(2 * pi<float>());
		else if (m_yaw < -pi<float>()) m_yaw += float(2 * pi<float>());
	}

	// updated mouse position
	m_mousePosition = vec2(xpos, ypos);
}


void Application::mouseButtonCallback(int button, int action, int mods) {
	(void)mods; // currently un-used

	// capture is left-mouse down
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		m_leftMouseDown = (action == GLFW_PRESS); // only other option is GLFW_RELEASE
}


void Application::scrollCallback(double xoffset, double yoffset) {
	(void)xoffset; // currently un-used
	m_distance *= pow(1.1f, -yoffset);
}


void Application::keyCallback(int key, int scancode, int action, int mods) {
	(void)key, (void)scancode, (void)action, (void)mods; // currently un-used
}


void Application::charCallback(unsigned int c) {
	(void)c; // currently un-used
}
