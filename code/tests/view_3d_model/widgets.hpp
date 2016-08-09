#ifndef WIDGETS_HPP_INCLUDED
#   define WIDGETS_HPP_INCLUDED

#include "./simulator.hpp"
#include <qtgl/widget_base.hpp>
#include <utility/assumptions.hpp>
#include <QLineEdit>
#include <QIntValidator>
#include <sstream>
#include <iostream>


//struct edit_clear_colour_component : public QLineEdit {
//    edit_clear_colour_component(simulator_ptr const sim,
//                                natural_8_bit const  component_index)
//        : QLineEdit()
//        , m_simulator(sim)
//        , m_component_index(component_index)
//    {
//        ASSUMPTION(m_simulator != nullptr);
//        setValidator( new QIntValidator(0,255) );
//        std::stringstream sstr;
//        sstr << (natural_32_bit)(m_simulator->clear_color()(m_component_index) * 255.0f + 0.5f);
//        setText(sstr.str().c_str());
//        QObject::connect(this,SIGNAL(editingFinished()),this,SLOT(editingFinishedSlot()));
//    }
//public slots:
//    void editingFinishedSlot()
//    {
//        vector3  colour = m_simulator->clear_color();
//        colour(m_component_index) = (float_32_bit)text().toInt() / 255.0f;
//        m_simulator->set_clear_color(colour);
//    }
//private:
//    Q_OBJECT
//    simulator_ptr  m_simulator;
//    natural_8_bit  m_component_index;
//};


#endif
