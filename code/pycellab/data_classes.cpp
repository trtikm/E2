#include "./data_classes.hpp"

namespace pytest {

data_holder::data_holder(unsigned int const init_value)
    : m_data(init_value)
{}

unsigned int data_holder::get_data() const
{
    return m_data;
}

void data_holder::set_data(unsigned int const value)
{
    m_data = value;
}

holder_caller::holder_caller(data_holder* const holder_ptr)
    : m_holder_ptr(holder_ptr)
{}

unsigned int holder_caller::get_data() const
{
    return m_holder_ptr->get_data();
}
void holder_caller::set_data(unsigned int const value)
{
    m_holder_ptr->set_data(value);
}

unsigned int get_data(data_holder const& holder)
{
    return holder.get_data();
}

void set_data(data_holder& holder, unsigned int const value)
{
    holder.set_data(value);
}

}
