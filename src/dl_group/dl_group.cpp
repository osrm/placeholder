#include <nil/crypto3/pubkey/dl_group.hpp>

#include <nil/crypto3/multiprecision/modular_inverse.hpp>
#include <nil/crypto3/multiprecision/modular_reduce.hpp>
#include <nil/crypto3/multiprecision/monty.hpp>

#include <nil/crypto3/asn1/der_enc.hpp>
#include <nil/crypto3/pubkey/pem.hpp>
#include <nil/crypto3/pubkey/workfactor.hpp>
#include <nil/crypto3/pubkey/dl_group/dl_group.hpp>

namespace nil {
    namespace crypto3 {

        class dl_group_data final {
        public:
            dl_group_data(const cpp_int &p, const cpp_int &q, const cpp_int &g) : m_p(p), m_q(q), m_g(g), m_mod_p(p),
                    m_monty_params(std::make_shared<montgomery_params>(m_p, m_mod_p)),
                    m_monty(monty_precompute(m_monty_params, m_g, /*window bits=*/4)), m_p_bits(p.bits()),
                    m_estimated_strength(dl_work_factor(m_p_bits)), m_exponent_bits(dl_exponent_size(m_p_bits)) {
            }

            ~dl_group_data() = default;

            dl_group_data(const dl_group_data &other) = delete;

            dl_group_data &operator=(const dl_group_data &other) = delete;

            const cpp_int &p() const {
                return m_p;
            }

            const cpp_int &q() const {
                return m_q;
            }

            const cpp_int &g() const {
                return m_g;
            }

            std::shared_ptr<const montgomery_params> monty_params_p() const {
                return m_monty_params;
            }

            size_t p_bits() const {
                return m_p_bits;
            }

            size_t p_bytes() const {
                return (m_p_bits + 7) / 8;
            }

            cpp_int power_g_p(const cpp_int &k) const {
                return monty_execute(*m_monty, k);
            }

        private:
            cpp_int m_p;
            cpp_int m_q;
            cpp_int m_g;
            modular_reducer m_mod_p;
            std::shared_ptr<const montgomery_params> m_monty_params;
            std::shared_ptr<const montgomery_exponentation_state> m_monty;
            size_t m_p_bits;
            size_t m_estimated_strength;
            size_t m_exponent_bits;
        };

//static
        std::shared_ptr<dl_group_data> dl_group::ber_decode_dl_group(const uint8_t *data, size_t data_len,
                                                                     dl_group::format format) {
            cpp_int p, q, g;

            ber_decoder decoder(data, data_len);
            ber_decoder ber = decoder.start_cons(SEQUENCE);

            if (format == dl_group::ANSI_X9_57) {
                ber.decode(p).decode(q).decode(g).verify_end();
            } else if (format == dl_group::ANSI_X9_42) {
                ber.decode(p).decode(g).decode(q).discard_remaining();
            } else if (format == dl_group::PKCS_3) {
                // q is left as zero
                ber.decode(p).decode(g).discard_remaining();
            } else {
                throw std::invalid_argument("Unknown dl_group encoding " + std::to_string(format));
            }

            return std::make_shared<dl_group_data>(p, q, g);
        }

        const dl_group_data &dl_group::data() const {
            if (m_data) {
                return *m_data;
            }

            throw Invalid_State("dl_group uninitialized");
        }

        std::shared_ptr<const montgomery_params> dl_group::monty_params_p() const {
            return data().monty_params_p();
        }

        size_t dl_group::p_bits() const {
            return data().p_bits();
        }

        size_t dl_group::p_bytes() const {
            return data().p_bytes();
        }

        cpp_int dl_group::multi_exponentiate(const cpp_int &x, const cpp_int &y, const cpp_int &z) const {
            return monty_multi_exp(data().monty_params_p(), get_g(), x, y, z);
        }

        cpp_int dl_group::power_g_p(const cpp_int &x) const {
            return data().power_g_p(x);
        }
    }
}
