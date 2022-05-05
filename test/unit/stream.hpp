//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef  BOOST_SOCKS_PROTO_TEST_UNIT_STREAM_HPP
#define  BOOST_SOCKS_PROTO_TEST_UNIT_STREAM_HPP

#include <boost/asio/io_context.hpp>
#include <boost/socks_proto/error.hpp>
#include <tuple>

namespace boost {
namespace socks_proto {
namespace test {
namespace detail {
template <class T, T... Ints>
class integer_sequence {};

template <std::size_t... Ints>
using index_sequence =
    integer_sequence<std::size_t, Ints...>;

template <class T, int N, int... Is>
struct sequence_generator
{
    using type =
        typename sequence_generator<
            T, N - 1, N - 1, Is...>::type;
};

template <class T, int... Is>
struct sequence_generator<T, 0, Is...>
{
    using type = integer_sequence<T, Is...>;
};

template <class T, T N>
using make_integer_sequence =
    typename sequence_generator<T, N>::type;

template <std::size_t N>
using make_index_sequence =
    make_integer_sequence<std::size_t, N>;
} // detail

template <class Handler, class... Args>
struct bound_handler {
    explicit
    bound_handler(
        Handler&& handler,
        const Args&... args)
        : handler_(std::move(handler))
        , args_(std::make_tuple(args...))
    {
    }

    void
    operator()()
    {
        apply(
            detail::make_index_sequence<
                std::tuple_size<
                    std::tuple<Args...>
                >::value>{});
    }

    void
    operator()() const
    {
        apply(
            detail::make_index_sequence<
                std::tuple_size<
                    std::tuple<Args...>
                >::value>{});
    }

private:
    template <std::size_t... I>
    void
    apply()
    {
        return invoke_handler(std::get<I>(args_)...);
    }

    void
    invoke_handler(Args&&... args)
    {
        std::move(handler_)(
            static_cast<const Args&>(args)...);
    }

    template <std::size_t... I>
    void
    apply() const
    {
        return invoke_handler(std::get<I>(args_)...);
    }

    void
    invoke_handler(Args&&... args) const
    {
        handler_(args...);
    }

    Handler handler_;
    std::tuple<Args...> args_;
};

template <class Handler, class... Args>
inline
bound_handler<
    typename std::decay<Handler>::type,
    Args...>
bind_handler(Handler&& handler, Args... args)
{
  return bound_handler<
      typename std::decay<Handler>::type,
      Args...>{std::move(handler), args...};
}

// A stream that is SyncReadStream,
// SyncWriteStream, AsyncReadStream,
// and AsyncWriteStream for tests
class stream
{
public:
    using executor_type = asio::io_context::executor_type;

    explicit
    stream(asio::io_context& io_context)
    : ioc_(io_context)
    {
    }

    // AsyncReadStream / AsyncWriteStream
    executor_type
    get_executor() noexcept
    {
        return ioc_.get_executor();
    }

    // SyncReadStream
    template <typename MutableBufferSequence>
    std::size_t
    read_some(const MutableBufferSequence& buffers)
    {
        std::size_t n = asio::buffer_copy(buffers,
            asio::buffer(rbuf_, rsize_) + rpos_,
            rnext_);
        rpos_ += n;
        if (rpos_ == rnext_) {
            rnext_ = rsize_ - rpos_;
        }
        return n;
    }

    // SyncReadStream
    template <typename MutableBufferSequence>
    std::size_t
    read_some(
        const MutableBufferSequence& buffers,
        error_code& ec)
    {
        ec = rec_;
        rec_ = rec2_;
        rec2_ = {};
        std::size_t n = this->read_some(buffers);
        if (!ec.failed() &&
            n != asio::buffer_size(buffers))
            ec = asio::error::eof;
        return n;
    }

    // SyncWriteStream
    template <typename ConstBufferSequence>
    std::size_t
    write_some(const ConstBufferSequence& buffers)
    {
        size_t n = asio::buffer_copy(
            asio::buffer(wbuf_, wsize_) + wpos_,
            buffers, wnext_);
        wpos_ += n;
        return n;
    }

    // SyncWriteStream
    template <typename ConstBufferSequence>
    std::size_t
    write_some(
        const ConstBufferSequence& buffers,
        error_code& ec)
    {
        ec = wec_;
        wec_ = wec2_;
        wec2_ = {};
        return this->write_some(buffers);
    }

    // AsyncReadStream
    template <typename MutableBufferSequence, typename Handler>
    void
    async_read_some(
        const MutableBufferSequence& buffers,
        Handler&& handler)
    {
        error_code ec;
        std::size_t n = read_some(buffers, ec);
        asio::post(
            get_executor(),
            bind_handler(std::move(handler), ec, n));
    }

