#include <utility/canonical_path.hpp>
#include <boost/filesystem.hpp>


boost::filesystem::path  canonical_path(boost::filesystem::path const&  path)
{
    if (!boost::filesystem::exists(path))
        return boost::filesystem::absolute(path);
    return boost::filesystem::canonical(path);
}
