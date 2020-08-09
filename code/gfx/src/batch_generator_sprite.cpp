#include <gfx/batch_generators.hpp>
#include <utility/msgstream.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <vector>
#include <array>

namespace gfx {


batch  create_sprite(image_rgba_8888 const&  img)
{
    static std::vector< std::array<float_32_bit, 3> > const vertices{
        { -1.0f, -1.0f, -1.0f }, {  1.0f, -1.0f, -1.0f }, {  1.0f,  1.0f, -1.0f },
        { -1.0f, -1.0f, -1.0f }, {  1.0f,  1.0f, -1.0f }, { -1.0f,  1.0f, -1.0f }
    };
    static std::vector< std::array<float_32_bit, 2> > const texcoords{
        { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f },
        { 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }
    };

    texture_file const  texture_props("", GL_RGBA, GL_REPEAT, GL_REPEAT, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);
    texture_image  image_props(img.width, img.height, img.data.data(), img.data.data() + img.data.size(), GL_RGBA, GL_UNSIGNED_BYTE);

    return create_triangle_mesh(vertices, texcoords, texture(0U, texture_props, image_props), true);
}


texture  get_sprite_texture(batch const  sprite_batch)
{
    return sprite_batch.get_textures_binding().bindings_map().at(FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE);
}

void  update_sprite(batch const  sprite_batch, image_rgba_8888 const&  img)
{
    texture const  tex = get_sprite_texture(sprite_batch);
    ASSUMPTION(tex.width() == img.width && tex.height() == img.height);

    if (tex.id() == 0U)
        tex.create_gl_image();

    glBindTexture(GL_TEXTURE_2D, tex.id());
    INVARIANT(glGetError() == 0U);

    glTexImage2D(GL_TEXTURE_2D, 0,
        tex.pixel_format(),
        tex.width(), tex.height(),
        0,
        tex.pixel_format(), tex.pixel_components_type(),
        img.data.data()
        );
    INVARIANT(glGetError() == 0U);
}


}
