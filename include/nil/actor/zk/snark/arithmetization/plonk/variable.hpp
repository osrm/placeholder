//---------------------------------------------------------------------------//
// Copyright (c) 2020-2021 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2020-2021 Nikita Kaskov <nbering@nil.foundation>
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
// @file Declaration of interfaces for:
// - a variable (i.e., x_i),
// - a linear term (i.e., a_i * x_i), and
// - a linear combination (i.e., sum_i a_i * x_i).
//---------------------------------------------------------------------------//

#ifndef ACTOR_ZK_PLONK_VARIABLE_HPP
#define ACTOR_ZK_PLONK_VARIABLE_HPP

#include <ostream>
#include <vector>
#include <functional>
#include <boost/functional/hash.hpp>

namespace nil {
    namespace actor {
        namespace math {

            /**
             * Forward declaration.
             */
            template<typename VariableType>
            struct term;

            /**
             * Forward declaration.
             */
            template<typename VariableType>
            struct expression;
        }    // namespace math

        namespace zk {
            namespace snark {

                /********************************* Variable **********************************/

                /**
                 * A variable represents a formal expression of the form "w^{index}_{rotation}" and type type.
                 */
                template<typename FieldType>
                class plonk_variable {

                public:
                    using field_type = FieldType;
                    using assignment_type = typename FieldType::value_type;

                    /**
                     * Mnemonic typedefs.
                     */
                    std::int32_t rotation;
                    enum column_type : std::uint8_t { witness, public_input, constant, selector } type;
                    std::size_t index;
                    bool relative;

                    constexpr plonk_variable() : index(0), rotation(0), relative(false), type(column_type::witness) {};

                    constexpr plonk_variable(const std::size_t index,
                                             std::int32_t rotation,
                                             bool relative = true,
                                             column_type type = column_type::witness) :
                        index(index),
                        rotation(rotation), relative(relative), type(type) {};

                    math::expression<plonk_variable<FieldType>> pow(const std::size_t power) const {
                        return math::term<plonk_variable<FieldType>>(*this).pow(power);
                    }

                    math::term<plonk_variable<FieldType>>
                        operator*(const assignment_type &field_coeff) const {
                        return math::term<plonk_variable<FieldType>>(*this) * field_coeff;
                    }

                    math::term<plonk_variable<FieldType>> operator*(const plonk_variable &other) const {
                        return math::term<plonk_variable<FieldType>>(*this) * other;
                    }

                    math::expression<plonk_variable<FieldType>>
                        operator+(const math::expression<plonk_variable<FieldType>> &other) const {
                        math::expression<plonk_variable<FieldType>> result(*this);
                        result += other;
                        return result;
                    }

                    math::expression<plonk_variable<FieldType>>
                        operator-(const math::expression<plonk_variable<FieldType>> &other) const {
                        math::expression<plonk_variable<FieldType>> result(*this);
                        result -= other;
                        return result;
                    }

                    math::term<plonk_variable<FieldType>> operator-() const {
                        return math::term<plonk_variable<FieldType>>(*this) * (-assignment_type::one());
                    }

                    bool operator==(const plonk_variable &other) const {
                        return ((this->index == other.index) && (this->rotation == other.rotation) &&
                                this->type == other.type && this->relative == other.relative);
                    }

                    bool operator!=(const plonk_variable &other) const {
                        return !(*this == other);
                    }

                    bool operator<(const plonk_variable &other) const {
                        if (this->index != other.index)
                            return this->index < other.index;
                        if (this->rotation != other.rotation)
                            return this->rotation < other.rotation;
                        if (this->type != other.type)
                            return this->type < other.type;
                        return this->relative < other.relative;
                    }
                };

                template<typename FieldType>
                math::term<plonk_variable<FieldType>>
                    operator*(const typename FieldType::value_type &field_coeff, const plonk_variable<FieldType> &var) {
                    return var * field_coeff;
                }

                template<typename FieldType>
                math::expression<plonk_variable<FieldType>>
                    operator+(const typename FieldType::value_type &field_val, const plonk_variable<FieldType> &var) {
                    return var + field_val;
                }

                template<typename FieldType>
                math::expression<plonk_variable<FieldType>>
                    operator-(const typename FieldType::value_type &field_val, const plonk_variable<FieldType> &var) {
                    return -(var - field_val);
                }
                
                // Used in the unit test, so we can use BOOST_CHECK_EQUALS, and see
                // the values of terms, when the check fails.
                template<typename FieldType>
                std::ostream& operator<<(std::ostream& os, const plonk_variable<FieldType>& var)
                {
                    std::map<typename plonk_variable<FieldType>::column_type, std::string> type_map = {
                        {plonk_variable<FieldType>::column_type::witness, "witness"}, 
                        {plonk_variable<FieldType>::column_type::public_input, "public_input"},
                        {plonk_variable<FieldType>::column_type::constant, "constant"},
                        {plonk_variable<FieldType>::column_type::selector, "selector"}
                    };
                    os << "var_" << var.index << '_' << var.rotation << '_'
                        << type_map[var.type]
                        << (var.relative ? "_relative" : "");
                    return os;
                }

            }    // namespace snark
        }        // namespace zk
    }            // namespace actor
}    // namespace nil

template<typename FieldType>
struct std::hash<nil::crypto3::zk::snark::plonk_variable<FieldType>>
{
    std::size_t operator()(const nil::crypto3::zk::snark::plonk_variable<FieldType>& var) const
    {
        std::size_t result = std::hash<std::int32_t>()(var.rotation);
        boost::hash_combine(result, std::hash<std::int8_t>()(var.type));
        boost::hash_combine(result, std::hash<std::size_t>()(var.index));
        boost::hash_combine(result, std::hash<bool>()(var.relative));
        return result;
    }
};

#endif    // ACTOR_ZK_PLONK_VARIABLE_HPP
