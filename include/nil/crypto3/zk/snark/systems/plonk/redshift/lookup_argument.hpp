//---------------------------------------------------------------------------//
// Copyright (c) 2021 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2021 Nikita Kaskov <nbering@nil.foundation>
// Copyright (c) 2022 Ilia Shirobokov <i.shirobokov@nil.foundation>
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_ZK_PLONK_REDSHIFT_LOOKUP_ARGUMENT_HPP
#define CRYPTO3_ZK_PLONK_REDSHIFT_LOOKUP_ARGUMENT_HPP

#include <nil/crypto3/math/polynomial/polynomial.hpp>
#include <nil/crypto3/math/polynomial/shift.hpp>
#include <nil/crypto3/math/domains/evaluation_domain.hpp>
#include <nil/crypto3/math/algorithms/make_evaluation_domain.hpp>

#include <nil/crypto3/hash/sha2.hpp>

#include <nil/crypto3/container/merkle/tree.hpp>

#include <nil/crypto3/zk/transcript/fiat_shamir.hpp>
#include <nil/crypto3/zk/commitments/polynomial/lpc.hpp>
#include <nil/crypto3/zk/snark/systems/plonk/redshift/params.hpp>
#include <nil/crypto3/zk/snark/systems/plonk/redshift/detail/redshift_policy.hpp>
#include <nil/crypto3/zk/snark/arithmetization/plonk/lookup_constraint.hpp>

namespace nil {
    namespace crypto3 {
        namespace zk {
            namespace snark {
                template <typename FieldType, typename CommitmentSchemeTypePermutation, typename ParamsType>
                class redshift_lookup_argument {
                    using transcript_hash_type = typename ParamsType::transcript_hash_type;
                    using transcript_type = transcript::fiat_shamir_heuristic_sequential<transcript_hash_type>;
                    using VariableType = plonk_variable<FieldType>;

                    static constexpr std::size_t argument_size = 5;

