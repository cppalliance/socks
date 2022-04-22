//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef BOOST_SOCKS_PROTO_STREAM_HPP
#define BOOST_SOCKS_PROTO_STREAM_HPP

#include <boost/asio/io_context.hpp>
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
    bound_handler(Handler& handler, const Args&... args)
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

    void operator()() const
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
bind_handler(Handler&& handler, Args&&... args)
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
    : io_context_(io_context)
    {
    }

    // AsyncReadStream / AsyncWriteStream
    executor_type
    get_executor() noexcept
    {
        return io_context_.get_executor();
    }

    // SyncReadStream
    template <typename MutableBufferSequence>
    std::size_t
    read_some(const MutableBufferSequence& buffers)
    {
        std::size_t n = asio::buffer_copy(buffers,
            asio::buffer(read_data_, read_length_) + read_position_,
            read_next_length_);
        read_position_ += n;
        return n;
    }

    // SyncReadStream
    template <typename MutableBufferSequence>
    std::size_t
    read_some(
        const MutableBufferSequence& buffers,
        system::error_code& ec)
    {
        ec = system::error_code{};
        std::size_t n = read_some(buffers);
        if (read_position_ == read_length_)
            ec = asio::error::eof;
        return n;
    }

    // SyncWriteStream
    template <typename ConstBufferSequence>
    std::size_t
    write_some(const ConstBufferSequence& buffers)
    {
        size_t n = asio::buffer_copy(
            asio::buffer(write_data_, write_length_) + write_position_,
            buffers, write_next_length_);
        write_position_ += n;
        return n;
    }

    // SyncWriteStream
    template <typename ConstBufferSequence>
    std::size_t
    write_some(
        const ConstBufferSequence& buffers,
        system::error_code& ec)
    {
        ec = system::error_code();
        return write_some(buffers);
    }

    // AsyncReadStream
    template <typename MutableBufferSequence, typename Handler>
    void
    async_read_some(
        const MutableBufferSequence& buffers,
        Handler&& handler)
    {
        std::size_t bytes_transferred = read_some(buffers);
        asio::post(
            get_executor(),
            bind_handler(
                std::move(handler),
                system::error_code(),
                bytes_transferred));
    }

    // AsyncWriteStream
    template <typename ConstBufferSequence, typename Handler>
    void
    async_write_some(
        const ConstBufferSequence& buffers,
        Handler&& handler)
    {
        std::size_t bytes_transferred = write_some(buffers);
        asio::post(get_executor(),
            asio::detail::bind_handler(
                std::move(handler),
                system::error_code(),
                bytes_transferred));
    }

    void
    reset_read(const void* data, std::size_t length)
    {
        BOOST_ASSERT(length <= max_length);
        std::memcpy(read_data_, data, length);
        read_length_ = length;
        read_position_ = 0;
        read_next_length_ = length;
    }

    void
    reset_write(std::size_t length = max_length)
    {
        BOOST_ASSERT(length <= max_length);
        memset(write_data_, 0, max_length);
        write_length_ = length;
        write_position_ = 0;
        write_next_length_ = length;
    }

    void next_read_length(std::size_t length)
    {
        read_next_length_ = length;
    }

    void next_write_length(std::size_t length)
    {
        write_next_length_ = length;
    }

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
            read_data_,
            read_position_);
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
            write_data_,
            write_position_);
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
    asio::io_context& io_context_;

    static constexpr std::size_t max_length = 8192;

    // Read
    unsigned char read_data_[max_length];
    std::size_t read_length_{0};
    std::size_t read_position_{0};
    std::size_t read_next_length_{0};

    // Write
    unsigned char write_data_[max_length];
    std::size_t write_length_{max_length};
    std::size_t write_position_{0};
    std::size_t write_next_length_{max_length};
};
} // test
} // socks_proto
} // boost

#endif //BOOST_SOCKS_PROTO_STREAM_HPP
