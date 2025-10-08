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
        glUniform3fv(glGetUniformLocation(shader, "uLightColor"), 1, value_ptr(vec3(1,1,1)));
        glUniform3fv(glGetUniformLocation(shader, "uAmbientColor"), 1, value_ptr(vec3(1,1,1)));

        for (const auto& pos : tree_positions) {
            const int lod = get_lod_level(pos);
            mat4 model = translate(mat4(1.0f), pos);

            model = scale(model, vec3(10,10,10));

            if (lod == 0) {
                glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3(1,1,1)));
                tree1_lod0.draw();
            } else {
                glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3(0,1,1)));
                tree1_lod0.draw();
            }

            glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, GL_FALSE, glm::value_ptr(view * model));

        }
    }

    /**
     * Returns the lod level the model should be rendered at with lower number being a higher lod
     * @param model_position
     * @return
     */
    int level_of_detail::get_lod_level(vec3 model_position) {
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

} // lod