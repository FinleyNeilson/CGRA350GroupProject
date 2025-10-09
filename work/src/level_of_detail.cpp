//
// Created by Fletcher on 7/10/2025.
//

#include "level_of_detail.h"

#include <glm/glm.hpp>

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

        for (const auto& pos : tree_positions) {
            const int lod = get_lod_level(pos);
            mat4 model = translate(mat4(1.0f), pos);

            model = scale(model, vec3(0.1,0.1,0.1));

            glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, GL_FALSE, glm::value_ptr(view * model));

            if (lod == 0) {
                glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3(1,1,1)));
                tree1_lod0.draw();
            } else {
                glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3(0,1,1)));
                tree1_lod0.draw();
            }
        }
    }

    /**
     * Returns the lod level the model should be rendered at with lower number being a higher lod
     * @param model_position
     * @return
     */
    int level_of_detail::get_lod_level(const vec3 model_position) const {
        float dist = distance(viewer_position, model_position);
        for (size_t i = 0; i < lod_thresholds.size(); ++i) {
            if (dist < lod_thresholds[i]) {
                return static_cast<int>(i);
            }
        }
        return static_cast<int>(lod_thresholds.size());
    }

    void level_of_detail::create_tree(const vec3& pos) {
        tree_positions.push_back(pos);
    }

    void level_of_detail::generate_trees(const HeightmapGenerator& m_terrain) {
        tree_positions.clear();
        std::mt19937 rng(std::random_device{}());
        for (float x = 0; x<20; x+=1.f) {
            for (float y = 0; y<20; y+=1.f) {
                // Add some variance to tree placement
                std::uniform_real_distribution<float> dist_x(-0.3f, 0.3f);
                std::uniform_real_distribution<float> dist_y(-0.3f, 0.3f);
                x +=  dist_x(rng);
                y +=  dist_y(rng);

                // clamp inside the ground area
                clamp(x, 0.f, 20.f);
                clamp(y, 0.f, 20.f);

                // Check height is in the grass level
                float height = m_terrain.getHeight(x*50.f,y*50.f);
                if (height < -2) { // TODO get grass level from m_terrain since changing amplitude messes with this
                    create_tree(vec3(x - 10,height ,y - 10));
                }
            }
        }
    }


} // lod