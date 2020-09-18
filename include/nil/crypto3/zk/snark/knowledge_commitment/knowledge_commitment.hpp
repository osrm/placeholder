//---------------------------------------------------------------------------//
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2020 Nikita Kaskov <nbering@nil.foundation>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_ZK_KNOWLEDGE_COMMITMENT_HPP_
#define CRYPTO3_ZK_KNOWLEDGE_COMMITMENT_HPP_

#include <nil/algebra/fields/field.hpp>

#include <nil/crypto3/zk/snark/sparse_vector.hpp>

namespace nil {
    namespace crypto3 {
        namespace zk {
            namespace snark {

                /********************** Knowledge commitment *********************************/

                /**
                 * A knowledge commitment is a pair (g,h) where g is in T1 and h in T2,
                 * and T1 and T2 are groups (written additively).
                 *
                 * Such pairs form a group by defining:
                 * - "zero" = (0,0)
                 * - "one" = (1,1)
                 * - a * (g,h) + b * (g',h') := ( a * g + b * g', a * h + b * h').
                 */
                template<typename T1, typename T2>
                struct knowledge_commitment {

                    T1 g;
                    T2 h;

                    knowledge_commitment<T1, T2>() = default;
                    knowledge_commitment<T1, T2>(const knowledge_commitment<T1, T2> &other) = default;
                    knowledge_commitment<T1, T2>(knowledge_commitment<T1, T2> &&other) = default;
                    knowledge_commitment<T1, T2>(const T1 &g, const T2 &h);

                    knowledge_commitment<T1, T2> &operator=(const knowledge_commitment<T1, T2> &other) = default;
                    knowledge_commitment<T1, T2> &operator=(knowledge_commitment<T1, T2> &&other) noexcept = default;
                    knowledge_commitment<T1, T2> operator+(const knowledge_commitment<T1, T2> &other) const;
                    knowledge_commitment<T1, T2> mixed_add(const knowledge_commitment<T1, T2> &other) const;
                    knowledge_commitment<T1, T2> dbl() const;

                    void to_special();
                    bool is_special() const;

                    bool is_zero() const;
                    bool operator==(const knowledge_commitment<T1, T2> &other) const;
                    bool operator!=(const knowledge_commitment<T1, T2> &other) const;

                    static knowledge_commitment<T1, T2> zero();
                    static knowledge_commitment<T1, T2> one();

                    void print() const;

                    static std::size_t size_in_bits();

                    static void batch_to_special_all_non_zeros(std::vector<knowledge_commitment<T1, T2>> &vec);
                };

                template<typename T1, typename T2, typename Backend,
                         boost::multiprecision::expression_template_option ExpressionTemplates>
                knowledge_commitment<T1, T2>
                    operator*(const boost::multiprecision::number<Backend, ExpressionTemplates> &lhs,
                              const knowledge_commitment<T1, T2> &rhs) {
                    return knowledge_commitment<T1, T2>(lhs * rhs.g, lhs * rhs.h);
                }

                template<typename T1, typename T2, typename FieldType>
                knowledge_commitment<T1, T2> operator*(const typename FieldType::value_type &lhs,
                                                       const knowledge_commitment<T1, T2> &rhs) {
                    return lhs * rhs;
                }

                template<typename T1, typename T2>
                std::ostream &operator<<(std::ostream &out, const knowledge_commitment<T1, T2> &kc);

                template<typename T1, typename T2>
                std::istream &operator>>(std::istream &in, knowledge_commitment<T1, T2> &kc);

                /******************** Knowledge commitment vector ****************************/

                /**
                 * A knowledge commitment vector is a sparse vector of knowledge commitments.
                 */
                template<typename T1, typename T2>
                using knowledge_commitment_vector = sparse_vector<knowledge_commitment<T1, T2>>;

                template<typename T1, typename T2>
                knowledge_commitment<T1, T2>::knowledge_commitment(const T1 &g, const T2 &h) : g(g), h(h) {
                }

                template<typename T1, typename T2>
                knowledge_commitment<T1, T2> knowledge_commitment<T1, T2>::zero() {
                    return knowledge_commitment<T1, T2>(T1::zero(), T2::zero());
                }

                template<typename T1, typename T2>
                knowledge_commitment<T1, T2> knowledge_commitment<T1, T2>::one() {
                    return knowledge_commitment<T1, T2>(T1::one(), T2::one());
                }

                template<typename T1, typename T2>
                knowledge_commitment<T1, T2>
                    knowledge_commitment<T1, T2>::operator+(const knowledge_commitment<T1, T2> &other) const {
                    return knowledge_commitment<T1, T2>(this->g + other.g, this->h + other.h);
                }

