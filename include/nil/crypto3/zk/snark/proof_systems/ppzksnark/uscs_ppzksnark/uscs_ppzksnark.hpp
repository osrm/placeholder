//---------------------------------------------------------------------------//
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//
// @file Declaration of interfaces for a ppzkSNARK for USCS.
//
// This includes:
// - class for proving key
// - class for verification key
// - class for processed verification key
// - class for key pair (proving key & verification key)
// - class for proof
// - generator algorithm
// - prover algorithm
// - verifier algorithm (with strong or weak input consistency)
// - online verifier algorithm (with strong or weak input consistency)
//
// The implementation instantiates the protocol of \[DFGK14], by following
// extending, and optimizing the approach described in \[BCTV14].
//
//
// Acronyms:
//
// - "ppzkSNARK" = "Pre-Processing Zero-Knowledge Succinct Non-interactive ARgument of Knowledge"
// - "USCS" = "Unitary-Square Constraint Systems"
//
// References:
//
// \[BCTV14]:
// "Succinct Non-Interactive Zero Knowledge for a von Neumann Architecture",
// Eli Ben-Sasson, Alessandro Chiesa, Eran Tromer, Madars Virza,
// USENIX Security 2014,
// <http://eprint.iacr.org/2013/879>
//
// \[DFGK14]:
// "Square Span Programs with Applications to Succinct NIZK Arguments"
// George Danezis, Cedric Fournet, Jens Groth, Markulf Kohlweiss,
// ASIACRYPT 2014,
// <http://eprint.iacr.org/2014/718>
//---------------------------------------------------------------------------//

#ifndef USCS_PPZKSNARK_HPP_
#define USCS_PPZKSNARK_HPP_

#include <memory>

#include <nil/crypto3/zk/snark/accumulation_vector.hpp>
#include <nil/crypto3/zk/snark/knowledge_commitment/knowledge_commitment.hpp>
#include <nil/crypto3/zk/snark/relations/constraint_satisfaction_problems/uscs/uscs.hpp>
#include <nil/crypto3/zk/snark/proof_systems/ppzksnark/uscs_ppzksnark/uscs_ppzksnark_params.hpp>

#include <nil/algebra/scalar_multiplication/multiexp.hpp>

#ifdef MULTICORE
#include <omp.h>
#endif

#include <nil/crypto3/zk/snark/reductions/uscs_to_ssp/uscs_to_ssp.hpp>
#include <nil/crypto3/zk/snark/relations/arithmetic_programs/ssp/ssp.hpp>

namespace nil {
    namespace crypto3 {
        namespace zk {
            namespace snark {

                /******************************** Proving key ********************************/

                template<typename ppT>
                class uscs_ppzksnark_proving_key;

                template<typename ppT>
                std::ostream &operator<<(std::ostream &out, const uscs_ppzksnark_proving_key<ppT> &pk);

                template<typename ppT>
                std::istream &operator>>(std::istream &in, uscs_ppzksnark_proving_key<ppT> &pk);

                /**
                 * A proving key for the USCS ppzkSNARK.
                 */
                template<typename ppT>
                class uscs_ppzksnark_proving_key {
                public:
                    algebra::G1_vector<ppT> V_g1_query;
                    algebra::G1_vector<ppT> alpha_V_g1_query;
                    algebra::G1_vector<ppT> H_g1_query;
                    algebra::G2_vector<ppT> V_g2_query;

                    uscs_ppzksnark_constraint_system<ppT> constraint_system;

                    uscs_ppzksnark_proving_key() {};
                    uscs_ppzksnark_proving_key<ppT> &operator=(const uscs_ppzksnark_proving_key<ppT> &other) = default;
                    uscs_ppzksnark_proving_key(const uscs_ppzksnark_proving_key<ppT> &other) = default;
                    uscs_ppzksnark_proving_key(uscs_ppzksnark_proving_key<ppT> &&other) = default;
                    uscs_ppzksnark_proving_key(algebra::G1_vector<ppT> &&V_g1_query,
                                               algebra::G1_vector<ppT> &&alpha_V_g1_query,
                                               algebra::G1_vector<ppT> &&H_g1_query,
                                               algebra::G2_vector<ppT> &&V_g2_query,
                                               uscs_ppzksnark_constraint_system<ppT> &&constraint_system) :
                        V_g1_query(std::move(V_g1_query)),
                        alpha_V_g1_query(std::move(alpha_V_g1_query)), H_g1_query(std::move(H_g1_query)),
                        V_g2_query(std::move(V_g2_query)), constraint_system(std::move(constraint_system)) {};

                    std::size_t G1_size() const {
                        return V_g1_query.size() + alpha_V_g1_query.size() + H_g1_query.size();
                    }

                    std::size_t G2_size() const {
                        return V_g2_query.size();
                    }

                    std::size_t G1_sparse_size() const {
                        return G1_size();
                    }

                    std::size_t G2_sparse_size() const {
                        return G2_size();
                    }

                    std::size_t size_in_bits() const {
                        return algebra::G1<ppT>::size_in_bits() * G1_size() +
                               algebra::G2<ppT>::size_in_bits() * G2_size();
                    }

