#ifndef EFLOOP_EXTERNAL_FEEDBACK_LOOP_HPP_INCLUDED
#   define EFLOOP_EXTERNAL_FEEDBACK_LOOP_HPP_INCLUDED

#include <cellab/neural_tissue.hpp>
#include <envlab/rules_and_logic_of_environment.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/instance_wrapper.hpp>
#include <utility/assumptions.hpp>
#include <utility>
#include <memory>

namespace efloop {


struct external_feedback_loop
{
    external_feedback_loop(
            std::shared_ptr<cellab::neural_tissue> neural_tissue,
            std::shared_ptr<envlab::rules_and_logic_of_environment> rules_and_logic_of_environment
            );

    std::shared_ptr<cellab::neural_tissue> get_neural_tissue();
    std::shared_ptr<cellab::neural_tissue const> get_neural_tissue() const;

    std::shared_ptr<envlab::rules_and_logic_of_environment> get_rules_and_logic_of_environment();
    std::shared_ptr<envlab::rules_and_logic_of_environment const> get_rules_and_logic_of_environment() const;

    void compute_next_state_of_both_neural_tissue_and_environment(
            natural_32_bit max_number_of_threads_neural_tissue_can_create_and_run_simultaneously,
            natural_32_bit max_number_of_threads_environment_can_create_and_run_simultaneously
            );

private:
    std::shared_ptr<cellab::neural_tissue> m_neural_tissue;
    std::shared_ptr<envlab::rules_and_logic_of_environment> m_rules_and_logic_of_environment;
};


}

#endif
