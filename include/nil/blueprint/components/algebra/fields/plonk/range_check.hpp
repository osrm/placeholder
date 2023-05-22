//---------------------------------------------------------------------------//
// Copyright (c) 2022 Ilia Shirobokov <i.shirobokov@nil.foundation>
// Copyright (c) 2023 Dmitrii Tabalin <d.tabalin@nil.foundation>
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

#ifndef CRYPTO3_ZK_BLUEPRINT_PLONK_FIELD_RANGE_CHECK_HPP
#define CRYPTO3_ZK_BLUEPRINT_PLONK_FIELD_RANGE_CHECK_HPP

#include <cmath>

#include <nil/marshalling/algorithms/pack.hpp>

#include <nil/crypto3/zk/snark/arithmetization/plonk/constraint_system.hpp>

#include <nil/blueprint/blueprint/plonk/circuit.hpp>
#include <nil/blueprint/blueprint/plonk/assignment.hpp>
#include <nil/blueprint/component.hpp>

#include <utility>
#include <type_traits>

namespace nil {
        namespace blueprint {
            namespace components {

                // Constraint that x < 2**R.
                // Works when R < modulus_bits.
                // Input: x \in Fp
                template<typename ArithmetizationType, std::uint32_t WitnessesAmount, std::size_t R>
                class range_check;

                // The idea is split x in ConstraintDegree-bit chunks.
                // Then, for each chunk x_i, we constraint that x_i < 2**ConstraintDegree.
                // Thus, we get R/ConstraintDegree chunks that is proved to be less than 2**ConstraintDegree.
                // We can aggreate them into one value < 2**R.
                // Layout:
                // W0  | W1   | ... | W14
                //  0  | ...  | ... | ...
                // sum | c_0  | ... | c_13
                // sum | c_14 | ... | c_27
                // ...
                // The last sum = x
                template<typename BlueprintFieldType, typename ArithmetizationParams, std::uint32_t WitnessesAmount,
                         std::size_t R>
                class range_check<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                              ArithmetizationParams>,
                                                                              WitnessesAmount, R>:
                    public plonk_component<BlueprintFieldType, ArithmetizationParams, WitnessesAmount, 1, 0> {

                    using component_type = plonk_component<BlueprintFieldType, ArithmetizationParams,
                                                           WitnessesAmount, 1, 0>;
                public:
                    using var = typename component_type::var;
                    constexpr static const std::size_t chunk_size = 2;
                    constexpr static const std::size_t reserved_columns = 1;
                    constexpr static const std::size_t chunks_per_row = WitnessesAmount - reserved_columns;
                    constexpr static const std::size_t bits_per_row = chunks_per_row * chunk_size;

                    constexpr static const std::size_t rows_amount =
                        1 + (R + bits_per_row - 1) / bits_per_row;    // ceil(R / bits_per_row)
                    constexpr static const std::size_t padded_chunks = (rows_amount - 1) * chunks_per_row;
                    constexpr static const std::size_t padding_size = padded_chunks - (R + chunk_size - 1) / chunk_size;
                    constexpr static const std::size_t padding_bits = padded_chunks * chunk_size - R;
                    constexpr static const std::size_t gates_amount = 1 + (R % chunk_size == 0 ? 0 : 1);

                    struct input_type {
                        var x;
                    };

                    struct result_type {
                        result_type(const range_check &component, std::size_t start_row_index) {}
                    };

                    template<typename ContainerType>
                        range_check(ContainerType witness):
                            component_type(witness, {}, {}) {};

                    template<typename WitnessContainerType, typename ConstantContainerType,
                             typename PublicInputContainerType>
                        range_check(WitnessContainerType witness, ConstantContainerType constant, PublicInputContainerType public_input):
                            component_type(witness, constant, public_input) {};

                    range_check(
                        std::initializer_list<typename component_type::witness_container_type::value_type> witnesses,
                        std::initializer_list<typename component_type::constant_container_type::value_type> constants,
                        std::initializer_list<typename component_type::public_input_container_type::value_type>
                            public_inputs) : component_type(witnesses, constants, public_inputs) {};

                };


                template<typename BlueprintFieldType, typename ArithmetizationParams, std::uint32_t WitnessesAmount,
                         std::size_t R>
                using plonk_range_check =
                    range_check<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                            ArithmetizationParams>,
                                WitnessesAmount, R>;