                    void print_size() const {
                        algebra::print_indent();
                        printf("* G1 elements in PK: %zu\n", this->G1_size());
                        algebra::print_indent();
                        printf("* Non-zero G1 elements in PK: %zu\n", this->G1_sparse_size());
                        algebra::print_indent();
                        printf("* G2 elements in PK: %zu\n", this->G2_size());
                        algebra::print_indent();
                        printf("* Non-zero G2 elements in PK: %zu\n", this->G2_sparse_size());
                        algebra::print_indent();
                        printf("* PK size in bits: %zu\n", this->size_in_bits());
                    }

                    bool operator==(const uscs_ppzksnark_proving_key<ppT> &other) const;
                    friend std::ostream &operator<<<ppT>(std::ostream &out, const uscs_ppzksnark_proving_key<ppT> &pk);
                    friend std::istream &operator>><ppT>(std::istream &in, uscs_ppzksnark_proving_key<ppT> &pk);
                };

                /******************************* Verification key ****************************/

                template<typename ppT>
                class uscs_ppzksnark_verification_key;

                template<typename ppT>
                std::ostream &operator<<(std::ostream &out, const uscs_ppzksnark_verification_key<ppT> &vk);

                template<typename ppT>
                std::istream &operator>>(std::istream &in, uscs_ppzksnark_verification_key<ppT> &vk);

                /**
                 * A verification key for the USCS ppzkSNARK.
                 */
                template<typename ppT>
                class uscs_ppzksnark_verification_key {
                public:
                    algebra::G2<ppT> tilde_g2;
                    algebra::G2<ppT> alpha_tilde_g2;
                    algebra::G2<ppT> Z_g2;

                    accumulation_vector<algebra::G1<ppT>> encoded_IC_query;

                    uscs_ppzksnark_verification_key() = default;
                    uscs_ppzksnark_verification_key(const algebra::G2<ppT> &tilde_g2,
                                                    const algebra::G2<ppT> &alpha_tilde_g2,
                                                    const algebra::G2<ppT> &Z_g2,
                                                    const accumulation_vector<algebra::G1<ppT>> &eIC) :
                        tilde_g2(tilde_g2),
                        alpha_tilde_g2(alpha_tilde_g2), Z_g2(Z_g2), encoded_IC_query(eIC) {};

                    std::size_t G1_size() const {
                        return encoded_IC_query.size();
                    }

                    std::size_t G2_size() const {
                        return 3;
                    }

                    std::size_t size_in_bits() const {
                        return encoded_IC_query.size_in_bits() + 3 * algebra::G2<ppT>::size_in_bits();
                    }

                    void print_size() const {
                        algebra::print_indent();
                        printf("* G1 elements in VK: %zu\n", this->G1_size());
                        algebra::print_indent();
                        printf("* G2 elements in VK: %zu\n", this->G2_size());
                        algebra::print_indent();
                        printf("* VK size in bits: %zu\n", this->size_in_bits());
                    }

                    bool operator==(const uscs_ppzksnark_verification_key<ppT> &other) const;
                    friend std::ostream &operator<<<ppT>(std::ostream &out,
                                                         const uscs_ppzksnark_verification_key<ppT> &vk);
                    friend std::istream &operator>><ppT>(std::istream &in, uscs_ppzksnark_verification_key<ppT> &vk);

                    static uscs_ppzksnark_verification_key<ppT> dummy_verification_key(const std::size_t input_size);
                };

                /************************ Processed verification key *************************/

                template<typename ppT>
                class uscs_ppzksnark_processed_verification_key;

                template<typename ppT>
                std::ostream &operator<<(std::ostream &out, const uscs_ppzksnark_processed_verification_key<ppT> &pvk);

                template<typename ppT>
                std::istream &operator>>(std::istream &in, uscs_ppzksnark_processed_verification_key<ppT> &pvk);

                /**
                 * A processed verification key for the USCS ppzkSNARK.
                 *
                 * Compared to a (non-processed) verification key, a processed verification key
                 * contains a small constant amount of additional pre-computed information that
                 * enables a faster verification time.
                 */
                template<typename ppT>
                class uscs_ppzksnark_processed_verification_key {
                public:
                    algebra::G1_precomp<ppT> pp_G1_one_precomp;
                    algebra::G2_precomp<ppT> pp_G2_one_precomp;
                    algebra::G2_precomp<ppT> vk_tilde_g2_precomp;
                    algebra::G2_precomp<ppT> vk_alpha_tilde_g2_precomp;
                    algebra::G2_precomp<ppT> vk_Z_g2_precomp;
                    algebra::GT<ppT> pairing_of_g1_and_g2;

                    accumulation_vector<algebra::G1<ppT>> encoded_IC_query;

                    bool operator==(const uscs_ppzksnark_processed_verification_key &other) const;
                    friend std::ostream &operator<<<ppT>(std::ostream &out,
                                                         const uscs_ppzksnark_processed_verification_key<ppT> &pvk);
                    friend std::istream &operator>>
                        <ppT>(std::istream &in, uscs_ppzksnark_processed_verification_key<ppT> &pvk);
                };

                /********************************** Key pair *********************************/

                /**
                 * A key pair for the USCS ppzkSNARK, which consists of a proving key and a verification key.
                 */
                template<typename ppT>
                class uscs_ppzksnark_keypair {
                public:
                    uscs_ppzksnark_proving_key<ppT> pk;
                    uscs_ppzksnark_verification_key<ppT> vk;

