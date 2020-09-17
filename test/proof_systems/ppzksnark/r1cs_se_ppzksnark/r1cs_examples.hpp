//---------------------------------------------------------------------------//
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2020 Nikita Kaskov <nbering@nil.foundation>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//
// @file Declaration of interfaces for a R1CS example, as well as functions to sample
// R1CS examples with prescribed parameters (according to some distribution).
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_ZK_R1CS_EXAMPLES_HPP_
#define CRYPTO3_ZK_R1CS_EXAMPLES_HPP_

#include <nil/crypto3/zk/snark/relations/constraint_satisfaction_problems/r1cs/r1cs.hpp>

namespace nil {
    namespace crypto3 {
        namespace zk {
            namespace snark {

                /**
                 * A R1CS example comprises a R1CS constraint system, R1CS input, and R1CS witness.
                 */
                template<typename FieldType>
                struct r1cs_example {
                    r1cs_constraint_system<FieldType> constraint_system;
                    r1cs_primary_input<FieldType> primary_input;
                    r1cs_auxiliary_input<FieldType> auxiliary_input;

                    r1cs_example<FieldType>() = default;
                    r1cs_example<FieldType>(const r1cs_example<FieldType> &other) = default;
                    r1cs_example<FieldType>(const r1cs_constraint_system<FieldType> &constraint_system,
                                         const r1cs_primary_input<FieldType> &primary_input,
                                         const r1cs_auxiliary_input<FieldType> &auxiliary_input) :
                        constraint_system(constraint_system),
                        primary_input(primary_input), auxiliary_input(auxiliary_input) {};
                    r1cs_example<FieldType>(r1cs_constraint_system<FieldType> &&constraint_system,
                                         r1cs_primary_input<FieldType> &&primary_input,
                                         r1cs_auxiliary_input<FieldType> &&auxiliary_input) :
                        constraint_system(std::move(constraint_system)),
                        primary_input(std::move(primary_input)), auxiliary_input(std::move(auxiliary_input)) {};
                };

                /**
                 * Generate a R1CS example such that:
                 * - the number of constraints of the R1CS constraint system is num_constraints;
                 * - the number of variables of the R1CS constraint system is (approximately) num_constraints;
                 * - the number of inputs of the R1CS constraint system is num_inputs;
                 * - the R1CS input consists of ``full'' field elements (typically require the whole log|Field| bits to
                 * represent).
                 */
                template<typename FieldType>
                r1cs_example<FieldType> generate_r1cs_example_with_field_input(std::size_t num_constraints, std::size_t num_inputs);

                /**
                 * Generate a R1CS example such that:
                 * - the number of constraints of the R1CS constraint system is num_constraints;
                 * - the number of variables of the R1CS constraint system is (approximately) num_constraints;
                 * - the number of inputs of the R1CS constraint system is num_inputs;
                 * - the R1CS input consists of binary values (as opposed to ``full'' field elements).
                 */
                template<typename FieldType>
                r1cs_example<FieldType> generate_r1cs_example_with_binary_input(std::size_t num_constraints, std::size_t num_inputs);