                template<typename BlueprintFieldType, typename ArithmetizationParams, std::uint32_t WitnessesAmount,
                         std::size_t R,
                         std::enable_if_t<R < BlueprintFieldType::modulus_bits, bool> = true>
                typename plonk_range_check<BlueprintFieldType, ArithmetizationParams,
                                           WitnessesAmount, R>::result_type
                generate_circuit(
                    const plonk_range_check<BlueprintFieldType, ArithmetizationParams, WitnessesAmount, R>
                        &component,
                    circuit<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                        ArithmetizationParams>>
                        &bp,
                    assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                        ArithmetizationParams>>
                        &assignment,
                    const typename plonk_range_check<BlueprintFieldType, ArithmetizationParams,
                                                     WitnessesAmount, R>::input_type
                        &instance_input,
                    const std::uint32_t start_row_index) {

                    auto selector_iterator = assignment.find_selector(component);
                    std::size_t first_selector_index;

                    if (selector_iterator == assignment.selectors_end()) {
                        first_selector_index = assignment.allocate_selector(component, component.gates_amount);
                        generate_gates(component, bp, assignment, instance_input, first_selector_index);
                    } else {
                        first_selector_index = selector_iterator->second;
                    }

                    assignment.enable_selector(first_selector_index, start_row_index + 1,
                                               start_row_index + component.rows_amount - 1);
                    if ((R % component.chunk_size) != 0) {
                        assignment.enable_selector(first_selector_index + 1, start_row_index + 1);
                    }

                    generate_copy_constraints(component, bp, assignment, instance_input, start_row_index);
                    generate_assignments_constants(component, assignment, instance_input, start_row_index);

                    return typename plonk_range_check<BlueprintFieldType, ArithmetizationParams,
                                                      WitnessesAmount, R>::result_type(
                            component, start_row_index);
                }

                template<typename BlueprintFieldType, typename ArithmetizationParams, std::uint32_t WitnessesAmount,
                         std::size_t R,
                         std::enable_if_t<R < BlueprintFieldType::modulus_bits, bool> = true>
                typename plonk_range_check<BlueprintFieldType, ArithmetizationParams,
                                           WitnessesAmount, R>::result_type
                generate_assignments(
                    const plonk_range_check<BlueprintFieldType, ArithmetizationParams, WitnessesAmount, R>
                        &component,
                    assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                           ArithmetizationParams>>
                        &assignment,
                    const typename plonk_range_check<BlueprintFieldType, ArithmetizationParams,
                                                     WitnessesAmount, R>::input_type
                        &instance_input,
                    const std::uint32_t start_row_index) {

                    std::size_t row = start_row_index;

                    using component_type = plonk_range_check<BlueprintFieldType, ArithmetizationParams,
                                                             WitnessesAmount, R>;
                    using value_type = typename BlueprintFieldType::value_type;
                    using integral_type = typename BlueprintFieldType::integral_type;
                    using chunk_type = std::uint8_t;
                    BOOST_ASSERT(component.chunk_size <= 8);

                    value_type x = var_value(assignment, instance_input.x);

                    integral_type x_integral = integral_type(x.data);

                    std::array<bool, R + component_type::padding_bits> bits;
                    bits.fill(0);
                    {
                        nil::marshalling::status_type status;
                        std::array<bool, BlueprintFieldType::modulus_bits> bytes_all =
                            nil::marshalling::pack<nil::marshalling::option::big_endian>(x_integral, status);
                        std::copy(bytes_all.end() - R, bytes_all.end(), bits.begin() + component_type::padding_bits);
                        assert(status == nil::marshalling::status_type::success);
                    }

                    BOOST_ASSERT(component.chunk_size <= 8);

                    std::array<chunk_type, component_type::padded_chunks> chunks;
                    for (std::size_t i = 0; i < component.padded_chunks; i++) {
                        chunk_type chunk_value = 0;
                        for (std::size_t j = 0; j < component.chunk_size; j++) {
                            chunk_value <<= 1;
                            chunk_value |= bits[i * component.chunk_size + j];
                        }
                        chunks[i] = chunk_value;
                    }

                    assignment.witness(component.W(0), row) = 0;
                    row++;

                    value_type sum = 0;

                    for (std::size_t i = 0; i < component.rows_amount - 1; i++) {
                        for (std::size_t j = 0; j < component.chunks_per_row; j++) {
                            assignment.witness(component.W(0 + component.reserved_columns + j), row) =
                                chunks[i * component.chunks_per_row + j];
                            sum *= (1 << component.chunk_size);
                            sum += chunks[i * component.chunks_per_row + j];
                        }
                        assignment.witness(component.W(0), row) = sum;
                        row++;
                    }

                    BOOST_ASSERT(row == start_row_index + component.rows_amount);

                    return typename component_type::result_type(component, start_row_index);
                }