                    uscs_ppzksnark_keypair() {};
                    uscs_ppzksnark_keypair(uscs_ppzksnark_proving_key<ppT> &&pk,
                                           uscs_ppzksnark_verification_key<ppT> &&vk) :
                        pk(std::move(pk)),
                        vk(std::move(vk)) {
                    }

                    uscs_ppzksnark_keypair(uscs_ppzksnark_keypair<ppT> &&other) = default;
                };

                /*********************************** Proof ***********************************/

                template<typename ppT>
                class uscs_ppzksnark_proof;

                template<typename ppT>
                std::ostream &operator<<(std::ostream &out, const uscs_ppzksnark_proof<ppT> &proof);

                template<typename ppT>
                std::istream &operator>>(std::istream &in, uscs_ppzksnark_proof<ppT> &proof);

                /**
                 * A proof for the USCS ppzkSNARK.
                 *
                 * While the proof has a structure, externally one merely opaquely produces,
                 * serializes/deserializes, and verifies proofs. We only expose some information
                 * about the structure for statistics purposes.
                 */
                template<typename ppT>
                class uscs_ppzksnark_proof {
                public:
                    algebra::G1<ppT> V_g1;
                    algebra::G1<ppT> alpha_V_g1;
                    algebra::G1<ppT> H_g1;
                    algebra::G2<ppT> V_g2;

                    uscs_ppzksnark_proof() {
                        // invalid proof with valid curve points
                        this->V_g1 = algebra::G1<ppT>::one();
                        this->alpha_V_g1 = algebra::G1<ppT>::one();
                        this->H_g1 = algebra::G1<ppT>::one();
                        this->V_g2 = algebra::G2<ppT>::one();
                    }
                    uscs_ppzksnark_proof(algebra::G1<ppT> &&V_g1,
                                         algebra::G1<ppT> &&alpha_V_g1,
                                         algebra::G1<ppT> &&H_g1,
                                         algebra::G2<ppT> &&V_g2) :
                        V_g1(std::move(V_g1)),
                        alpha_V_g1(std::move(alpha_V_g1)), H_g1(std::move(H_g1)), V_g2(std::move(V_g2)) {};

                    std::size_t G1_size() const {
                        return 3;
                    }

                    std::size_t G2_size() const {
                        return 1;
                    }

                    std::size_t size_in_bits() const {
                        return G1_size() * algebra::G1<ppT>::size_in_bits() +
                               G2_size() * algebra::G2<ppT>::size_in_bits();
                    }

                    void print_size() const {
                        algebra::print_indent();
                        printf("* G1 elements in proof: %zu\n", this->G1_size());
                        algebra::print_indent();
                        printf("* G2 elements in proof: %zu\n", this->G2_size());
                        algebra::print_indent();
                        printf("* Proof size in bits: %zu\n", this->size_in_bits());
                    }

                    bool is_well_formed() const {
                        return (V_g1.is_well_formed() && alpha_V_g1.is_well_formed() && H_g1.is_well_formed() &&
                                V_g2.is_well_formed());
                    }

                    bool operator==(const uscs_ppzksnark_proof<ppT> &other) const;
                    friend std::ostream &operator<<<ppT>(std::ostream &out, const uscs_ppzksnark_proof<ppT> &proof);
                    friend std::istream &operator>><ppT>(std::istream &in, uscs_ppzksnark_proof<ppT> &proof);
                };

                /***************************** Main algorithms *******************************/

                /**
                 * A generator algorithm for the USCS ppzkSNARK.
                 *
                 * Given a USCS constraint system CS, this algorithm produces proving and verification keys for CS.
                 */
                template<typename ppT>
                uscs_ppzksnark_keypair<ppT> uscs_ppzksnark_generator(const uscs_ppzksnark_constraint_system<ppT> &cs);

                /**
                 * A prover algorithm for the USCS ppzkSNARK.
                 *
                 * Given a USCS primary input X and a USCS auxiliary input Y, this algorithm
                 * produces a proof (of knowledge) that attests to the following statement:
                 *               ``there exists Y such that CS(X,Y)=0''.
                 * Above, CS is the USCS constraint system that was given as input to the generator algorithm.
                 */
                template<typename ppT>
                uscs_ppzksnark_proof<ppT>
                    uscs_ppzksnark_prover(const uscs_ppzksnark_proving_key<ppT> &pk,
                                          const uscs_ppzksnark_primary_input<ppT> &primary_input,
                                          const uscs_ppzksnark_auxiliary_input<ppT> &auxiliary_input);

                /*
                 Below are four variants of verifier algorithm for the USCS ppzkSNARK.

                 These are the four cases that arise from the following two choices:

                 (1) The verifier accepts a (non-processed) verification key or, instead, a processed verification key.
                     In the latter case, we call the algorithm an "online verifier".

                 (2) The verifier checks for "weak" input consistency or, instead, "strong" input consistency.
                     Strong input consistency requires that |primary_input| = CS.num_inputs, whereas
                     weak input consistency requires that |primary_input| <= CS.num_inputs (and
                     the primary input is implicitly padded with zeros up to length CS.num_inputs).
                 */

