
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
    glm::vec3 lightColor = glm::vec3(1.4f);
    glm::vec3 ambientColor = glm::vec3(0.45f);

    glUniform3fv(glGetUniformLocation(shader, "uLightDir"), 1, value_ptr(lightDir_view));
    glUniform3fv(glGetUniformLocation(shader, "uLightColor"), 1, value_ptr(lightColor));
    glUniform3fv(glGetUniformLocation(shader, "uAmbientColor"), 1, value_ptr(ambientColor));

    // texture tiling
    glUniform1f(glGetUniformLocation(shader, "uTileX"), tileX);
    glUniform1f(glGetUniformLocation(shader, "uTileZ"), tileZ);

    // bind textures to distinct units
    if (texture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture); // base sand
        glUniform1i(glGetUniformLocation(shader, "uBaseTex"), 0);
        // set wrap if repeating
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    if (texSnow) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texSnow);
        glUniform1i(glGetUniformLocation(shader, "uTexSnow"), 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    if (texStone) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texStone);
        glUniform1i(glGetUniformLocation(shader, "uTexStone"), 2);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    if (texGrass) {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, texGrass);
        glUniform1i(glGetUniformLocation(shader, "uTexGrass"), 3);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    // tints
    glUniform3fv(glGetUniformLocation(shader, "uTintSnow"), 1, value_ptr(tintSnow));
    glUniform3fv(glGetUniformLocation(shader, "uTintStone"), 1, value_ptr(tintStone));
    glUniform3fv(glGetUniformLocation(shader, "uTintGrass"), 1, value_ptr(tintGrass));

    // upload min/max heights (recompute after terrain changes)
    glUniform1f(glGetUniformLocation(shader, "uMinHeight"), minHeight);
    glUniform1f(glGetUniformLocation(shader, "uMaxHeight"), maxHeight);

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

	cgra::rgba_image img(std::string(CGRA_SRCDIR) + "/res/textures/sand.jpg");
    m_model.texture = img.uploadTexture();

	// after creating shaders but before using model
	cgra::rgba_image imgSnow(std::string(CGRA_SRCDIR) + "/res/textures/snow.jpg");
	m_model.texSnow = imgSnow.uploadTexture();

	cgra::rgba_image imgStone(std::string(CGRA_SRCDIR) + "/res/textures/stone.png");
	m_model.texStone = imgStone.uploadTexture();

	cgra::rgba_image imgGrass(std::string(CGRA_SRCDIR) + "/res/textures/grass.png");
	m_model.texGrass = imgGrass.uploadTexture();

	m_currentShaderIdx = 0;
	m_model.shader = m_shaders[m_currentShaderIdx];

    regenerateTerrain();
}

