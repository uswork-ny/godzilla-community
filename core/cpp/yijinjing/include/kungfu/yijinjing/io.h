/**
 * This is source code modified under the Apache License 2.0.
 * Original Author: Keren Dong
 * Modifier: kx@godzilla.dev
 * Modification date: March 3, 2025
 */

#ifndef KUNGFU_IO_H
#define KUNGFU_IO_H

#include <kungfu/yijinjing/journal/journal.h>
#include <kungfu/yijinjing/nanomsg/socket.h>

namespace kungfu
{

    namespace yijinjing
    {

        class io_device
        {
        public:

            io_device(data::location_ptr home, bool low_latency, bool lazy);

            const data::location_ptr get_home() const
            { return home_; }

            const data::location_ptr get_live_home() const
            { return live_home_; }

            bool is_low_latency()
            { return low_latency_; }

            journal::reader_ptr open_reader_to_subscribe();

            journal::reader_ptr open_reader(const data::location_ptr &location, uint32_t dest_id);

            journal::writer_ptr open_writer(uint32_t dest_id);

            journal::writer_ptr open_writer_at(const data::location_ptr &location, uint32_t dest_id);

            nanomsg::socket_ptr
            connect_socket(const data::location_ptr &location, const nanomsg::protocol &p, int timeout = 0);

            nanomsg::socket_ptr
            bind_socket(const nanomsg::protocol &p, int timeout = 0);

            nanomsg::url_factory_ptr get_url_factory() const
            { return url_factory_; }

            publisher_ptr get_publisher()
            { return publisher_; }

            observer_ptr get_observer()
            { return observer_; }

        protected:
            data::location_ptr home_;
            data::location_ptr live_home_;
            const bool low_latency_;
            const bool lazy_;
            nanomsg::url_factory_ptr url_factory_;
            publisher_ptr publisher_;
            observer_ptr observer_;
        };

        DECLARE_PTR(io_device)

        class io_device_with_reply : public io_device
        {
        public:

            io_device_with_reply(data::location_ptr home, bool low_latency, bool lazy);

            nanomsg::socket_ptr get_rep_sock() const
            { return rep_sock_; }

        protected:
            nanomsg::socket_ptr rep_sock_;
        };

        DECLARE_PTR(io_device_with_reply)

        class io_device_master : public io_device_with_reply
        {
        public:
            io_device_master(data::location_ptr home, bool low_latency);
        };

        DECLARE_PTR(io_device_master)

        class io_device_client : public io_device_with_reply
        {
        public:
            io_device_client(data::location_ptr home, bool low_latency);
        };

        DECLARE_PTR(io_device_client)
    }
}
#endif //KUNGFU_IO_H