                /**
                 * A verifier algorithm for the USCS ppzkSNARK that:
                 * (1) accepts a non-processed verification key, and
                 * (2) has weak input consistency.
                 */
                template<typename ppT>
                bool uscs_ppzksnark_verifier_weak_IC(const uscs_ppzksnark_verification_key<ppT> &vk,
                                                     const uscs_ppzksnark_primary_input<ppT> &primary_input,
                                                     const uscs_ppzksnark_proof<ppT> &proof);

                /**
                 * A verifier algorithm for the USCS ppzkSNARK that:
                 * (1) accepts a non-processed verification key, and
                 * (2) has strong input consistency.
                 */
                template<typename ppT>
                bool uscs_ppzksnark_verifier_strong_IC(const uscs_ppzksnark_verification_key<ppT> &vk,
                                                       const uscs_ppzksnark_primary_input<ppT> &primary_input,
                                                       const uscs_ppzksnark_proof<ppT> &proof);

                /**
                 * Convert a (non-processed) verification key into a processed verification key.
                 */
                template<typename ppT>
                uscs_ppzksnark_processed_verification_key<ppT>
                    uscs_ppzksnark_verifier_process_vk(const uscs_ppzksnark_verification_key<ppT> &vk);

                /**
                 * A verifier algorithm for the USCS ppzkSNARK that:
                 * (1) accepts a processed verification key, and
                 * (2) has weak input consistency.
                 */
                template<typename ppT>
                bool uscs_ppzksnark_online_verifier_weak_IC(const uscs_ppzksnark_processed_verification_key<ppT> &pvk,
                                                            const uscs_ppzksnark_primary_input<ppT> &primary_input,
                                                            const uscs_ppzksnark_proof<ppT> &proof);

                /**
                 * A verifier algorithm for the USCS ppzkSNARK that:
                 * (1) accepts a processed verification key, and
                 * (2) has strong input consistency.
                 */
                template<typename ppT>
                bool uscs_ppzksnark_online_verifier_strong_IC(const uscs_ppzksnark_processed_verification_key<ppT> &pvk,
                                                              const uscs_ppzksnark_primary_input<ppT> &primary_input,
                                                              const uscs_ppzksnark_proof<ppT> &proof);

                template<typename ppT>
                bool uscs_ppzksnark_proving_key<ppT>::operator==(const uscs_ppzksnark_proving_key<ppT> &other) const {
                    return (this->V_g1_query == other.V_g1_query && this->alpha_V_g1_query == other.alpha_V_g1_query &&
                            this->H_g1_query == other.H_g1_query && this->V_g2_query == other.V_g2_query &&
                            this->constraint_system == other.constraint_system);
                }

                template<typename ppT>
                std::ostream &operator<<(std::ostream &out, const uscs_ppzksnark_proving_key<ppT> &pk) {
                    out << pk.V_g1_query;
                    out << pk.alpha_V_g1_query;
                    out << pk.H_g1_query;
                    out << pk.V_g2_query;
                    out << pk.constraint_system;

                    return out;
                }

                template<typename ppT>
                std::istream &operator>>(std::istream &in, uscs_ppzksnark_proving_key<ppT> &pk) {
                    in >> pk.V_g1_query;
                    in >> pk.alpha_V_g1_query;
                    in >> pk.H_g1_query;
                    in >> pk.V_g2_query;
                    in >> pk.constraint_system;

                    return in;
                }

                template<typename ppT>
                bool uscs_ppzksnark_verification_key<ppT>::operator==(
                    const uscs_ppzksnark_verification_key<ppT> &other) const {
                    return (this->tilde_g2 == other.tilde_g2 && this->alpha_tilde_g2 == other.alpha_tilde_g2 &&
                            this->Z_g2 == other.Z_g2 && this->encoded_IC_query == other.encoded_IC_query);
                }

                template<typename ppT>
                std::ostream &operator<<(std::ostream &out, const uscs_ppzksnark_verification_key<ppT> &vk) {
                    out << vk.tilde_g2 << OUTPUT_NEWLINE;
                    out << vk.alpha_tilde_g2 << OUTPUT_NEWLINE;
                    out << vk.Z_g2 << OUTPUT_NEWLINE;
                    out << vk.encoded_IC_query << OUTPUT_NEWLINE;

                    return out;
                }

                template<typename ppT>
                std::istream &operator>>(std::istream &in, uscs_ppzksnark_verification_key<ppT> &vk) {
                    in >> vk.tilde_g2;
                    algebra::consume_OUTPUT_NEWLINE(in);
                    in >> vk.alpha_tilde_g2;
                    algebra::consume_OUTPUT_NEWLINE(in);
                    in >> vk.Z_g2;
                    algebra::consume_OUTPUT_NEWLINE(in);
                    in >> vk.encoded_IC_query;
                    algebra::consume_OUTPUT_NEWLINE(in);

                    return in;
                }

                template<typename ppT>
                bool uscs_ppzksnark_processed_verification_key<ppT>::operator==(
                    const uscs_ppzksnark_processed_verification_key<ppT> &other) const {
                    return (this->pp_G1_one_precomp == other.pp_G1_one_precomp &&
                            this->pp_G2_one_precomp == other.pp_G2_one_precomp &&
                            this->vk_tilde_g2_precomp == other.vk_tilde_g2_precomp &&
                            this->vk_alpha_tilde_g2_precomp == other.vk_alpha_tilde_g2_precomp &&
                            this->vk_Z_g2_precomp == other.vk_Z_g2_precomp &&
                            this->pairing_of_g1_and_g2 == other.pairing_of_g1_and_g2 &&
                            this->encoded_IC_query == other.encoded_IC_query);
                }

