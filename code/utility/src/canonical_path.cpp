#include <utility/canonical_path.hpp>
#include <utility/timeprof.hpp>
#include <filesystem>


std::filesystem::path  canonical_path(std::filesystem::path const&  path)
{
    TMPROF_BLOCK();
    if (!std::filesystem::exists(path))
        return std::filesystem::absolute(path);
    return std::filesystem::canonical(path);
}
