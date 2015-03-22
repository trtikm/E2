#ifndef EFLOOP_EXTERNAL_FEEDBACK_LOOP_HPP_INCLUDED
#   define EFLOOP_EXTERNAL_FEEDBACK_LOOP_HPP_INCLUDED

#   include <cellab/neural_tissue.hpp>
#   include <envlab/rules_and_logic_of_environment.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <boost/noncopyable.hpp>
#   include <vector>
#   include <memory>

namespace efloop {


struct external_feedback_loop : private boost::noncopyable
{
    external_feedback_loop(
            std::vector< std::shared_ptr<cellab::neural_tissue> > const&  neural_tissues,
            std::shared_ptr<envlab::rules_and_logic_of_environment>  rules_and_logic_of_environment
            );

    natural_32_bit num_neural_tissues() const;
    std::shared_ptr<cellab::neural_tissue> get_neural_tissue(natural_32_bit const index);
    std::shared_ptr<cellab::neural_tissue const> get_neural_tissue(natural_32_bit const index) const;

    std::shared_ptr<envlab::rules_and_logic_of_environment> get_rules_and_logic_of_environment();
    std::shared_ptr<envlab::rules_and_logic_of_environment const> get_rules_and_logic_of_environment() const;

    /**
     * @param num_threads_avalilable_for_computation_of_neural_tissues
     *      We assume num_threads_avalilable_for_computation_of_neural_tissues.size()==this->num_neural_tissues().
     *      If num_threads_avalilable_for_computation_of_neural_tissues[i]==0, then
     *      this->get_neural_tissue(i) will only be updated on the thread calling the method.
     *      Otherwise, this->get_neural_tissue(i) will not be updated on the calling thread, but rather on
     *      at most num_threads_avalilable_for_computation_of_neural_tissues[i] newly created threads.
     * @param num_threads_avalilable_for_computation_of_environment
     *      We assume num_threads_avalilable_for_computation_of_environment > 0.
     *      If num_threads_avalilable_for_computation_of_environment==1, then
     *      this->get_rules_and_logic_of_environment() will only be updated on the thread calling the method.
     *      If num_threads_avalilable_for_computation_of_environment!=1 and there exists i s.t.
     *      num_threads_avalilable_for_computation_of_neural_tissues[i]==0, then
     *      this->get_rules_and_logic_of_environment() will not be updated on the calling thread, but rather on
     *      at most num_threads_avalilable_for_computation_of_environment newly created threads.
     *      Otherwise, this->get_rules_and_logic_of_environment() will be updated on the calling thread and also
     *      on at most num_threads_avalilable_for_computation_of_environment threads-1 newly created threads.
     */
    void compute_next_state_of_neural_tissues_and_environment(
            std::vector<natural_32_bit> const&  num_threads_avalilable_for_computation_of_neural_tissues,
            natural_32_bit const  num_threads_avalilable_for_computation_of_environment
            );

private:
    std::vector< std::shared_ptr<cellab::neural_tissue> > m_neural_tissues;
    std::shared_ptr<envlab::rules_and_logic_of_environment> m_rules_and_logic_of_environment;
};


}

#endif