                template<typename ppT>
                std::ostream &operator<<(std::ostream &out, const uscs_ppzksnark_processed_verification_key<ppT> &pvk) {
                    out << pvk.pp_G1_one_precomp << OUTPUT_NEWLINE;
                    out << pvk.pp_G2_one_precomp << OUTPUT_NEWLINE;
                    out << pvk.vk_tilde_g2_precomp << OUTPUT_NEWLINE;
                    out << pvk.vk_alpha_tilde_g2_precomp << OUTPUT_NEWLINE;
                    out << pvk.vk_Z_g2_precomp << OUTPUT_NEWLINE;
                    out << pvk.pairing_of_g1_and_g2 << OUTPUT_NEWLINE;
                    out << pvk.encoded_IC_query << OUTPUT_NEWLINE;

                    return out;
                }

                template<typename ppT>
                std::istream &operator>>(std::istream &in, uscs_ppzksnark_processed_verification_key<ppT> &pvk) {
                    in >> pvk.pp_G1_one_precomp;
                    algebra::consume_OUTPUT_NEWLINE(in);
                    in >> pvk.pp_G2_one_precomp;
                    algebra::consume_OUTPUT_NEWLINE(in);
                    in >> pvk.vk_tilde_g2_precomp;
                    algebra::consume_OUTPUT_NEWLINE(in);
                    in >> pvk.vk_alpha_tilde_g2_precomp;
                    algebra::consume_OUTPUT_NEWLINE(in);
                    in >> pvk.vk_Z_g2_precomp;
                    algebra::consume_OUTPUT_NEWLINE(in);
                    in >> pvk.pairing_of_g1_and_g2;
                    algebra::consume_OUTPUT_NEWLINE(in);
                    in >> pvk.encoded_IC_query;
                    algebra::consume_OUTPUT_NEWLINE(in);

                    return in;
                }

                template<typename ppT>
                bool uscs_ppzksnark_proof<ppT>::operator==(const uscs_ppzksnark_proof<ppT> &other) const {
                    return (this->V_g1 == other.V_g1 && this->alpha_V_g1 == other.alpha_V_g1 &&
                            this->H_g1 == other.H_g1 && this->V_g2 == other.V_g2);
                }

                template<typename ppT>
                std::ostream &operator<<(std::ostream &out, const uscs_ppzksnark_proof<ppT> &proof) {
                    out << proof.V_g1 << OUTPUT_NEWLINE;
                    out << proof.alpha_V_g1 << OUTPUT_NEWLINE;
                    out << proof.H_g1 << OUTPUT_NEWLINE;
                    out << proof.V_g2 << OUTPUT_NEWLINE;

                    return out;
                }

                template<typename ppT>
                std::istream &operator>>(std::istream &in, uscs_ppzksnark_proof<ppT> &proof) {
                    in >> proof.V_g1;
                    algebra::consume_OUTPUT_NEWLINE(in);
                    in >> proof.alpha_V_g1;
                    algebra::consume_OUTPUT_NEWLINE(in);
                    in >> proof.H_g1;
                    algebra::consume_OUTPUT_NEWLINE(in);
                    in >> proof.V_g2;
                    algebra::consume_OUTPUT_NEWLINE(in);

                    return in;
                }

                template<typename ppT>
                uscs_ppzksnark_verification_key<ppT>
                    uscs_ppzksnark_verification_key<ppT>::dummy_verification_key(const std::size_t input_size) {
                    uscs_ppzksnark_verification_key<ppT> result;
                    result.tilde_g2 = algebra::Fr<ppT>::random_element() * algebra::G2<ppT>::one();
                    result.alpha_tilde_g2 = algebra::Fr<ppT>::random_element() * algebra::G2<ppT>::one();
                    result.Z_g2 = algebra::Fr<ppT>::random_element() * algebra::G2<ppT>::one();

                    algebra::G1<ppT> base = algebra::Fr<ppT>::random_element() * algebra::G1<ppT>::one();
                    algebra::G1_vector<ppT> v;
                    for (std::size_t i = 0; i < input_size; ++i) {
                        v.emplace_back(algebra::Fr<ppT>::random_element() * algebra::G1<ppT>::one());
                    }

                    result.encoded_IC_query = accumulation_vector<algebra::G1<ppT>>(v);

                    return result;
                }

