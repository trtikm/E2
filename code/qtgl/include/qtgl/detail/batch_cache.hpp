#ifndef QTGL_DETAIL_BATCH_CACHE_HPP_INCLUDED
#   define QTGL_DETAIL_BATCH_CACHE_HPP_INCLUDED

#   include <qtgl/batch.hpp>
#   include <boost/filesystem/path.hpp>
#   include <unordered_map>
#   include <vector>
#   include <tuple>
#   include <memory>
#   include <mutex>

namespace qtgl { namespace detail {


struct batch_cache
{
    static batch_cache&  instance();

    void clear();

    void  insert_load_request(boost::filesystem::path const&  batch_file);
    batch_ptr  find(boost::filesystem::path const&  batch_file);

    void  cached(std::vector<boost::filesystem::path>&  output);
    void  failed(std::vector< std::pair<boost::filesystem::path,std::string> >&  output);

    void  process_pending_batches();

private:
    batch_cache();

    batch_cache(batch_cache const&) = delete;
    batch_cache& operator=(batch_cache const&) = delete;

    void  receiver(boost::filesystem::path const&  path,
                   std::shared_ptr<batch const> const  data,
                   std::string const&  error_message //!< Empty string means no error.
                   );

    std::unordered_map<boost::filesystem::path,   //!< Batch file path-name.
                       std::shared_ptr<batch const>,
                       size_t(*)(boost::filesystem::path const&)
                       >  m_cached_batches;

    std::vector< std::tuple<boost::filesystem::path,std::shared_ptr<batch const>,std::string> >  m_pending_batches;

    std::unordered_map<boost::filesystem::path,
                       std::string,
                       size_t(*)(boost::filesystem::path const&)
                       >  m_failed_loads;

    mutable std::mutex  m_mutex;
};


}}

#endif
