#include <qtgl/shader_data_linkers.hpp>
#include <utility/invariants.hpp>
#include <utility/assumptions.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


void  compose_skeleton_binding_data_with_frame_of_keyframe_animation(
        modelspace const  pose,
        skeleton_alignment const  alignment,
        std::vector<matrix44> const&  frame,
        std::vector<matrix44>&  output
        )
{
    ASSUMPTION(pose.loaded_successfully());
    ASSUMPTION(alignment.loaded_successfully());
    ASSUMPTION(frame.size() == pose.get_coord_systems().size());

    output.resize(frame.size());

    matrix44 A;
    angeo::from_base_matrix(alignment.get_skeleton_alignment(), A);
    for (std::size_t i = 0UL; i != pose.get_coord_systems().size(); ++i)
    {
        matrix44 M;
        angeo::to_base_matrix(pose.get_coord_systems().at(i), M);
        output.at(i) = frame.at(i) * M * A;
    }
}


void  compose_skeleton_binding_data_with_frame_of_keyframe_animation(
        modelspace const  modelspace,
        skeleton_alignment const  alignment,
        std::vector<matrix44>&  frame  // the results will be composed with the current data.
        )
{
    ASSUMPTION(modelspace.loaded_successfully());
    ASSUMPTION(alignment.loaded_successfully());
    ASSUMPTION(frame.size() == modelspace.get_coord_systems().size());

    matrix44 A;
    angeo::from_base_matrix(alignment.get_skeleton_alignment(), A);
    for (std::size_t i = 0UL; i != modelspace.get_coord_systems().size(); ++i)
    {
        matrix44 M;
        angeo::to_base_matrix(modelspace.get_coord_systems().at(i), M);
        M *= A;
        frame.at(i) *= M;
    }
}


}

namespace qtgl {


vertex_shader_uniform_data_provider::vertex_shader_uniform_data_provider(
        batch const  batch_,
        std::vector<matrix44> const&  transformations,
        vector4 const&  diffuse_colour
        )
    : vertex_shader_uniform_data_provider_base(batch_)
    , m_transformations()
    , m_diffuse_colour(diffuse_colour)
{
    if (get_batch().is_attached_to_skeleton())
    {
        if (transformations.size() == 1UL)
            m_transformations.resize(get_batch().get_modelspace().get_coord_systems().size(), transformations.front());
        else
            compose_skeleton_binding_data_with_frame_of_keyframe_animation(
                    get_batch().get_modelspace(),
                    get_batch().get_skeleton_alignment(),
                    transformations,
                    m_transformations
                    );
    }
    else
    {
        ASSUMPTION(transformations.size() == 1UL);
        m_transformations.push_back(transformations.front());
    }
}


}