                template<typename ppT>
                uscs_ppzksnark_keypair<ppT> uscs_ppzksnark_generator(const uscs_ppzksnark_constraint_system<ppT> &cs) {

                    /* draw random element at which the SSP is evaluated */

                    const algebra::Fr<ppT> t = algebra::Fr<ppT>::random_element();

                    /* perform USCS-to-SSP reduction */

                    ssp_instance_evaluation<algebra::Fr<ppT>> ssp_inst =
                        uscs_to_ssp_instance_map_with_evaluation(cs, t);

                    /* construct various tables of FieldType elements */

                    algebra::Fr_vector<ppT> Vt_table = std::move(
                        ssp_inst.Vt);    // ssp_inst.Vt is now in unspecified state, but we do not use it later
                    algebra::Fr_vector<ppT> Ht_table = std::move(
                        ssp_inst.Ht);    // ssp_inst.Ht is now in unspecified state, but we do not use it later

                    Vt_table.emplace_back(ssp_inst.Zt);

                    algebra::Fr_vector<ppT> Xt_table =
                        algebra::Fr_vector<ppT>(Vt_table.begin(), Vt_table.begin() + ssp_inst.num_inputs() + 1);
                    algebra::Fr_vector<ppT> Vt_table_minus_Xt_table =
                        algebra::Fr_vector<ppT>(Vt_table.begin() + ssp_inst.num_inputs() + 1, Vt_table.end());

                    /* sanity checks */

                    assert(Vt_table.size() == ssp_inst.num_variables() + 2);
                    assert(Ht_table.size() == ssp_inst.degree() + 1);
                    assert(Xt_table.size() == ssp_inst.num_inputs() + 1);
                    assert(Vt_table_minus_Xt_table.size() == ssp_inst.num_variables() + 2 - ssp_inst.num_inputs() - 1);
                    for (std::size_t i = 0; i < ssp_inst.num_inputs() + 1; ++i) {
                        assert(!Xt_table[i].is_zero());
                    }

                    const algebra::Fr<ppT> alpha = algebra::Fr<ppT>::random_element();

                    const std::size_t g1_exp_count = Vt_table.size() + Vt_table_minus_Xt_table.size() + Ht_table.size();
                    const std::size_t g2_exp_count = Vt_table_minus_Xt_table.size();

                    std::size_t g1_window = algebra::get_exp_window_size<algebra::G1<ppT>>(g1_exp_count);
                    std::size_t g2_window = algebra::get_exp_window_size<algebra::G2<ppT>>(g2_exp_count);

                    algebra::window_table<algebra::G1<ppT>> g1_table =
                        get_window_table(algebra::Fr<ppT>::size_in_bits(), g1_window, algebra::G1<ppT>::one());

                    algebra::window_table<algebra::G2<ppT>> g2_table =
                        get_window_table(algebra::Fr<ppT>::size_in_bits(), g2_window, algebra::G2<ppT>::one());

                    algebra::G1_vector<ppT> V_g1_query =
                        batch_exp(algebra::Fr<ppT>::size_in_bits(), g1_window, g1_table, Vt_table_minus_Xt_table);
#ifdef USE_MIXED_ADDITION
                    algebra::batch_to_special<algebra::G1<ppT>>(V_g1_query);
#endif

                    algebra::G1_vector<ppT> alpha_V_g1_query = batch_exp_with_coeff(
                        algebra::Fr<ppT>::size_in_bits(), g1_window, g1_table, alpha, Vt_table_minus_Xt_table);
#ifdef USE_MIXED_ADDITION
                    algebra::batch_to_special<algebra::G1<ppT>>(alpha_V_g1_query);
#endif

                    algebra::G1_vector<ppT> H_g1_query =
                        batch_exp(algebra::Fr<ppT>::size_in_bits(), g1_window, g1_table, Ht_table);
#ifdef USE_MIXED_ADDITION
                    algebra::batch_to_special<algebra::G1<ppT>>(H_g1_query);
#endif

                    algebra::G2_vector<ppT> V_g2_query =
                        batch_exp(algebra::Fr<ppT>::size_in_bits(), g2_window, g2_table, Vt_table);
#ifdef USE_MIXED_ADDITION
                    algebra::batch_to_special<algebra::G2<ppT>>(V_g2_query);
#endif
                    const algebra::Fr<ppT> tilde = algebra::Fr<ppT>::random_element();
                    algebra::G2<ppT> tilde_g2 = tilde * algebra::G2<ppT>::one();
                    algebra::G2<ppT> alpha_tilde_g2 = (alpha * tilde) * algebra::G2<ppT>::one();
                    algebra::G2<ppT> Z_g2 = ssp_inst.Zt * algebra::G2<ppT>::one();

                    algebra::G1<ppT> encoded_IC_base = Xt_table[0] * algebra::G1<ppT>::one();
                    algebra::G1_vector<ppT> encoded_IC_values =
                        batch_exp(algebra::Fr<ppT>::size_in_bits(), g1_window, g1_table,
                                  algebra::Fr_vector<ppT>(Xt_table.begin() + 1, Xt_table.end()));

                    accumulation_vector<algebra::G1<ppT>> encoded_IC_query(std::move(encoded_IC_base),
                                                                           std::move(encoded_IC_values));

                    uscs_ppzksnark_verification_key<ppT> vk =
                        uscs_ppzksnark_verification_key<ppT>(tilde_g2, alpha_tilde_g2, Z_g2, encoded_IC_query);

                    uscs_ppzksnark_constraint_system<ppT> cs_copy = cs;
                    uscs_ppzksnark_proving_key<ppT> pk = uscs_ppzksnark_proving_key<ppT>(std::move(V_g1_query),
                                                                                         std::move(alpha_V_g1_query),
                                                                                         std::move(H_g1_query),
                                                                                         std::move(V_g2_query),
                                                                                         std::move(cs_copy));

                    pk.print_size();
                    vk.print_size();

                    return uscs_ppzksnark_keypair<ppT>(std::move(pk), std::move(vk));
                }

