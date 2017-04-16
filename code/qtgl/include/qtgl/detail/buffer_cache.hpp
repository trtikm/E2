#ifndef QTGL_DETAIL_BUFFER_CACHE_HPP_INCLUDED
#   define QTGL_DETAIL_BUFFER_CACHE_HPP_INCLUDED

#   include <qtgl/buffer.hpp>
#   include <boost/filesystem/path.hpp>
#   include <unordered_map>
#   include <vector>
#   include <memory>
#   include <mutex>

namespace qtgl { namespace detail {


struct buffer_cache
{
    using  buffer_data_ptr = std::shared_ptr<std::vector<natural_8_bit> const>;
    using  buffer_load_info = std::tuple<buffer_properties_ptr,     //!< Properties of the loaded buffer's data
                                         buffer_data_ptr,           //!< Raw data of the buffer.
                                         std::string                //!< Error message. Empty string means no error.
                                         >;
    using  failed_loads_map = std::unordered_map<boost::filesystem::path, buffer_load_info,
                                                 size_t(*)(boost::filesystem::path const&) >;

    static buffer_cache&  instance();

    void clear();

    void  insert_load_request(boost::filesystem::path const&  buffer_file);
    std::weak_ptr<buffer const>  find(boost::filesystem::path const&  buffer_file);

    void  cached(std::vector<buffer_properties_ptr>&  output);
    void  failed(std::vector< std::pair<buffer_properties_ptr,std::string> >&  output);

    void  process_pending_buffers();

private:
    buffer_cache();

    buffer_cache(buffer_cache const&) = delete;
    buffer_cache& operator=(buffer_cache const&) = delete;

    void  receiver(buffer_properties_ptr const  props,
                   buffer_data_ptr const  data,
                   boost::filesystem::path const&  path,
                   std::string const&  error_message //!< Empty string means no error.
                   );

    std::unordered_map<boost::filesystem::path,   //!< Shader file path-name.
                       buffer_ptr,
                       size_t(*)(boost::filesystem::path const&)
                       >  m_cached_buffers;

    std::vector<buffer_load_info>  m_pending_buffers;

    std::unordered_map<boost::filesystem::path,
                       buffer_load_info,
                       size_t(*)(boost::filesystem::path const&)
                       >  m_failed_loads;

    mutable std::mutex  m_mutex;
};


}}

#endif