void Application::regenerateTerrain() {
	m_terrain.setParameters(ui_octaves, ui_frequency, ui_amplitude, ui_gain, ui_lacunarity);
	m_terrain.regenerate();

    auto mm = m_terrain.computeMinMax();
	m_model.minHeight = mm.first;
	m_model.maxHeight = mm.second;

	m_model.mesh = plane_terrain(m_terrain.getWidth(), m_terrain.getDepth(), m_terrain);
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

	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 2.5f;

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

		float col[3] = { m_model.color.r, m_model.color.g, m_model.color.b };
		if (ImGui::SliderFloat3("RGB sliders", col, 0.0f, 1.0f)) {
			m_model.color = glm::vec3(col[0], col[1], col[2]);
		}
	}

	if (ImGui::CollapsingHeader("Terrain", ImGuiTreeNodeFlags_DefaultOpen)) {

            // Tiling
    ImGui::Text("Texture tiling");
    float tileX = m_model.tileX;
    float tileZ = m_model.tileZ;
    if (ImGui::InputFloat("Tile X", &tileX, 1.0f, 10.0f)) m_model.tileX = glm::max(0.0001f, tileX);
    if (ImGui::InputFloat("Tile Z", &tileZ, 1.0f, 10.0f)) m_model.tileZ = glm::max(0.0001f, tileZ);

    ImGui::Spacing();
    ImGui::Text("Layer tints (multiplier)");
    // Tints: color pickers
    float tintSnow[3]  = { m_model.tintSnow.r,  m_model.tintSnow.g,  m_model.tintSnow.b };
    float tintStone[3] = { m_model.tintStone.r, m_model.tintStone.g, m_model.tintStone.b };
    float tintGrass[3] = { m_model.tintGrass.r, m_model.tintGrass.g, m_model.tintGrass.b };

    if (ImGui::ColorEdit3("Snow Tint", tintSnow)) {
        m_model.tintSnow = glm::vec3(tintSnow[0], tintSnow[1], tintSnow[2]);
    }
    if (ImGui::ColorEdit3("Stone Tint", tintStone)) {
        m_model.tintStone = glm::vec3(tintStone[0], tintStone[1], tintStone[2]);
    }
    if (ImGui::ColorEdit3("Grass Tint", tintGrass)) {
        m_model.tintGrass = glm::vec3(tintGrass[0], tintGrass[1], tintGrass[2]);
    }

    ImGui::Spacing();
    ImGui::Text("Base fbm2d Layer Parameters");

    ImGui::SliderInt("Octaves", &ui_octaves, 1, 16);
    ImGui::InputFloat("Frequency", &ui_frequency, 0.001f, 0.0f);
    ImGui::InputFloat("Amplitude", &ui_amplitude, 0.01f, 0.0f);
    ImGui::InputFloat("Gain", &ui_gain, 0.01f, 0.0f);
    ImGui::InputFloat("Lacunarity", &ui_lacunarity, 0.01f, 0.0f);
    ImGui::Separator();
    ImGui::Text("Extra Noise Layers");

    // Display each extra layer
    auto& layers = m_terrain.getLayers();
    for (size_t i = 0; i < layers.size(); ++i) {
        auto& layer = layers[i];
        float freq = layer.first;
        float amp  = layer.second;

        ImGui::PushID((int)i); // ensure unique IDs
        ImGui::InputFloat("Freq", &freq, 0.001f, 0.0f);
        ImGui::InputFloat("Amp", &amp, 0.01f, 0.0f);
        if (ImGui::Button("Remove")) {
            m_terrain.removeLayer(i);
            regenerateTerrain();
            ImGui::PopID();
            break; // break to avoid invalidating iterator
        }
        ImGui::PopID();

        // update layer if sliders changed
        layer.first = freq;
        layer.second = amp;
    }

    ImGui::Separator();

    if (ImGui::Button("Regenerate Base + extra layers")) {
        regenerateTerrain();
    }

    ImGui::Text("Add New Layer");
    static float newFreq = 0.005f;
    static float newAmp  = 0.5f;

    ImGui::InputFloat("New Layer Freq", &newFreq, 0.001f, 0.0f);
    ImGui::InputFloat("New Layer Amp", &newAmp, 0.01f, 0.0f);

    if (ImGui::Button("Add Layer")) {
        m_terrain.addLayer(newFreq, newAmp);
        regenerateTerrain();
    }

    ImGui::SameLine();
    if (ImGui::Button("Reset Base")) {
        ui_octaves = HeightmapGenerator::DefaultParams::OCTAVES;
        ui_frequency = HeightmapGenerator::DefaultParams::FREQUENCY;
        ui_amplitude = HeightmapGenerator::DefaultParams::AMPLITUDE;
        ui_gain = HeightmapGenerator::DefaultParams::GAIN;
        ui_lacunarity = HeightmapGenerator::DefaultParams::LACUNARITY;

        regenerateTerrain();
    }


	ImGui::Spacing();
    ImGui::Text("Thermal Erosion");
    ImGui::SliderInt("Iterations", &ui_erosionIterations, 1, 60);
    ImGui::InputFloat("Repose Angle", &ui_reposeAngle, 0.01f, 0.0f);

    if (ImGui::Button("Apply Erosion")) {
        m_terrain.applyThermalErosionMultiNeighbor(ui_erosionIterations, ui_reposeAngle);
        m_model.mesh = plane_terrain(m_terrain.getWidth(), m_terrain.getDepth(), m_terrain);
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
