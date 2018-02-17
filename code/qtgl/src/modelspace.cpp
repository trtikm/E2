#include <qtgl/modelspace.hpp>

namespace qtgl {


void  apply_modelspace_to_frame_of_keyframe_animation(
        modelspace const&  modelspace,
        std::vector<matrix44>&  frame  // the results will be composed with the current data.
        )
{
    ASSUMPTION(modelspace.loaded_successfully());
    ASSUMPTION(frame.size() == modelspace.get_coord_systems().size());
    for (std::size_t i = 0UL; i != modelspace.get_coord_systems().size(); ++i)
    {
        matrix44 M;
        angeo::to_base_matrix(modelspace.get_coord_systems().at(i), M);
        frame.at(i) *= M;
    }
}


}
