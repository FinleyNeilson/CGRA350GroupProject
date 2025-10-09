//
// Created by Fletcher on 7/10/2025.
//

#include "level_of_detail.h"

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace glm;

namespace lod {
    /**
     * Updates and draws all the models using the lod system
     * @param view
     * @param proj
     */
    void level_of_detail::update_lod(const mat4 &view, mat4 &proj) {
        glUseProgram(shader);
        glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));

        for (size_t i = 0; i < tree_positions.size(); ++i) {
            const auto& pos = tree_positions[i];
            float rotation = tree_rotations[i];
            const int lod = get_lod_level(pos);
            mat4 model = translate(mat4(1.0f), pos);

            model = rotate(model, rotation, vec3(0.f,1.f,0.f));

            model = scale(model, vec3(0.1,0.1,0.1));

            glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, GL_FALSE, glm::value_ptr(view * model));

            if (lod == 0) {
                glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3(0.55f, 0.27f, 0.07f)));
                tree1.draw();
                glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3(0.0f, 0.5f, 0.0f)));
                leaves1.draw();
            } else {
                glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3(0.55f, 0.27f, 0.07f)));
                tree2.draw();
                glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3(0.0f, 0.5f, 0.0f)));
                leaves2.draw();
            }
        }
    }

    /**
     * Returns the lod level the model should be rendered at with lower number being a higher lod
     * @param model_position
     * @return
     */
    int level_of_detail::get_lod_level(const vec3 model_position) const {
        float dist = distance(m_target_position, model_position);
        for (size_t i = 0; i < lod_thresholds.size(); ++i) {
            if (dist < lod_thresholds[i]) {
                return static_cast<int>(i);
            }
        }
        return static_cast<int>(lod_thresholds.size());
    }

    void level_of_detail::create_tree(const vec3& pos, float rotation) {
        tree_positions.push_back(pos);
        tree_rotations.push_back(rotation);
    }

    void level_of_detail::generate_trees(const HeightmapGenerator& m_terrain, float grassTopHeight) {
        tree_positions.clear();
        tree_rotations.clear();
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> angle_dist(0.0f, glm::two_pi<float>());
        for (float x = 0; x<20; x+=1.f) {
            for (float y = 0; y<20; y+=1.f) {
                std::uniform_real_distribution<float> dist_x(-0.3f, 0.3f);
                std::uniform_real_distribution<float> dist_y(-0.3f, 0.3f);
                x +=  dist_x(rng);
                y +=  dist_y(rng);
                float rotation = angle_dist(rng);
                clamp(x, 0.f, 20.f);
                clamp(y, 0.f, 20.f);
                float height = m_terrain.getHeight(x*50.f,y*50.f);
                if (height < grassTopHeight) {
                    create_tree(vec3(x - 10,height ,y - 10), rotation);
                }
            }
        }
    }

    void level_of_detail::draw_lod_target(const glm::mat4 &view, const glm::mat4 &proj) {
        if (!m_show_target) {
            return;
        }

        glUseProgram(shader);
        glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
        glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3(1,0,0)));

        // Move target in a circle if enabled
        if (m_move_target) {
            const auto time = static_cast<float>(glfwGetTime());
            const float radius = 10.0f;
            const float speed = 1.0f;
            const float x = radius * cos(speed * time);
            const float z = radius * sin(speed * time);
            m_target_position = vec3(x, max_height + 2, z);
        } else {
            m_target_position = vec3(0, max_height + 1, 0);
        }

        mat4 model = translate(mat4(1.0f), m_target_position);
        model = scale(model, vec3(0.5));
        glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, GL_FALSE, glm::value_ptr(view * model));
        m_target.draw();

        // Draw outer sphere to visual lod
        if (m_draw_lod_visualize) {
            model = translate(mat4(1.0f), m_target_position);
            model = scale(model, vec3(lod_thresholds.front()));
            glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, GL_FALSE, glm::value_ptr(view * model));
            glPolygonMode(GL_FRONT_AND_BACK, (true) ? GL_LINE : GL_FILL);
            m_target.draw();
        }

    }

} // lod