                template<typename ppT>
                uscs_ppzksnark_proof<ppT>
                    uscs_ppzksnark_prover(const uscs_ppzksnark_proving_key<ppT> &pk,
                                          const uscs_ppzksnark_primary_input<ppT> &primary_input,
                                          const uscs_ppzksnark_auxiliary_input<ppT> &auxiliary_input) {

                    const algebra::Fr<ppT> d = algebra::Fr<ppT>::random_element();

                    const ssp_witness<algebra::Fr<ppT>> ssp_wit =
                        uscs_to_ssp_witness_map(pk.constraint_system, primary_input, auxiliary_input, d);

                    /* sanity checks */
                    assert(pk.constraint_system.is_satisfied(primary_input, auxiliary_input));
                    assert(pk.V_g1_query.size() == ssp_wit.num_variables() + 2 - ssp_wit.num_inputs() - 1);
                    assert(pk.alpha_V_g1_query.size() == ssp_wit.num_variables() + 2 - ssp_wit.num_inputs() - 1);
                    assert(pk.H_g1_query.size() == ssp_wit.degree() + 1);
                    assert(pk.V_g2_query.size() == ssp_wit.num_variables() + 2);

                    algebra::G1<ppT> V_g1 = ssp_wit.d * pk.V_g1_query[pk.V_g1_query.size() - 1];
                    algebra::G1<ppT> alpha_V_g1 = ssp_wit.d * pk.alpha_V_g1_query[pk.alpha_V_g1_query.size() - 1];
                    algebra::G1<ppT> H_g1 = algebra::G1<ppT>::zero();
                    algebra::G2<ppT> V_g2 = pk.V_g2_query[0] + ssp_wit.d * pk.V_g2_query[pk.V_g2_query.size() - 1];

#ifdef MULTICORE
                    const std::size_t chunks = omp_get_max_threads();    // to override, set OMP_NUM_THREADS env var or call
                                                                    // omp_set_num_threads()
#else
                    const std::size_t chunks = 1;
#endif

                    // MAYBE LATER: do queries 1,2,4 at once for slightly better speed

                    V_g1 = V_g1 + algebra::multi_exp_with_mixed_addition<algebra::G1<ppT>, algebra::Fr<ppT>,
                                                                         algebra::multi_exp_method_BDLO12>(
                                      pk.V_g1_query.begin(),
                                      pk.V_g1_query.begin() + (ssp_wit.num_variables() - ssp_wit.num_inputs()),
                                      ssp_wit.coefficients_for_Vs.begin() + ssp_wit.num_inputs(),
                                      ssp_wit.coefficients_for_Vs.begin() + ssp_wit.num_variables(), chunks);

                    alpha_V_g1 =
                        alpha_V_g1 + algebra::multi_exp_with_mixed_addition<algebra::G1<ppT>, algebra::Fr<ppT>,
                                                                            algebra::multi_exp_method_BDLO12>(
                                         pk.alpha_V_g1_query.begin(),
                                         pk.alpha_V_g1_query.begin() + (ssp_wit.num_variables() - ssp_wit.num_inputs()),
                                         ssp_wit.coefficients_for_Vs.begin() + ssp_wit.num_inputs(),
                                         ssp_wit.coefficients_for_Vs.begin() + ssp_wit.num_variables(), chunks);

                    H_g1 =
                        H_g1 + algebra::multi_exp<algebra::G1<ppT>, algebra::Fr<ppT>, algebra::multi_exp_method_BDLO12>(
                                   pk.H_g1_query.begin(), pk.H_g1_query.begin() + ssp_wit.degree() + 1,
                                   ssp_wit.coefficients_for_H.begin(),
                                   ssp_wit.coefficients_for_H.begin() + ssp_wit.degree() + 1, chunks);

                    V_g2 =
                        V_g2 + algebra::multi_exp<algebra::G2<ppT>, algebra::Fr<ppT>, algebra::multi_exp_method_BDLO12>(
                                   pk.V_g2_query.begin() + 1, pk.V_g2_query.begin() + ssp_wit.num_variables() + 1,
                                   ssp_wit.coefficients_for_Vs.begin(),
                                   ssp_wit.coefficients_for_Vs.begin() + ssp_wit.num_variables(), chunks);

                    uscs_ppzksnark_proof<ppT> proof = uscs_ppzksnark_proof<ppT>(std::move(V_g1), std::move(alpha_V_g1),
                                                                                std::move(H_g1), std::move(V_g2));

                    return proof;
                }

                template<typename ppT>
                uscs_ppzksnark_processed_verification_key<ppT>
                    uscs_ppzksnark_verifier_process_vk(const uscs_ppzksnark_verification_key<ppT> &vk) {
                    uscs_ppzksnark_processed_verification_key<ppT> pvk;

                    pvk.pp_G1_one_precomp = ppT::precompute_G1(algebra::G1<ppT>::one());
                    pvk.pp_G2_one_precomp = ppT::precompute_G2(algebra::G2<ppT>::one());

                    pvk.vk_tilde_g2_precomp = ppT::precompute_G2(vk.tilde_g2);
                    pvk.vk_alpha_tilde_g2_precomp = ppT::precompute_G2(vk.alpha_tilde_g2);
                    pvk.vk_Z_g2_precomp = ppT::precompute_G2(vk.Z_g2);

                    pvk.pairing_of_g1_and_g2 = ppT::miller_loop(pvk.pp_G1_one_precomp, pvk.pp_G2_one_precomp);

                    pvk.encoded_IC_query = vk.encoded_IC_query;

                    return pvk;
                }

