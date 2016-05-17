#ifndef QTGL_DETAIL_FRAGMENT_PROGRAM_CACHE_HPP_INCLUDED
#   define QTGL_DETAIL_FRAGMENT_PROGRAM_CACHE_HPP_INCLUDED

#   include <qtgl/shader.hpp>
#   include <boost/filesystem/path.hpp>
#   include <unordered_map>
#   include <vector>
#   include <memory>
#   include <mutex>
#   include <tuple>

namespace qtgl { namespace detail {


struct fragment_program_cache
{
    using  source_code_lines_ptr = std::shared_ptr<std::vector<std::string> const>;
    using  program_load_info = std::tuple<boost::filesystem::path,  //!< Shader file path-name.
                                          source_code_lines_ptr,    //!< Lines of the shader's code.
                                          std::string               //!< Error message. Empty string means no error.
                                          >;
    using  failed_loads_map = std::unordered_map<boost::filesystem::path, program_load_info,
                                                 size_t(*)(boost::filesystem::path const&) >;

    static fragment_program_cache&  instance();

    void clear(bool const  props_to_files = false, bool const destroy_also_dummy_program = false);

    void  insert_load_request(boost::filesystem::path const&  shader_file);
    bool  insert_load_request(fragment_program_properties_ptr const  props);
    bool  insert(boost::filesystem::path const&  shader_file, fragment_program_ptr const  program);

    std::weak_ptr<fragment_program const>  find(boost::filesystem::path const&  shader_file);
    std::weak_ptr<fragment_program const>  find(fragment_program_properties_ptr const  props);
    std::weak_ptr<fragment_program const>  get_dummy_program() const noexcept { return m_dummy_program; }

    bool  associate_properties_with_pathname(fragment_program_properties_ptr const  props,
                                             boost::filesystem::path const&  shader_file);

    boost::filesystem::path  find_shader_file(fragment_program_properties_ptr const  props) const;

private:
    fragment_program_cache();

    fragment_program_cache(fragment_program_cache const&) = delete;
    fragment_program_cache& operator=(fragment_program_cache const&) = delete;

    void  receiver(boost::filesystem::path const&  shader_file,
                   source_code_lines_ptr const  source_code_lines,
                   std::string const&  error_message //!< Empty string means no error.
                   );

    void  process_pending_programs();

    std::unordered_map<boost::filesystem::path,   //!< Shader file path-name.
                       fragment_program_ptr,
                       size_t(*)(boost::filesystem::path const&)
                       >  m_cached_programs;

    std::vector<program_load_info>  m_pending_programs;

    std::unordered_map<
            fragment_program_properties_ptr,
            boost::filesystem::path,
            size_t(*)(fragment_program_properties_ptr const),
            bool(*)(fragment_program_properties_ptr const,fragment_program_properties_ptr const)
            >  m_props_to_pathnames;

    std::unordered_map<boost::filesystem::path,
                       program_load_info,
                       size_t(*)(boost::filesystem::path const&)
                       >  m_failed_loads;

    mutable std::mutex  m_mutex;

    fragment_program_ptr  m_dummy_program;


//    fragment_program_cache(natural_32_bit const  max_num_programs,
//                         natural_32_bit const  max_summary_size_of_all_programs);
//    struct priority
//    {
//    };
//    std::vector< std::pair<priority,program_pathname> >  m_heap_of_priorities;
//    natural_32_bit  m_summary_size_of_all_programs;
//    natural_32_bit  m_max_num_programs;
//    natural_32_bit  m_max_summary_size_of_all_programs;

};


}}

#endif