                template<typename FieldType>
                r1cs_example<FieldType> generate_r1cs_example_with_field_input(std::size_t num_constraints, std::size_t num_inputs) {
                    std::cout << "Call to generate_r1cs_example_with_field_input" << std::endl;

                    assert(num_inputs <= num_constraints + 2);

                    r1cs_constraint_system<FieldType> cs;
                    cs.primary_input_size = num_inputs;
                    cs.auxiliary_input_size = 2 + num_constraints - num_inputs;    // TODO: explain this

                    r1cs_variable_assignment<FieldType> full_variable_assignment;
                    typename FieldType::value_type a = random_element<FieldType>();
                    typename FieldType::value_type b = random_element<FieldType>();
                    full_variable_assignment.push_back(a);
                    full_variable_assignment.push_back(b);

                    for (std::size_t i = 0; i < num_constraints - 1; ++i) {
                        linear_combination<FieldType> A, B, C;

                        if (i % 2) {
                            // a * b = c
                            A.add_term(i + 1, 1);
                            B.add_term(i + 2, 1);
                            C.add_term(i + 3, 1);
                            typename FieldType::value_type tmp = a * b;
                            full_variable_assignment.push_back(tmp);
                            a = b;
                            b = tmp;
                        } else {
                            // a + b = c
                            B.add_term(0, 1);
                            A.add_term(i + 1, 1);
                            A.add_term(i + 2, 1);
                            C.add_term(i + 3, 1);
                            typename FieldType::value_type tmp = a + b;
                            full_variable_assignment.push_back(tmp);
                            a = b;
                            b = tmp;
                        }

                        cs.add_constraint(r1cs_constraint<FieldType>(A, B, C));
                    }

                    linear_combination<FieldType> A, B, C;
                    typename FieldType::value_type fin = typename FieldType::value_type::zero();
                    for (std::size_t i = 1; i < cs.num_variables(); ++i) {
                        A.add_term(i, 1);
                        B.add_term(i, 1);
                        fin = fin + full_variable_assignment[i - 1];
                    }
                    C.add_term(cs.num_variables(), 1);
                    cs.add_constraint(r1cs_constraint<FieldType>(A, B, C));
                    full_variable_assignment.push_back(fin.squared());

                    /* split variable assignment */
                    r1cs_primary_input<FieldType> primary_input(full_variable_assignment.begin(),
                        full_variable_assignment.begin() + num_inputs);
                    r1cs_primary_input<FieldType> auxiliary_input(full_variable_assignment.begin() + num_inputs,
                        full_variable_assignment.end());

                    /* sanity checks */
                    assert(cs.num_variables() == full_variable_assignment.size());
                    assert(cs.num_variables() >= num_inputs);
                    assert(cs.num_inputs() == num_inputs);
                    assert(cs.num_constraints() == num_constraints);
                    assert(cs.is_satisfied(primary_input, auxiliary_input));

                    return r1cs_example<FieldType>(std::move(cs), std::move(primary_input), std::move(auxiliary_input));
                }

                template<typename FieldType>
                r1cs_example<FieldType> generate_r1cs_example_with_binary_input(std::size_t num_constraints,
                                                                             std::size_t num_inputs) {
                    std::cout << "Call to generate_r1cs_example_with_binary_input" << std::endl;

                    assert(num_inputs >= 1);

                    r1cs_constraint_system<FieldType> cs;
                    cs.primary_input_size = num_inputs;
                    cs.auxiliary_input_size = num_constraints; /* we will add one auxiliary variable per constraint */

                    r1cs_variable_assignment<FieldType> full_variable_assignment;
                    for (std::size_t i = 0; i < num_inputs; ++i) {
                        full_variable_assignment.push_back(FieldType(std::rand() % 2));
                    }

                    std::size_t lastvar = num_inputs - 1;
                    for (std::size_t i = 0; i < num_constraints; ++i) {
                        ++lastvar;
                        const std::size_t u = (i == 0 ? std::rand() % num_inputs : std::rand() % i);
                        const std::size_t v = (i == 0 ? std::rand() % num_inputs : std::rand() % i);

                        /* chose two random bits and XOR them together:
                           res = u + v - 2 * u * v
                           2 * u * v = u + v - res
                        */
                        linear_combination<FieldType> A, B, C;
                        A.add_term(u + 1, 2);
                        B.add_term(v + 1, 1);
                        if (u == v) {
                            C.add_term(u + 1, 2);
                        } else {
                            C.add_term(u + 1, 1);
                            C.add_term(v + 1, 1);
                        }
                        C.add_term(lastvar + 1, -typename FieldType::value_type::one());

                        cs.add_constraint(r1cs_constraint<FieldType>(A, B, C));
                        full_variable_assignment.push_back(full_variable_assignment[u] + full_variable_assignment[v] -
                                                           full_variable_assignment[u] * full_variable_assignment[v] -
                                                           full_variable_assignment[u] * full_variable_assignment[v]);
                    }

                    /* split variable assignment */
                    r1cs_primary_input<FieldType> primary_input(full_variable_assignment.begin(),
                        full_variable_assignment.begin() + num_inputs);
                    r1cs_primary_input<FieldType> auxiliary_input(full_variable_assignment.begin() + num_inputs,
                        full_variable_assignment.end());

                    /* sanity checks */
                    assert(cs.num_variables() == full_variable_assignment.size());
                    assert(cs.num_variables() >= num_inputs);
                    assert(cs.num_inputs() == num_inputs);
                    assert(cs.num_constraints() == num_constraints);
                    assert(cs.is_satisfied(primary_input, auxiliary_input));

                    return r1cs_example<FieldType>(std::move(cs), std::move(primary_input), std::move(auxiliary_input));
                }

            }    // namespace snark
        }        // namespace zk
    }            // namespace crypto3
}    // namespace nil

#endif    // R1CS_EXAMPLES_HPP_
