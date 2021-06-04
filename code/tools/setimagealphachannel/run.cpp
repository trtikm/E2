#include <setimagealphachannel/program_info.hpp>
#include <setimagealphachannel/program_options.hpp>
#include <angeo/tensor_math.hpp>
#include <gfx/image.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/msgstream.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>


void run(int argc, char* argv[])
{
    TMPROF_BLOCK();


    if (!get_program_options()->has_input_image() || !get_program_options()->has_output_image())
    {
        std::cout << "Wrong usage of the tool. Use the command --help." << std::endl;
        return;
    }
    if (!boost::filesystem::is_regular_file(get_program_options()->get_input_image()))
    {
        std::cout << "Cannot acces the input image '" << get_program_options()->get_input_image()  << "'" << std::endl;
        return;
    }

    try
    {
        gfx::image_rgba_8888  img;
        gfx::load_png_image(boost::filesystem::absolute(get_program_options()->get_input_image()), img);
        for (natural_32_bit  i = 0U, n = (natural_32_bit)img.data.size(); i < n; i += 4)
        {
            img.data.at(i + 3U) = img.data.at(i + 0U);
            img.data.at(i + 0U) = 255U;
            img.data.at(i + 1U) = 255U;
            img.data.at(i + 2U) = 255U;
        }
        gfx::save_png_image(boost::filesystem::absolute(get_program_options()->get_output_image()), img);
    }
    catch (std::exception const& e)
    {
        std::cout << "Failed to convert the input image "
                  << "'" << get_program_options()->get_input_image() << "'"
                  << " to output image "
                  << "'" << get_program_options()->get_output_image() << "'"
                  << ". Details:\n"
                  << e.what() << std::endl;
        LOG(error, e.what());
        return;
    }
}
