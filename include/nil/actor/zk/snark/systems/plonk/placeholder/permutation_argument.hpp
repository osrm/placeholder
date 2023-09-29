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

#ifndef ACTOR_ZK_PLONK_PLACEHOLDER_PERMUTATION_ARGUMENT_HPP
#define ACTOR_ZK_PLONK_PLACEHOLDER_PERMUTATION_ARGUMENT_HPP

#include <algorithm>

#include <nil/actor/math/polynomial/polynomial.hpp>
#include <nil/actor/math/polynomial/polynomial_dfs.hpp>
#include <nil/actor/math/polynomial/shift.hpp>
#include <nil/actor/math/domains/evaluation_domain.hpp>
#include <nil/actor/math/algorithms/make_evaluation_domain.hpp>

#include <nil/crypto3/hash/sha2.hpp>

#include <nil/actor/container/merkle/tree.hpp>

#include <nil/actor/zk/transcript/fiat_shamir.hpp>
#include <nil/actor/zk/snark/systems/plonk/placeholder/params.hpp>
#include <nil/actor/zk/snark/systems/plonk/placeholder/detail/placeholder_policy.hpp>
#include <nil/actor/zk/snark/systems/plonk/placeholder/detail/placeholder_scoped_profiler.hpp>
#include <nil/actor/zk/snark/systems/plonk/placeholder/preprocessor.hpp>

namespace nil {
    namespace actor {
        namespace zk {
            namespace snark {
                template<typename FieldType, typename ParamsType>
                class placeholder_permutation_argument {

                    using transcript_hash_type = typename ParamsType::transcript_hash_type;
                    using transcript_type = transcript::fiat_shamir_heuristic_sequential<transcript_hash_type>;

                    static constexpr std::size_t argument_size = 3;

                    using permutation_commitment_scheme_type = typename ParamsType::runtime_size_commitment_scheme_type;

                public:
                    struct prover_result_type {
                        std::array<math::polynomial_dfs<typename FieldType::value_type>, argument_size> F_dfs;

                        math::polynomial_dfs<typename FieldType::value_type> permutation_polynomial_dfs;

                        typename permutation_commitment_scheme_type::precommitment_type permutation_poly_precommitment;
                    };

                    static inline math::polynomial_dfs<typename FieldType::value_type> polynomial_product(
                        std::vector<math::polynomial_dfs<typename FieldType::value_type>> multipliers)
                    {
                        // TODO(martun): Make this part parallel.
                        while (multipliers.size() != 1) {
                            for (int i = 0; i < multipliers.size() / 2; ++i) {
                                multipliers[i] = multipliers[2 * i] * multipliers[2 * i + 1];
                            }
                            if (multipliers.size() % 2 != 0) {
                                multipliers[multipliers.size() / 2] = multipliers[multipliers.size() - 1];
                            }
                            // Delete the second half.
                            multipliers.resize(multipliers.size() / 2 + multipliers.size() % 2);
                        }
                        return multipliers[0];
                    }

