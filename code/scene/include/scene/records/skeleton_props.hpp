#ifndef E2_SCENE_RECORDS_SKELETON_PROPS_HPP_INCLUDED
#   define E2_SCENE_RECORDS_SKELETON_PROPS_HPP_INCLUDED

#   include <ai/skeletal_motion_templates.hpp>
#   include <boost/filesystem/path.hpp>
#   include <memory>

namespace scn {


struct  skeleton_props  final
{
    boost::filesystem::path  skeleton_directory;
    ai::skeletal_motion_templates  skeletal_motion_templates;
};


using  skeleton_props_ptr = std::shared_ptr<skeleton_props>;
using  skeleton_props_const_ptr = std::shared_ptr<skeleton_props const>;


inline skeleton_props_ptr  create_skeleton_props(
        boost::filesystem::path const&  skeleton_dir,
        ai::skeletal_motion_templates const  skeletal_motion_templates
        )
{
    skeleton_props_ptr const  props = std::make_shared<skeleton_props>();
    {
        props->skeleton_directory = skeleton_dir;
        props->skeletal_motion_templates = skeletal_motion_templates;
    }
    return props;
}


}

#endif
