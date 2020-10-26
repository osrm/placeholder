//---------------------------------------------------------------------------//
// Copyright (c) 2020 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2020 Nikita Kaskov <nbering@nil.foundation>
// Copyright (c) 2020 Ilias Khairullin <ilias@nil.foundation>
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

#ifndef CRYPTO3_ALGEBRA_CURVES_BLS12_HPP
#define CRYPTO3_ALGEBRA_CURVES_BLS12_HPP

#include <nil/crypto3/algebra/curves/detail/bls12/basic_policy.hpp>
#include <nil/crypto3/algebra/curves/detail/bls12/g1.hpp>
#include <nil/crypto3/algebra/curves/detail/bls12/g2.hpp>

#include <nil/crypto3/algebra/pairing/bls12.hpp>

namespace nil {
    namespace crypto3 {
        namespace algebra {
            namespace curves {

                using namespace nil::crypto3::algebra;

                /*
                    E/Fp: y^2 = x^3 + 4.
                */
                template<std::size_t ModulusBits, std::size_t GeneratorBits = CHAR_BIT>
                struct bls12 {

                    using policy_type = detail::bls12_basic_policy<ModulusBits, GeneratorBits>;

                    typedef typename policy_type::base_field_type base_field_type;
                    typedef typename policy_type::scalar_field_type scalar_field_type;
                    typedef typename policy_type::number_type number_type;
                    typedef typename policy_type::extended_number_type extended_number_type;

                    constexpr static const std::size_t base_field_bits = policy_type::base_field_bits;
                    constexpr static const number_type p = policy_type::p;

                    constexpr static const std::size_t scalar_field_bits = policy_type::scalar_field_bits;
                    constexpr static const number_type q = policy_type::q;

                    typedef typename detail::bls12_g1<base_field_bits> g1_type;
                    typedef typename detail::bls12_g2<base_field_bits> g2_type;

                    typedef std::vector<g1_type> g1_vector;
                    typedef std::vector<g2_type> g2_vector;

                    constexpr static const bool has_affine_pairing = false;

                    typedef typename pairing::pairing_policy<bls12<ModulusBits, GeneratorBits>> pairing_policy;

                    typedef typename policy_type::gt_field_type gt_type;
                };

                typedef bls12<381> bls12_381;
                typedef bls12<377> bls12_377;
            }    // namespace curves
        }        // namespace algebra
    }            // namespace crypto3
}    // namespace nil

#endif    // CRYPTO3_ALGEBRA_CURVES_BLS12_381_HPP