                    static inline future<prover_result_type> prove_eval(
                        const plonk_constraint_system<FieldType, typename ParamsType::arithmetization_params>
                            &constraint_system,
                        const typename placeholder_public_preprocessor<FieldType, ParamsType>::preprocessed_data_type
                            preprocessed_data,
                        const plonk_table_description<FieldType, typename ParamsType::arithmetization_params>
                            &table_description,
                        const plonk_polynomial_dfs_table<FieldType, typename ParamsType::arithmetization_params>
                            &column_polynomials,
                        const typename ParamsType::commitment_params_type& fri_params,
                        transcript_type& transcript) {

                        PROFILE_PLACEHOLDER_SCOPE("permutation_argument_prove_eval_time");

                        const std::vector<math::polynomial_dfs<typename FieldType::value_type>> &S_sigma =
                            preprocessed_data.permutation_polynomials;
                        const std::vector<math::polynomial_dfs<typename FieldType::value_type>> &S_id =
                            preprocessed_data.identity_polynomials;
                        std::shared_ptr<math::evaluation_domain<FieldType>> basic_domain =
                            preprocessed_data.common_data.basic_domain;
                        // 1. $\beta_1, \gamma_1 = \challenge$
                        typename FieldType::value_type beta = transcript.template challenge<FieldType>();
                        typename FieldType::value_type gamma = transcript.template challenge<FieldType>();

                        // 2. Calculate id_binding, sigma_binding for j from 1 to N_rows
                        // 3. Calculate $V_P$
                        math::polynomial_dfs<typename FieldType::value_type> V_P(basic_domain->size() - 1,
                                                                                 basic_domain->size());

                        std::vector<math::polynomial_dfs<typename FieldType::value_type>> g_v;
                        std::vector<math::polynomial_dfs<typename FieldType::value_type>> h_v;
                        for (std::size_t i = 0; i < S_id.size(); i++) {
                            BOOST_ASSERT(column_polynomials[i].size() == basic_domain->size());
                            BOOST_ASSERT(S_id[i].size() == basic_domain->size());
                            BOOST_ASSERT(S_sigma[i].size() == basic_domain->size());

                            g_v.push_back(column_polynomials[i] + beta * S_id[i] + gamma);
                            h_v.push_back(column_polynomials[i] + beta * S_sigma[i] + gamma);
                        }

                        V_P[0] = FieldType::value_type::one();
                        for (std::size_t j = 1; j < basic_domain->size(); j++) {
                            typename FieldType::value_type nom = FieldType::value_type::one();
                            typename FieldType::value_type denom = FieldType::value_type::one();

                            for (std::size_t i = 0; i < S_id.size(); i++) {
                                nom *= g_v[i][j - 1];
                                denom *= h_v[i][j - 1];
                            }
                            V_P[j] = V_P[j - 1] * nom / denom;
                        }
                        V_P.resize(fri_params.D[0]->m).get();

                        // 4. Compute and add commitment to $V_P$ to $\text{transcript}$.
                        typename permutation_commitment_scheme_type::precommitment_type V_P_tree =
                            zk::algorithms::precommit<permutation_commitment_scheme_type>(
                                V_P, fri_params.D[0], fri_params.step_list.front()).get();
                        typename permutation_commitment_scheme_type::commitment_type V_P_commitment =
                            zk::algorithms::commit<permutation_commitment_scheme_type>(V_P_tree);
                        transcript(V_P_commitment);
                        // 5. Calculate g_perm, h_perm
                        math::polynomial_dfs<typename FieldType::value_type> g = polynomial_product(g_v);
                        math::polynomial_dfs<typename FieldType::value_type> h = polynomial_product(h_v);

                        math::polynomial_dfs<typename FieldType::value_type> one_polynomial(
                            0, V_P.size(), FieldType::value_type::one());
                        std::array<math::polynomial_dfs<typename FieldType::value_type>, argument_size> F_dfs;
                        math::polynomial_dfs<typename FieldType::value_type> V_P_shifted =
                            math::polynomial_shift(V_P, 1, basic_domain->m).get();

                        F_dfs[0] = preprocessed_data.common_data.lagrange_0 * (one_polynomial - V_P);
                        F_dfs[1] = (one_polynomial - (preprocessed_data.q_last + preprocessed_data.q_blind)) * (V_P_shifted * h - V_P * g);
                        F_dfs[2] = preprocessed_data.q_last * V_P * (V_P - one_polynomial);

                        prover_result_type res = {F_dfs, V_P, V_P_tree};

                        return make_ready_future<prover_result_type>(res);
                    }

                    static inline std::array<typename FieldType::value_type, argument_size> verify_eval(
                        const typename placeholder_public_preprocessor<FieldType, ParamsType>::preprocessed_data_type
                            &preprocessed_data,
                        // y
                        const typename FieldType::value_type &challenge,
                        // f(y):
                        const std::vector<typename FieldType::value_type> &column_polynomials_values,
                        // V_P(y):
                        const typename FieldType::value_type &perm_polynomial_value,
                        // V_P(omega * y):
                        const typename FieldType::value_type &perm_polynomial_shifted_value,
                        const typename permutation_commitment_scheme_type::commitment_type &V_P_commitment,
                        transcript_type& transcript) {

                        const std::vector<math::polynomial_dfs<typename FieldType::value_type>> &S_sigma =
                            preprocessed_data.permutation_polynomials;
                        const std::vector<math::polynomial_dfs<typename FieldType::value_type>> &S_id =
                            preprocessed_data.identity_polynomials;

                        // 1. Get beta, gamma
                        typename FieldType::value_type beta = transcript.template challenge<FieldType>();
                        typename FieldType::value_type gamma = transcript.template challenge<FieldType>();

                        // 2. Add commitment to V_P to transcript
                        transcript(V_P_commitment);

                        // 3. Calculate h_perm, g_perm at challenge point
                        math::polynomial_dfs<typename FieldType::value_type> g_poly(
                            0, preprocessed_data.common_data.basic_domain->size(), FieldType::value_type::one()
                        );
                        math::polynomial_dfs<typename FieldType::value_type> h_poly(
                            0, preprocessed_data.common_data.basic_domain->size(), FieldType::value_type::one()
                        );

                        for (std::size_t i = 0; i < column_polynomials_values.size(); i++) {
                            typename FieldType::value_type pp = column_polynomials_values[i] + gamma;

                            g_poly = g_poly * (S_id[i] * beta + pp);
                            h_poly = h_poly * (S_sigma[i] * beta  + pp);

                        }
                        std::array<typename FieldType::value_type, argument_size> F;
                        typename FieldType::value_type one = FieldType::value_type::one();

                        F[0] = preprocessed_data.common_data.lagrange_0.evaluate(challenge) *
                               (one - perm_polynomial_value);
                        F[1] = ((one - preprocessed_data.q_last - preprocessed_data.q_blind) *
                               (perm_polynomial_shifted_value * h_poly - perm_polynomial_value * g_poly)).evaluate(challenge);
                        F[2] = preprocessed_data.q_last.evaluate(challenge) *
                               (perm_polynomial_value.squared() - perm_polynomial_value);

                        return F;
                    }
                };
            }    // namespace snark
        }        // namespace zk
    }            // namespace actor
}    // namespace nil

#endif    // #ifndef ACTOR_ZK_PLONK_PLACEHOLDER_PERMUTATION_ARGUMENT_HPP