                    typedef detail::redshift_policy<FieldType, ParamsType> policy_type;
                public:
                    struct prover_lookup_result {
                        std::array<math::polynomial<typename FieldType::value_type>, argument_size> F;
                        math::polynomial<typename FieldType::value_type> input_polynomial;
                        typename CommitmentSchemeTypePermutation::precommitment_type input_precommitment;
                        math::polynomial<typename FieldType::value_type> value_polynomial;
                        typename CommitmentSchemeTypePermutation::precommitment_type value_precommitment;
                        math::polynomial<typename FieldType::value_type> V_L_polynomial;
                        typename CommitmentSchemeTypePermutation::precommitment_type V_L_precommitment;
                    };
                    static inline prover_lookup_result prove_eval(
                        typename policy_type::constraint_system_type &constraint_system,
                        const typename policy_type::preprocessed_public_data_type preprocessed_data,
                        const plonk_assignment_table<FieldType,
                        typename ParamsType::arithmetization_params> &plonk_columns,
                        typename CommitmentSchemeTypePermutation::params_type fri_params,
                        transcript_type &transcript = transcript_type()) {
                        // $/theta = \challenge$
                        typename FieldType::value_type theta = transcript.template challenge<FieldType>();
                        // Construct lookup gates
                        const std::vector<plonk_gate<FieldType, plonk_lookup_constraint<FieldType>>> lookup_gates = constraint_system.lookup_gates();

                        std::shared_ptr<math::evaluation_domain<FieldType>> domain = preprocessed_data.common_data.basic_domain;
                                                
                        std::array<math::polynomial<typename FieldType::value_type>, argument_size> F;

                        std::vector<typename FieldType::value_type> F_compr_input(preprocessed_data.common_data.rows_amount, 0);

                        std::vector<typename FieldType::value_type> F_compr_value(preprocessed_data.common_data.rows_amount, 0);

                        typename FieldType::value_type theta_acc = FieldType::value_type::one();

                        // Construct the input lookup compression and table compression values
                        //TODO: change to new form
                        for (std::size_t i = 0; i < lookup_gates.size(); i++) {
                            for (std::size_t j = 0; j < lookup_gates[i].constraints.size(); j++) {
                                for (math::non_linear_term<VariableType> lookup :lookup_gates[i].constraints[j].lookup_input) {
                                    int k = 0;
                                    std::vector<typename FieldType::value_type> input_assignment;
                                    std::vector<typename FieldType::value_type> value_assignment;
                                    switch (lookup.vars[0].type) {
                                        case VariableType::column_type::witness:
                                            input_assignment = plonk_columns.witness(lookup.vars[0].index);
                                            break;
                                        case VariableType::column_type::public_input:
                                            input_assignment = plonk_columns.public_input(lookup.vars[0].index);
                                            break;
                                        case VariableType::column_type::constant:
                                            input_assignment = plonk_columns.constant(lookup.vars[0].index);
                                            break;
                                    }
                                    switch (lookup_gates[i].constraints[j].lookup_value[k].type) {
                                        case VariableType::column_type::witness:
                                            value_assignment = plonk_columns.witness(lookup_gates[i].constraints[j].lookup_value[k].index);
                                            break;
                                        case VariableType::column_type::public_input:
                                            value_assignment = plonk_columns.public_input(lookup_gates[i].constraints[j].lookup_value[k].index);
                                            break;
                                        case VariableType::column_type::constant:
                                            value_assignment = plonk_columns.constant(lookup_gates[i].constraints[j].lookup_value[k].index);
                                            break;
                                    }
                                    for (std::size_t t = 0; t < preprocessed_data.common_data.rows_amount; t++) {
                                        F_compr_input[t] = F_compr_input[t] + theta_acc * input_assignment[(j + lookup.vars[0].rotation) % input_assignment.size()] * lookup.coeff
                                         * plonk_columns.selector(lookup_gates[i].selector_index)[t];
                                        F_compr_value[t] = F_compr_value[t] + theta_acc * value_assignment[j] * plonk_columns.selector(lookup_gates[i].selector_index)[t];
                                    }
                                    k++;
                                    theta_acc = theta * theta_acc;
                                }
                            }
                        }
                        
                        //Produce the permutation polynomials $S_{\texttt{perm}}(X)$ and $A_{\texttt{perm}}(X)$
                        std::vector<typename FieldType::value_type> F_perm_input = F_compr_input;
                        std::sort(F_perm_input.begin(), F_perm_input.end());
                        //to-do better sort for F_perm_value
                        std::vector<typename FieldType::value_type>  F_perm_value = F_compr_value;
                        F_perm_value[0] = F_perm_input[0];
                        for (std::size_t i = 1; i < preprocessed_data.common_data.rows_amount; i++) {
                            if (F_perm_input[i] != F_perm_input[i - 1]){
                                if (F_perm_input[i] != F_perm_value[i]) {
                                    auto index = std::distance(F_perm_value.begin(), std::find(F_perm_value.begin(), F_perm_value.end(), F_perm_input[i]));
                                    auto tmp = F_perm_value[i];
                                    F_perm_value[i] = F_perm_value[index];
                                    F_perm_value[index] = tmp;
                                }
                            }
                        } 
                        domain->inverse_fft(F_perm_input);

                        math::polynomial<typename FieldType::value_type> F_perm_input_poly(F_perm_input.begin(),
                                                                             F_perm_input.end());

                        domain->inverse_fft(F_perm_value);

                        math::polynomial<typename FieldType::value_type> F_perm_value_poly(F_perm_value.begin(),
                                                                             F_perm_value.end());
                        typename CommitmentSchemeTypePermutation::precommitment_type F_perm_input_tree =
                            CommitmentSchemeTypePermutation::precommit(F_perm_input_poly, fri_params.D[0]);
                        typename CommitmentSchemeTypePermutation::commitment_type F_perm_input_commitment =
                            CommitmentSchemeTypePermutation::commit(F_perm_input_tree);
                        transcript(F_perm_input_commitment);

                        typename CommitmentSchemeTypePermutation::precommitment_type F_perm_value_tree =
                            CommitmentSchemeTypePermutation::precommit(F_perm_value_poly, fri_params.D[0]);
                        typename CommitmentSchemeTypePermutation::commitment_type F_perm_value_commitment =
                            CommitmentSchemeTypePermutation::commit(F_perm_value_tree);
                        transcript(F_perm_value_commitment);

                        //Compute $V_L(X)$
                        typename FieldType::value_type beta = transcript.template challenge<FieldType>();

                        typename FieldType::value_type gamma = transcript.template challenge<FieldType>();
                        std::vector<typename FieldType::value_type> V_L_interpolation_points(preprocessed_data.common_data.rows_amount);

                        V_L_interpolation_points[0] = FieldType::value_type::one();
                        for (std::size_t j = 1; j < preprocessed_data.common_data.rows_amount; j++) {
                            V_L_interpolation_points[j] = (V_L_interpolation_points[j - 1] * (F_compr_input[j - 1] + beta) * (F_compr_value[j - 1] + gamma)) * 
                            ((F_perm_input[j - 1] + beta) * (F_perm_value[j - 1] + gamma)).inversed();
                        }

                        domain->inverse_fft(V_L_interpolation_points);

                        math::polynomial<typename FieldType::value_type> V_L(V_L_interpolation_points.begin(),
                                                                             V_L_interpolation_points.end());
                        
                        typename CommitmentSchemeTypePermutation::precommitment_type V_L_tree =
                            CommitmentSchemeTypePermutation::precommit(V_L, fri_params.D[0]);
                        typename CommitmentSchemeTypePermutation::commitment_type V_L_commitment =
                            CommitmentSchemeTypePermutation::commit(V_L_tree);
                        transcript(V_L_commitment);

                        //Calculate lookup-related numerators of the quotinent polynomial
                        domain->inverse_fft(F_compr_input);

                        math::polynomial<typename FieldType::value_type> F_compr_input_poly(F_compr_input.begin(),
                                                                             F_compr_input.end());

                        domain->inverse_fft(F_compr_value);

                        math::polynomial<typename FieldType::value_type> F_compr_value_poly(F_compr_value.begin(),
                                                                             F_compr_value.end());
                        math::polynomial<typename FieldType::value_type> g = (F_compr_input_poly + beta) * (F_compr_value_poly + gamma);
                        math::polynomial<typename FieldType::value_type> h = (F_perm_input_poly + beta) * (F_perm_value_poly + gamma);
                        math::polynomial<typename FieldType::value_type> one_polynomial = {1};

                        math::polynomial<typename FieldType::value_type> V_L_shifted =
                            math::polynomial_shift<FieldType>(V_L, domain->get_domain_element(1));

                        math::polynomial<typename FieldType::value_type> F_perm_input_poly_shifted =
                            math::polynomial_shift<FieldType>(F_perm_input_poly, domain->get_domain_element(-1));


                        F[0] = preprocessed_data.common_data.lagrange_0 * (one_polynomial - V_L);
                        F[1] = (one_polynomial - (preprocessed_data.q_last + preprocessed_data.q_blind))* (V_L_shifted * h - V_L * g);
                        F[2] = preprocessed_data.q_last * (V_L*V_L - V_L);
                        F[3] = preprocessed_data.common_data.lagrange_0 * (F_perm_input_poly - F_perm_value_poly);
                        F[4] = (one_polynomial - (preprocessed_data.q_last + preprocessed_data.q_blind))* (F_perm_input_poly - F_perm_value_poly)*(F_perm_input_poly - F_perm_input_poly_shifted);

                        prover_lookup_result res = {F, F_perm_input_poly, F_perm_input_tree, F_perm_value_poly, F_perm_value_tree, V_L, V_L_tree};

                        return res;
                    }

