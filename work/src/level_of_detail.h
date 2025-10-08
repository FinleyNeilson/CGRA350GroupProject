//
// Created by Fletcher on 7/10/2025.
//

#ifndef LEVEL_OF_DETAIL_H
#define LEVEL_OF_DETAIL_H
#include <glm/glm.hpp>
#include <vector>

#include "cgra/cgra_mesh.hpp"
#include "cgra/cgra_wavefront.hpp"

namespace lod {

class level_of_detail {
private:
    // TODO use this value
    glm::vec3 viewer_position = {0,0,0}; // Position of where the LOD distance is calculated from

    const std::string TREE_FILE = CGRA_SRCDIR "/res/assets/t1.obj";
    cgra::gl_mesh tree1_lod0 = cgra::load_wavefront_data(TREE_FILE).build();

    //const std::string TREE_FILE2 = CGRA_SRCDIR "/res/assets/tree1_lod2.obj";
    //cgra::gl_mesh tree1_lod2 = cgra::load_wavefront_data(TREE_FILE2).build();

    std::vector<glm::vec3> tree_positions;



public:
    void update_lod(const glm::mat4 &view, glm::mat4 &proj);
    int get_lod_level(glm::vec3 model_position) const;
    void create_tree(const glm::vec3& pos);

    std::vector<float> lod_thresholds; // Vector of thresholds that determine the distance lod level will change
    GLuint shader = 0;
};

} // lod

#endif //LEVEL_OF_DETAIL_H
