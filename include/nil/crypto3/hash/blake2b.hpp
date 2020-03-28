//---------------------------------------------------------------------------//
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_BLAKE2B_HPP
#define CRYPTO3_BLAKE2B_HPP

#include <nil/crypto3/hash/detail/blake2b/blake2b_functions.hpp>

#include <nil/crypto3/hash/detail/haifa_construction.hpp>
#include <nil/crypto3/hash/detail/haifa_stream_processor.hpp>
#include <nil/crypto3/hash/detail/finalizer.hpp>


namespace nil {
    namespace crypto3 {
        namespace hash {
            template<std::size_t DigestBits>
            class blake2b_compressor {
            protected:
                typedef detail::blake2b_functions<DigestBits> policy_type;

                typedef typename policy_type::byte_type byte_type;
                typedef typename policy_type::state_type::value_type value_type;

            public:
                typedef typename policy_type::iv_generator iv_generator;

                constexpr static const std::size_t word_bits = policy_type::word_bits;
                typedef typename policy_type::word_type word_type;

                constexpr static const std::size_t state_bits = policy_type::state_bits;
                constexpr static const std::size_t state_words = policy_type::state_words;
                typedef typename policy_type::state_type state_type;

                constexpr static const std::size_t block_bits = policy_type::block_bits;
                constexpr static const std::size_t block_words = policy_type::block_words;
                typedef typename policy_type::block_type block_type;

                constexpr static const std::size_t salt_bits = policy_type::salt_bits;
                typedef typename policy_type::salt_type salt_type;
                constexpr static const salt_type salt_value = policy_type::salt_value;

                static void process_block(state_type &state, const block_type &block, value_type seen = value_type(),
                                          value_type finalizator = value_type()) {
                    std::array<word_type, state_words * 2> v;

                    std::move(state.begin(), state.end(), v.begin());
                    std::move(iv_generator()().begin(), iv_generator()().end(), v.begin() + state_words);

                    std::array<typename state_type::value_type, 2> s = {seen / CHAR_BIT + ((seen % CHAR_BIT) ? 1 : 0), 0x00};

                    v[12] ^= s[0];
                    v[13] ^= s[1];

                    s = {finalizator, 0};

                    v[14] ^= s[0];
                    v[15] ^= s[1];

                    s.fill(0);

                    policy_type::template round<0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15>(v, block);
                    policy_type::template round<14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3>(v, block);
                    policy_type::template round<11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4>(v, block);
                    policy_type::template round<7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8>(v, block);
                    policy_type::template round<9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13>(v, block);
                    policy_type::template round<2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9>(v, block);
                    policy_type::template round<12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11>(v, block);
                    policy_type::template round<13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10>(v, block);
                    policy_type::template round<6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5>(v, block);
                    policy_type::template round<10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0>(v, block);
                    policy_type::template round<0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15>(v, block);
                    policy_type::template round<14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3>(v, block);

                    for (size_t i = 0; i < state_words; i++) {
                        state[i] ^= v[i] ^ v[i + state_words];
                    }
                }
            };

            /*!
             * @brief Blake2b. A recently designed hash function. Very fast on 64-bit processors.
             * Can output a hash of any length between 1 and 64 bytes, this is specified by passing
             * a value to the constructor with the desired length.
             *
             * @ingroup hash
             * @tparam DigestBits
             */
            template<std::size_t DigestBits>
            class blake2b {
                typedef detail::blake2b_policy<DigestBits> policy_type;

            public:
                struct construction {
                    struct params_type {
                        typedef typename stream_endian::little_octet_big_bit digest_endian;

                        constexpr static const std::size_t length_bits = policy_type::word_bits;
                        constexpr static const std::size_t digest_bits = policy_type::digest_bits;
                    };

                    typedef ::nil::crypto3::hash::detail::finalizer<typename params_type::digest_endian, typename policy_type::word_type, 
                                                              policy_type::word_bits, policy_type::block_bits> 
                        finalizer;
                    typedef haifa_construction<params_type, typename policy_type::iv_generator,
                                               blake2b_compressor<DigestBits>, finalizer>
                        type;

                    constexpr static const std::size_t word_bits = policy_type::word_bits;
                    typedef typename policy_type::word_type word_type;
                };

                template<typename StateAccumulator, std::size_t ValueBits>
                struct stream_processor {
                    struct params_type {
                        typedef typename stream_endian::little_octet_big_bit endian;

                        constexpr static const std::size_t length_bits = construction::params_type::length_bits;
                        constexpr static const std::size_t value_bits = ValueBits;
                    };

                    typedef haifa_stream_processor<construction, StateAccumulator, params_type> type;
                };

                constexpr static const std::size_t digest_bits = DigestBits;
                typedef typename policy_type::digest_type digest_type;
            };
        }    // namespace hash
    }        // namespace crypto3
}    // namespace nil

#endif