                template<typename T1, typename T2>
                knowledge_commitment<T1, T2>
                    knowledge_commitment<T1, T2>::mixed_add(const knowledge_commitment<T1, T2> &other) const {
                    return knowledge_commitment<T1, T2>(this->g.mixed_add(other.g), this->h.mixed_add(other.h));
                }

                template<typename T1, typename T2>
                knowledge_commitment<T1, T2> knowledge_commitment<T1, T2>::dbl() const {
                    return knowledge_commitment<T1, T2>(this->g.dbl(), this->h.dbl());
                }

                template<typename T1, typename T2>
                void knowledge_commitment<T1, T2>::to_special() {
                    this->g.to_special();
                    this->h.to_special();
                }

                template<typename T1, typename T2>
                bool knowledge_commitment<T1, T2>::is_special() const {
                    return this->g->is_special() && this->h->is_special();
                }

                template<typename T1, typename T2>
                bool knowledge_commitment<T1, T2>::is_zero() const {
                    return (g.is_zero() && h.is_zero());
                }

                template<typename T1, typename T2>
                bool knowledge_commitment<T1, T2>::operator==(const knowledge_commitment<T1, T2> &other) const {
                    return (this->g == other.g && this->h == other.h);
                }

                template<typename T1, typename T2>
                bool knowledge_commitment<T1, T2>::operator!=(const knowledge_commitment<T1, T2> &other) const {
                    return !((*this) == other);
                }

                template<typename T1, typename T2>
                void knowledge_commitment<T1, T2>::print() const {
                    printf("knowledge_commitment.g:\n");
                    g.print();
                    printf("knowledge_commitment.h:\n");
                    h.print();
                }

                template<typename T1, typename T2>
                std::size_t knowledge_commitment<T1, T2>::size_in_bits() {
                    return T1::size_in_bits() + T2::size_in_bits();
                }

                template<typename T1, typename T2>
                std::ostream &operator<<(std::ostream &out, const knowledge_commitment<T1, T2> &kc) {
                    out << kc.g << OUTPUT_SEPARATOR << kc.h;
                    return out;
                }

                template<typename T1, typename T2>
                std::istream &operator>>(std::istream &in, knowledge_commitment<T1, T2> &kc) {
                    in >> kc.g;
                    algebra::consume_OUTPUT_SEPARATOR(in);
                    in >> kc.h;
                    return in;
                }

                template<typename T1, typename T2>
                void knowledge_commitment<T1, T2>::batch_to_special_all_non_zeros(
                    std::vector<knowledge_commitment<T1, T2>> &vec) {
                    // it is guaranteed that every vec[i] is non-zero,
                    // but, for any i, *one* of vec[i].g and vec[i].h might still be zero,
                    // so we still have to handle zeros separately

                    // we separately process g's first, then h's
                    // to lower memory consumption
                    std::vector<T1> g_vec;
                    g_vec.reserve(vec.size());

                    for (std::size_t i = 0; i < vec.size(); ++i) {
                        if (!vec[i].g.is_zero()) {
                            g_vec.emplace_back(vec[i].g);
                        }
                    }

                    T1::batch_to_special_all_non_zeros(g_vec);
                    auto g_it = g_vec.begin();
                    T1 T1_zero_special = T1::zero();
                    T1_zero_special.to_special();

                    for (std::size_t i = 0; i < vec.size(); ++i) {
                        if (!vec[i].g.is_zero()) {
                            vec[i].g = *g_it;
                            ++g_it;
                        } else {
                            vec[i].g = T1_zero_special;
                        }
                    }

                    g_vec.clear();

                    // exactly the same thing, but for h:
                    std::vector<T2> h_vec;
                    h_vec.reserve(vec.size());

                    for (std::size_t i = 0; i < vec.size(); ++i) {
                        if (!vec[i].h.is_zero()) {
                            h_vec.emplace_back(vec[i].h);
                        }
                    }

                    T2::batch_to_special_all_non_zeros(h_vec);
                    auto h_it = h_vec.begin();
                    T2 T2_zero_special = T2::zero();
                    T2_zero_special.to_special();

                    for (std::size_t i = 0; i < vec.size(); ++i) {
                        if (!vec[i].h.is_zero()) {
                            vec[i].h = *h_it;
                            ++h_it;
                        } else {
                            vec[i].h = T2_zero_special;
                        }
                    }

                    h_vec.clear();
                }
            }    // namespace snark
        }        // namespace zk
    }            // namespace crypto3
}    // namespace nil

#endif    // KNOWLEDGE_COMMITMENT_HPP_