                template<typename BlueprintFieldType, typename ArithmetizationParams, std::uint32_t WitnessesAmount,
                         std::size_t R,
                         std::enable_if_t<R < BlueprintFieldType::modulus_bits, bool> = true>
                void generate_gates(
                    const plonk_range_check<BlueprintFieldType, ArithmetizationParams,
                                            WitnessesAmount, R>
                        &component,
                    circuit<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                        ArithmetizationParams>>
                        &bp,
                    assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                           ArithmetizationParams>>
                        &assignment,
                    const typename plonk_range_check<BlueprintFieldType, ArithmetizationParams,
                                                     WitnessesAmount, R>::input_type
                        &instance_input,
                    const std::size_t first_selector_index) {

                    using var = typename plonk_range_check<BlueprintFieldType, ArithmetizationParams,
                                                           WitnessesAmount, R>::var;
                    using constraint_type = crypto3::zk::snark::plonk_constraint<BlueprintFieldType>;
                    using gate_type = typename crypto3::zk::snark::plonk_gate<BlueprintFieldType,
                                                    crypto3::zk::snark::plonk_constraint<BlueprintFieldType>>;

                    typename BlueprintFieldType::value_type base_two = 2;

                    std::vector<constraint_type> constraints;

                    auto generate_chunk_size_constraint = [](var v, std::size_t size) {
                        constraint_type constraint = v;
                        for (std::size_t i = 1; i < (1 << size); i++) {
                            constraint = constraint * (v - i);
                        }
                        return constraint;
                    };

                    // assert chunk size
                    for (std::size_t i = 0; i < component.chunks_per_row; i++) {
                        constraint_type chunk_range_constraint = generate_chunk_size_constraint(
                            var(component.W(0 + component.reserved_columns + i), 0, true), component.chunk_size);

                        constraints.push_back(bp.add_constraint(chunk_range_constraint));
                    }
                    // assert sum
                    constraint_type sum_constraint = var(component.W(0 + component.reserved_columns), 0, true);
                    for (std::size_t i = 1; i < component.chunks_per_row; i++) {
                        sum_constraint =
                            base_two.pow(component.chunk_size) * sum_constraint +
                            var(component.W(0 + component.reserved_columns + i), 0, true);
                    }
                    sum_constraint = sum_constraint +
                                        base_two.pow(component.chunk_size * component.chunks_per_row) *
                                                    var(component.W(0), -1, true) -
                                        var(component.W(0), 0, true);
                    constraints.push_back(bp.add_constraint(sum_constraint));

                    gate_type gate(first_selector_index, constraints);
                    bp.add_gate(gate);

                    if (R % component.chunk_size == 0) return;
                    // If R is not divisible by chunk size, the first chunk should be constrained to be
                    // less than 2^{R % component.chunk_size}
                    constraint_type first_chunk_range_constraint = generate_chunk_size_constraint(
                        var(component.W(0 + component.reserved_columns + component.padding_size), 0, true),
                        R % component.chunk_size);

                    gate = gate_type(first_selector_index + 1, first_chunk_range_constraint);
                    bp.add_gate(gate);
                }

                template<typename BlueprintFieldType, typename ArithmetizationParams, std::uint32_t WitnessesAmount,
                         std::size_t R,
                         std::enable_if_t<R < BlueprintFieldType::modulus_bits, bool> = true>
                void generate_copy_constraints(
                    const plonk_range_check<BlueprintFieldType, ArithmetizationParams,
                                            WitnessesAmount, R>
                        &component,
                    circuit<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                        ArithmetizationParams>>
                        &bp,
                    assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                            ArithmetizationParams>>
                        &assignment,
                    const typename plonk_range_check<BlueprintFieldType, ArithmetizationParams,
                                                     WitnessesAmount, R>::input_type
                        &instance_input,
                    const std::uint32_t start_row_index) {

                    using var = typename plonk_range_check<BlueprintFieldType, ArithmetizationParams,
                                                           WitnessesAmount, R>::var;

                    var zero(0, start_row_index, false, var::column_type::constant);
                    bp.add_copy_constraint({zero, var(component.W(0), start_row_index, false)});

                    for (std::size_t i = 1; i <= component.padding_size; i++) {
                        bp.add_copy_constraint({zero, var(component.W(i), start_row_index + 1, false)});
                    }

                    bp.add_copy_constraint({instance_input.x,
                                            var(component.W(0),
                                                start_row_index + component.rows_amount - 1, false)});
                }

                template<typename BlueprintFieldType, typename ArithmetizationParams, std::uint32_t WitnessesAmount,
                         std::size_t R,
                         std::enable_if_t<R < BlueprintFieldType::modulus_bits, bool> = true>
                void generate_assignments_constants(
                    const plonk_range_check<BlueprintFieldType, ArithmetizationParams,
                                            WitnessesAmount, R>
                        &component,
                    assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                            ArithmetizationParams>>
                        &assignment,
                    const typename plonk_range_check<BlueprintFieldType, ArithmetizationParams,
                                                     WitnessesAmount, R>::input_type
                        &instance_input,
                    const std::uint32_t start_row_index) {

                    assignment.constant(component.C(0), start_row_index) = 0;
                }
            }   // namespace components
        }       // namespace blueprint
}   // namespace nil

#endif    // CRYPTO3_ZK_BLUEPRINT_PLONK_FIELD_RANGE_CHECK_HPP