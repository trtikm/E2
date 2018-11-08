#ifndef E2_SCENE_SCENE_HISTORY_HPP_INCLUDED
#   define E2_SCENE_SCENE_HISTORY_HPP_INCLUDED

#   include <scene/scene.hpp>
#   include <scene/scene_history_node.hpp>
#   include <scene/scene_history_nodes_default.hpp>
#   include <scene/scene_history_nodes_specialised.hpp>
#   include <scene/scene_record_id.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/std_pair_hash.hpp>
#   include <boost/filesystem/path.hpp>
#   include <memory>
#   include <string>
#   include <vector>
#   include <functional>

namespace scn {


struct  scene_history final
{
    scene_history();

    void  insert(scene_history_node_ptr const  node_ptr);

    template<typename scene_history_node_type, typename... arg_types>
    void  insert(arg_types... args_for_constructor_of_history_node)
    {
        insert(scene_history_node_ptr(new scene_history_node_type(args_for_constructor_of_history_node...)));
    }

    void  commit();
    natural_64_bit  get_active_commit_id() const;
    bool  is_commit_valid(natural_64_bit const  commit_id) const;
    bool  was_applied_mutator_since_commit(natural_64_bit const  commit_id) const;

    void  undo();
    void  redo();

    void  clear();
    bool  empty() const { return m_active_commit == 0UL; }

private:

    /// Returns the index of the commit in 'm_history' vector.
    std::size_t  find_commit(natural_64_bit const  commit_id) const;

    std::vector<scene_history_node_ptr>  m_history;
    std::size_t  m_active_commit;
};

using scene_history_ptr = std::shared_ptr<scene_history>;
using scene_history_const_ptr = std::shared_ptr<scene_history const>;


inline constexpr natural_64_bit  get_invalid_scene_history_commit_id() noexcept { return 0ULL; }


}

#endif
