//
// Created by Fletcher on 7/10/2025.
//

#ifndef LEVEL_OF_DETAIL_H
#define LEVEL_OF_DETAIL_H
#include <glm/glm.hpp>
#include <vector>

#include "heightmap_generator.hpp"
#include "cgra/cgra_mesh.hpp"
#include "cgra/cgra_wavefront.hpp"

namespace lod {

class level_of_detail {
private:
    const std::string TREE_FILE = CGRA_SRCDIR "/res/assets/a.obj";
    cgra::gl_mesh tree1 = cgra::load_wavefront_data(TREE_FILE).build();
    const std::string LEAVES_FILE = CGRA_SRCDIR "/res/assets/al.obj";
    cgra::gl_mesh leaves1 = cgra::load_wavefront_data(LEAVES_FILE).build();

    const std::string TREE_FILE2 = CGRA_SRCDIR "/res/assets/b.obj";
    cgra::gl_mesh tree2 = cgra::load_wavefront_data(TREE_FILE2).build();
    const std::string LEAVES_FILE2 = CGRA_SRCDIR "/res/assets/bl.obj";
    cgra::gl_mesh leaves2 = cgra::load_wavefront_data(LEAVES_FILE2).build();

    const std::string LOD_TARGET_FILE = CGRA_SRCDIR "/res/assets/sphere.obj";
    cgra::gl_mesh m_target = cgra::load_wavefront_data(LOD_TARGET_FILE).build();
    glm::vec3 m_target_position = {0.f,0.f,0.f};

    std::vector<glm::vec3> tree_positions;
    std::vector<float> tree_rotations; // Store rotation for each tree
    void create_tree(const glm::vec3& pos, float rotation); // Accept rotation



public:
    void update_lod(const glm::mat4 &view, glm::mat4 &proj);
    int get_lod_level(glm::vec3 model_position) const;
    void generate_trees(const HeightmapGenerator& m_terrain);
    void draw_lod_target(const glm::mat4 &view, const glm::mat4 &proj);

    std::vector<float> lod_thresholds; // Vector of thresholds that determine the distance lod level will change
    GLuint shader = 0;
    float max_height = 0;
    bool m_draw_lod_visualize = false;
    bool m_move_target = false;
};

} // lod

#endif //LEVEL_OF_DETAIL_H