                    static inline std::array<typename FieldType::value_type, argument_size>
                        verify_eval(const typename policy_type::preprocessed_public_data_type preprocessed_data,
                                    const std::vector<plonk_gate<FieldType, plonk_lookup_constraint<FieldType>>> &lookup_gates,
                                    // y
                                    const typename FieldType::value_type &challenge,
                                    typename policy_type::evaluation_map &evaluations,
                                    // A_perm(y):
                                    const typename FieldType::value_type &F_perm_input_polynomial_value,
                                    // A_perm(y * omega ^ {-1}):
                                    const typename FieldType::value_type &F_perm_input_shifted_polynomial_value,
                                    // S_perm(y):
                                    const typename FieldType::value_type &F_perm_value_polynomial_value,
                                    // V_L(y):
                                    const typename FieldType::value_type &V_L_polynomial_value,
                                    // V_P(omega * y):
                                    const typename FieldType::value_type &V_L_polynomial_shifted_value,
                                    const typename CommitmentSchemeTypePermutation::commitment_type &F_perm_input_commitment,
                                    const typename CommitmentSchemeTypePermutation::commitment_type &F_perm_value_commitment,
                                    const typename CommitmentSchemeTypePermutation::commitment_type &V_L_commitment,
                                    transcript_type &transcript = transcript_type()) {
                            // 1. Get theta
                            typename FieldType::value_type theta = transcript.template challenge<FieldType>();
                            // 2. Add commitments to transcript
                            transcript(F_perm_input_commitment);
                            transcript(F_perm_value_commitment);

                            // 3. Calculate input_lookup and value_lookup compression at challenge point
                            typename FieldType::value_type F_input_compr = FieldType::value_type::zero();
                            typename FieldType::value_type F_value_compr = FieldType::value_type::zero();

                            typename FieldType::value_type theta_acc = FieldType::value_type::one();

                            for (std::size_t i = 0; i < lookup_gates.size(); i++) {
                                for (std::size_t j = 0; j < lookup_gates[i].constraints.size(); j++) {
                                    for (math::non_linear_term<VariableType> lookup :lookup_gates[i].constraints[j].lookup_input) {
                                        int k = 0;
                                        std::tuple<std::size_t,
                                        int, typename VariableType::column_type> input_key = std::make_tuple(lookup.vars[0].index, lookup.vars[0].rotation, lookup.vars[0].type);
                                        std::tuple<std::size_t,
                                        int, typename VariableType::column_type> value_key = std::make_tuple(lookup_gates[i].constraints[j].lookup_value[k].index,
                                        lookup_gates[i].constraints[j].lookup_value[k].rotation, lookup_gates[i].constraints[j].lookup_value[k].type);

                                         std::tuple<std::size_t, int, typename plonk_variable<FieldType>::column_type> selector_key = std::make_tuple(lookup_gates[i].selector_index, 0,
                                        plonk_variable<FieldType>::column_type::selector);

                                        F_input_compr = F_input_compr + theta_acc * evaluations[input_key] * lookup.coeff
                                        * evaluations[selector_key];

                                        F_value_compr = F_value_compr + theta_acc * evaluations[value_key]
                                        * evaluations[selector_key];
                                        k++;
                                        theta_acc = theta * theta_acc;
                                    }
                                }
                            }

                            //4. Denote g and h
                            typename FieldType::value_type beta = transcript.template challenge<FieldType>();
                            typename FieldType::value_type gamma = transcript.template challenge<FieldType>();
                            transcript(V_L_commitment);
                            typename FieldType::value_type g = (F_input_compr + beta) * (F_value_compr + gamma);
                            typename FieldType::value_type h = (F_perm_input_polynomial_value + beta) * (F_perm_value_polynomial_value + gamma);
                            std::array<typename FieldType::value_type, argument_size> F;
                            typename FieldType::value_type one = FieldType::value_type::one();
                            F[0] = preprocessed_data.common_data.lagrange_0.evaluate(challenge) * (one - V_L_polynomial_value);
                            F[1] = (one - preprocessed_data.q_last.evaluate(challenge) -
                                preprocessed_data.q_blind.evaluate(challenge)) *
                               (V_L_polynomial_shifted_value * h - V_L_polynomial_value * g);
                            F[2] = preprocessed_data.q_last.evaluate(challenge) * (V_L_polynomial_value * V_L_polynomial_value - V_L_polynomial_value);
                            F[3] = preprocessed_data.common_data.lagrange_0.evaluate(challenge) *
                            (F_perm_input_polynomial_value - F_perm_value_polynomial_value);
                            F[4] = (one - preprocessed_data.q_last.evaluate(challenge) -
                                preprocessed_data.q_blind.evaluate(challenge)) * (F_perm_input_polynomial_value - F_perm_value_polynomial_value) *
                                (F_perm_input_polynomial_value - F_perm_input_shifted_polynomial_value);

                        return F;
                    }
                };
                
            }
        }
    }
}

#endif    // #ifndef CRYPTO3_ZK_PLONK_REDSHIFT_PERMUTATION_ARGUMENT_HPP
