bool is_this_big_endian_machine()
{
    unsigned int const value = 1U;
    return *reinterpret_cast<unsigned char const*>(&value) == 1;
}