    // AsyncWriteStream
    template <typename ConstBufferSequence, typename Handler>
    void
    async_write_some(
        const ConstBufferSequence& buffers,
        Handler&& handler)
    {
        error_code ec;
        std::size_t n = write_some(buffers, ec);
        asio::post(get_executor(),
            bind_handler(std::move(handler), ec, n));
    }

    void
    reset_read(const void* data, std::size_t length)
    {
        if (data)
        {
            BOOST_ASSERT(length <= max_cap_);
            std::memcpy(rbuf_, data, length);
        }
        else
        {
            length = 0;
        }
        rsize_ = length;
        rpos_ = 0;
        rnext_ = length;
    }

    void
    append_read(const void* data, std::size_t length)
    {
        if (data)
        {
            BOOST_ASSERT(length <= max_cap_ - rsize_);
            std::memcpy(rbuf_ + rsize_, data, length);
        }
        else
        {
            length = 0;
        }
        rsize_ += length;
        rpos_ = 0;
    }

    void
    reset_write(std::size_t length = max_cap_)
    {
        BOOST_ASSERT(length <= max_cap_);
        memset(wbuf_, 0, max_cap_);
        wsize_ = length;
        wpos_ = 0;
        wnext_ = length;
    }

    void
    reset_read_ec(error_code ec)
    {
        rec_ = ec;
    }

    void
    reset_write_ec(error_code ec)
    {
        wec_ = ec;
    }

    void
    reset_read_ec2(error_code ec2)
    {
        rec2_ = ec2;
    }

    void
    reset_write_ec2(error_code ec2)
    {
        wec2_ = ec2;
    }

    void next_read_length(std::size_t length)
    {
        rnext_ = length;
    }

    void next_write_length(std::size_t length)
    {
        wnext_ = length;
    }

    template <typename Iterator>
    bool
    equal_read_buffers(
        Iterator begin,
        Iterator end,
        std::size_t length)
    {
        return equal_buffers(
            begin,
            end,
            length,
            rbuf_,
            rpos_);
    }

    template <typename Iterator>
    bool
    equal_write_buffers(
        Iterator begin,
        Iterator end,
        std::size_t length)
    {
        return equal_buffers(
            begin,
            end,
            length,
            wbuf_,
            wpos_);
    }

    template <typename ConstBufferSequence>
    bool
    equal_read_buffers(
        const ConstBufferSequence& buffers,
        std::size_t length)
    {
        return equal_read_buffers(
            asio::buffer_sequence_begin(buffers),
            asio::buffer_sequence_end(buffers),
            length);
    }

    template <typename ConstBufferSequence>
    bool
    equal_write_buffers(
        const ConstBufferSequence& buffers,
        std::size_t length)
    {
        return equal_write_buffers(
            asio::buffer_sequence_begin(buffers),
            asio::buffer_sequence_end(buffers),
            length);
    }

    template <typename ConstBufferSequence>
    bool
    equal_read_buffers(
        const ConstBufferSequence& buffers)
    {
        return equal_read_buffers(
            asio::buffer_sequence_begin(buffers),
            asio::buffer_sequence_end(buffers),
            asio::buffer_size(buffers));
    }

    template <typename ConstBufferSequence>
    bool
    equal_write_buffers(
        const ConstBufferSequence& buffers)
    {
        return equal_write_buffers(
            asio::buffer_sequence_begin(buffers),
            asio::buffer_sequence_end(buffers),
            asio::buffer_size(buffers));
    }

private:
    template <typename Iterator>
    static
    bool
    equal_buffers(
        Iterator begin,
        Iterator end,
        std::size_t length,
        unsigned char* buf_data,
        std::size_t buf_length)
    {
        if (length != buf_length)
            return false;
        Iterator buf_iter = begin;
        std::size_t checked_length = 0;
        while (
            buf_iter != end &&
            checked_length < length)
        {
            std::size_t buffer_length =
                asio::buffer_size(*buf_iter);
            if (buffer_length > length - checked_length)
                buffer_length = length - checked_length;
            if (std::memcmp(
                    buf_data + checked_length,
                    buf_iter->data(),
                    buffer_length) != 0)
                return false;
            checked_length += buffer_length;
            ++buf_iter;
        }
        return true;
    }



    asio::io_context& ioc_;

    static constexpr std::size_t max_cap_ = 8192;

    // Read
    unsigned char rbuf_[max_cap_];
    std::size_t rsize_{0};
    std::size_t rpos_{0};
    std::size_t rnext_{0};
    error_code rec_{};
    error_code rec2_{};

    // Write
    unsigned char wbuf_[max_cap_];
    std::size_t wsize_{max_cap_};
    std::size_t wpos_{0};
    std::size_t wnext_{max_cap_};
    error_code wec_{};
    error_code wec2_{};
};
} // test
} // socks_proto
} // boost

#endif // BOOST_SOCKS_PROTO_TEST_UNIT_STREAM_HPP
