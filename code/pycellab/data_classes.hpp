#ifndef PYTEST_DATA_CLASSES_HPP_INCLUDED
#   define PYTEST_DATA_CLASSES_HPP_INCLUDED

namespace pytest {

struct data_holder
{
    data_holder(unsigned int const init_value);
    unsigned int get_data() const;
    void set_data(unsigned int const value);
private:
    unsigned int m_data;
};

struct holder_caller
{
    holder_caller(data_holder* const holder_ptr);
    unsigned int get_data() const;
    void set_data(unsigned int const value);
private:
    data_holder* m_holder_ptr;
};

unsigned int get_data(data_holder const& holder);
void set_data(data_holder& holder, unsigned int const value);

}

#endif