                template<typename ppT>
                bool uscs_ppzksnark_online_verifier_weak_IC(const uscs_ppzksnark_processed_verification_key<ppT> &pvk,
                                                            const uscs_ppzksnark_primary_input<ppT> &primary_input,
                                                            const uscs_ppzksnark_proof<ppT> &proof) {
                    assert(pvk.encoded_IC_query.domain_size() >= primary_input.size());

                    const accumulation_vector<algebra::G1<ppT>> accumulated_IC =
                        pvk.encoded_IC_query.template accumulate_chunk<algebra::Fr<ppT>>(primary_input.begin(),
                                                                                         primary_input.end(), 0);
                    assert(accumulated_IC.is_fully_accumulated());
                    const algebra::G1<ppT> &acc = accumulated_IC.first;

                    bool result = true;

                    if (!proof.is_well_formed()) {
                        result = false;
                    }

                    algebra::G1_precomp<ppT> proof_V_g1_with_acc_precomp = ppT::precompute_G1(proof.V_g1 + acc);
                    algebra::G2_precomp<ppT> proof_V_g2_precomp = ppT::precompute_G2(proof.V_g2);
                    algebra::Fqk<ppT> V_1 = ppT::miller_loop(proof_V_g1_with_acc_precomp, pvk.pp_G2_one_precomp);
                    algebra::Fqk<ppT> V_2 = ppT::miller_loop(pvk.pp_G1_one_precomp, proof_V_g2_precomp);
                    algebra::GT<ppT> V = ppT::final_exponentiation(V_1 * V_2.unitary_inverse());
                    if (V != algebra::GT<ppT>::one()) {
                        result = false;
                    }

                    algebra::G1_precomp<ppT> proof_H_g1_precomp = ppT::precompute_G1(proof.H_g1);
                    algebra::Fqk<ppT> SSP_1 = ppT::miller_loop(proof_V_g1_with_acc_precomp, proof_V_g2_precomp);
                    algebra::Fqk<ppT> SSP_2 = ppT::miller_loop(proof_H_g1_precomp, pvk.vk_Z_g2_precomp);
                    algebra::GT<ppT> SSP =
                        ppT::final_exponentiation(SSP_1.unitary_inverse() * SSP_2 * pvk.pairing_of_g1_and_g2);
                    if (SSP != algebra::GT<ppT>::one()) {
                        result = false;
                    }

                    algebra::G1_precomp<ppT> proof_V_g1_precomp = ppT::precompute_G1(proof.V_g1);
                    algebra::G1_precomp<ppT> proof_alpha_V_g1_precomp = ppT::precompute_G1(proof.alpha_V_g1);
                    algebra::Fqk<ppT> alpha_V_1 = ppT::miller_loop(proof_V_g1_precomp, pvk.vk_alpha_tilde_g2_precomp);
                    algebra::Fqk<ppT> alpha_V_2 = ppT::miller_loop(proof_alpha_V_g1_precomp, pvk.vk_tilde_g2_precomp);
                    algebra::GT<ppT> alpha_V = ppT::final_exponentiation(alpha_V_1 * alpha_V_2.unitary_inverse());
                    if (alpha_V != algebra::GT<ppT>::one()) {
                        result = false;
                    }

                    return result;
                }

                template<typename ppT>
                bool uscs_ppzksnark_verifier_weak_IC(const uscs_ppzksnark_verification_key<ppT> &vk,
                                                     const uscs_ppzksnark_primary_input<ppT> &primary_input,
                                                     const uscs_ppzksnark_proof<ppT> &proof) {
                    uscs_ppzksnark_processed_verification_key<ppT> pvk = uscs_ppzksnark_verifier_process_vk<ppT>(vk);
                    bool result = uscs_ppzksnark_online_verifier_weak_IC<ppT>(pvk, primary_input, proof);
                    return result;
                }

                template<typename ppT>
                bool uscs_ppzksnark_online_verifier_strong_IC(const uscs_ppzksnark_processed_verification_key<ppT> &pvk,
                                                              const uscs_ppzksnark_primary_input<ppT> &primary_input,
                                                              const uscs_ppzksnark_proof<ppT> &proof) {
                    bool result = true;

                    if (pvk.encoded_IC_query.domain_size() != primary_input.size()) {
                        result = false;
                    } else {
                        result = uscs_ppzksnark_online_verifier_weak_IC(pvk, primary_input, proof);
                    }

                    return result;
                }

                template<typename ppT>
                bool uscs_ppzksnark_verifier_strong_IC(const uscs_ppzksnark_verification_key<ppT> &vk,
                                                       const uscs_ppzksnark_primary_input<ppT> &primary_input,
                                                       const uscs_ppzksnark_proof<ppT> &proof) {
                    uscs_ppzksnark_processed_verification_key<ppT> pvk = uscs_ppzksnark_verifier_process_vk<ppT>(vk);
                    bool result = uscs_ppzksnark_online_verifier_strong_IC<ppT>(pvk, primary_input, proof);
                    return result;
                }
            }    // namespace snark
        }        // namespace zk
    }            // namespace crypto3
}    // namespace nil

#endif    // USCS_PPZKSNARK_HPP_
