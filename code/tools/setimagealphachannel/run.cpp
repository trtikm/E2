#include <setimagealphachannel/program_info.hpp>
#include <setimagealphachannel/program_options.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/msgstream.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <QImage>


static void  load_file(std::string const&  pathname, std::vector<natural_8_bit>&  buffer)
{
    TMPROF_BLOCK();

    buffer.resize(boost::filesystem::file_size(pathname));
    boost::filesystem::ifstream  istr(pathname, std::ios_base::binary);
    if (!istr.good())
        throw std::runtime_error(msgstream() << "Cannot open the passed image file: " << pathname);
    istr.read((char*)& buffer.at(0U), buffer.size());
    if (istr.bad())
        throw std::runtime_error(msgstream() << "Cannot read the passed image file: " << pathname);
}


static QImage  load_image(std::vector<natural_8_bit> const&  buffer)
{
    TMPROF_BLOCK();

    QImage  qimage;
    if (!qimage.loadFromData(&buffer.at(0),(int)buffer.size()))
        throw std::runtime_error("Qt function QImage::loadFromData() has failed.");

    QImage::Format const  desired_format = qimage.hasAlphaChannel() ? QImage::Format_RGBA8888 : QImage::Format_RGB888;
    if (qimage.format() != desired_format)
    {
        TMPROF_BLOCK();

        // We subclass QImage, because its method 'convertToFormat_helper' is protected.
        struct format_convertor : public QImage
        {
            format_convertor(QImage const& orig, QImage::Format const  desired_format)
                : QImage(orig)
                , m_desired_format(desired_format)
            {}
            QImage  operator()() const
            {
                return convertToFormat_helper(m_desired_format,Qt::AutoColor);
            }
        private:
            QImage::Format  m_desired_format;
        };
        qimage = format_convertor(qimage, desired_format)();
    }

    return qimage;
}


static QImage  merge_colour_with_alpha_image(QImage const  colour_image, QImage const  alpha_image)
{
    TMPROF_BLOCK();

    ASSUMPTION(colour_image.width() == alpha_image.width() && colour_image.height() == alpha_image.height());

    QImage  resulting_image(colour_image.width(), colour_image.height(), QImage::Format_RGBA8888);

    for (int x = 0; x != resulting_image.width(); ++x)
        for (int y = 0; y != resulting_image.height(); ++y)
        {
            QColor const  c = colour_image.pixelColor(x, y);
            QColor const  a = alpha_image.pixelColor(x, y);
            QColor const  r(c.red(), c.green(), c.blue(), (a.red() + a.green() + a.blue()) / 3);
            resulting_image.setPixelColor(x, y, r);
        }

    return resulting_image;
}


void run(int argc, char* argv[])
{
    TMPROF_BLOCK();

    if (!get_program_options()->has_colour_input_image() ||
        !get_program_options()->has_alpha_input_image() ||
        !get_program_options()->has_resulting_image())
    {
        std::cout << "Wrong use of the tool. Use the command --help." << std::endl;
        return;
    }
    if (!boost::filesystem::is_regular_file(get_program_options()->get_colour_input_image()))
    {
        std::cout << "Cannot acces the colour image '"
                  << get_program_options()->get_colour_input_image()  << "'" << std::endl;
        return;
    }
    if (!boost::filesystem::is_regular_file(get_program_options()->get_alpha_input_image()))
    {
        std::cout << "Cannot acces the alpha image '"
                  << get_program_options()->get_alpha_input_image()  << "'" << std::endl;
        return;
    }
    QImage  colour_image;
    try
    {
        std::vector<natural_8_bit>  buffer;
        load_file(get_program_options()->get_colour_input_image(), buffer);
        colour_image = load_image(buffer);
    }
    catch (std::exception const& e)
    {
        std::cout << "Load of colour image '" << get_program_options()->get_colour_input_image() << "'has FAILED. Details:\n"
                  << e.what() << std::endl;
        return;
    }

    QImage  alpha_image;
    try
    {
        std::vector<natural_8_bit>  buffer;
        load_file(get_program_options()->get_alpha_input_image(), buffer);
        alpha_image = load_image(buffer);
    }
    catch (std::exception const& e)
    {
        std::cout << "Load of alpha image '" << get_program_options()->get_alpha_input_image() << "'has FAILED. Details:\n"
                  << e.what() << std::endl;
        return;
    }

    if (colour_image.width() != alpha_image.width() || colour_image.height() != alpha_image.height())
    {
        std::cout << "The loaded colour and alpha images have different sizes.\n"
                     "Please, resize the images to have the same width and same height." << std::endl;
        return;
    }

    QImage  result_image;
    try
    {
        result_image = merge_colour_with_alpha_image(colour_image, alpha_image);
    }
    catch (std::exception const& e)
    {
        std::cout << "The merge of colour with alpha image has FAILED. Details:\n"
                  << e.what() << std::endl;
        return;
    }

    try
    {
        result_image.save(get_program_options()->get_resulting_image().c_str());
    }
    catch (std::exception const& e)
    {
        std::cout << "The save of the merged image to file '" << get_program_options()->get_resulting_image() << "'has FAILED. Details:\n"
                  << e.what() << std::endl;
        return;
    }
}
