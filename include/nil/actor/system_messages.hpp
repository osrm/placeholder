//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2017-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

#include <cstdint>
#include <type_traits>
#include <vector>

#include <nil/actor/actor_addr.hpp>
#include <nil/actor/deep_to_string.hpp>
#include <nil/actor/fwd.hpp>
#include <nil/actor/group.hpp>
#include <nil/actor/meta/type_name.hpp>
#include <nil/actor/stream_slot.hpp>

namespace nil {
    namespace actor {

        /// Sent to all links when an actor is terminated.
        /// @note Actors can override the default handler by calling
        ///       `self->set_exit_handler(...)`.
        struct exit_msg {
            /// The source of this message, i.e., the terminated actor.
            actor_addr source;

            /// The exit reason of the terminated actor.
            error reason;
        };

        /// @relates exit_msg
        inline bool operator==(const exit_msg &x, const exit_msg &y) noexcept {
            return x.source == y.source && x.reason == y.reason;
        }

        /// @relates exit_msg
        template<class Inspector>
        typename Inspector::result_type inspect(Inspector &f, exit_msg &x) {
            return f(meta::type_name("exit_msg"), x.source, x.reason);
        }

        /// Sent to all actors monitoring an actor when it is terminated.
        struct down_msg {
            /// The source of this message, i.e., the terminated actor.
            actor_addr source;

            /// The exit reason of the terminated actor.
            error reason;
        };

        /// @relates down_msg
        inline bool operator==(const down_msg &x, const down_msg &y) noexcept {
            return x.source == y.source && x.reason == y.reason;
        }

        /// @relates down_msg
        inline bool operator!=(const down_msg &x, const down_msg &y) noexcept {
            return !(x == y);
        }

        /// @relates down_msg
        template<class Inspector>
        typename Inspector::result_type inspect(Inspector &f, down_msg &x) {
            return f(meta::type_name("down_msg"), x.source, x.reason);
        }

        /// Sent to all members of a group when it goes offline.
        struct group_down_msg {
            /// The source of this message, i.e., the now unreachable group.
            group source;
        };

        /// @relates group_down_msg
        template<class Inspector>
        typename Inspector::result_type inspect(Inspector &f, group_down_msg &x) {
            return f(meta::type_name("group_down_msg"), x.source);
        }

        /// Sent to all actors monitoring a node when =nil; Actor loses connection to it.
        struct node_down_msg {
            /// The disconnected node.
            node_id node;

            /// The cause for the disconnection. No error (a default-constructed error
            /// object) indicates an ordinary shutdown.
            error reason;
        };

        /// @relates node_down_msg
        inline bool operator==(const node_down_msg &x, const node_down_msg &y) noexcept {
            return x.node == y.node && x.reason == y.reason;
        }

        /// @relates node_down_msg
        inline bool operator!=(const node_down_msg &x, const node_down_msg &y) noexcept {
            return !(x == y);
        }

        /// @relates node_down_msg
        template<class Inspector>
        typename Inspector::result_type inspect(Inspector &f, node_down_msg &x) {
            return f(meta::type_name("node_down_msg"), x.node, x.reason);
        }

        /// Signalizes a timeout event.
        /// @note This message is handled implicitly by the runtime system.
        struct timeout_msg {
            /// Type of the timeout (usually either "receive" or "cycle").
            std::string type;
            /// Actor-specific timeout ID.
            uint64_t timeout_id;
        };

        /// @relates timeout_msg
        template<class Inspector>
        typename Inspector::result_type inspect(Inspector &f, timeout_msg &x) {
            return f(meta::type_name("timeout_msg"), x.type, x.timeout_id);
        }

        /// Demands the receiver to open a new stream from the sender to the receiver.
        struct open_stream_msg {
            /// Reserved slot on the source.
            stream_slot slot;

            /// Contains a type-erased stream<T> object as first argument followed by
            /// any number of user-defined additional handshake data.
            message msg;

            /// Identifies the previous stage in the pipeline.
            strong_actor_ptr prev_stage;

            /// Identifies the original receiver of this message.
            strong_actor_ptr original_stage;

            /// Configures the priority for stream elements.
            stream_priority priority;
        };

        /// @relates open_stream_msg
        template<class Inspector>
        typename Inspector::result_type inspect(Inspector &f, open_stream_msg &x) {
            return f(meta::type_name("open_stream_msg"), x.slot, x.msg, x.prev_stage, x.original_stage, x.priority);
        }

    }    // namespace actor
}    // namespace nil
