//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef  BOOST_SOCKS_TEST_UNIT_STREAM_HPP
#define  BOOST_SOCKS_TEST_UNIT_STREAM_HPP

#include <boost/asio/io_context.hpp>
#include <boost/socks/error.hpp>
#include <tuple>

namespace boost {
namespace socks {
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
    stream(
        asio::io_context& io_context,
        std::size_t fail_at = 0,
        error_code fail_with = {})
        : ioc_(io_context)
        , fail_at_(fail_at)
        , fail_with_(fail_with)
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
        std::size_t n = asio::buffer_copy(
            buffers,
            asio::buffer(out_.data(), out_.size()) + n_read_,
            std::min(out_.size() - n_read_, max_read_some_));
        n_read_ += n;
        return n;
    }

    // SyncReadStream
    template <typename MutableBufferSequence>
    std::size_t
    read_some(
        const MutableBufferSequence& buffers,
        error_code& ec)
    {
        maybe_fail(ec);
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
        std::size_t n0 = in_.size();
        std::size_t n1 = asio::buffer_size(buffers);
        in_.resize(n0 + n1);
        size_t n = asio::buffer_copy(
            asio::buffer(in_.data(), in_.size()) + n0,
            buffers,
            n1);
        return n;
    }

    // SyncWriteStream
    template <typename ConstBufferSequence>
    std::size_t
    write_some(
        const ConstBufferSequence& buffers,
        error_code& ec)
    {
        maybe_fail(ec);
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
            out_.resize(length);
            std::memcpy(out_.data(), data, length);
        }
        else
        {
            out_.resize(0);
        }
        n_read_ = 0;
        max_read_some_ = -1;
    }

    template <
        class ConstBufferSequence,
        typename std::enable_if<
            asio::is_const_buffer_sequence<
                ConstBufferSequence
                >::value,
            int>::type = 0
        >
    void
    reset_read(ConstBufferSequence const& buffers)
    {
        out_.resize(asio::buffer_size(buffers));
        asio::buffer_copy(asio::buffer(out_), buffers);
        n_read_ = 0;
        max_read_some_ = -1;
    }

    void
    append_read(const void* data, std::size_t length)
    {
        if (data)
        {
            std::size_t n0 = out_.size();
            out_.resize(n0 + length);
            std::memcpy(out_.data() + n0, data, length);
        }
    }

    void
    reset_write()
    {
        in_.clear();
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
            out_.data(),
            n_read_);
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
            in_.data(),
            in_.size());
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
    void
    maybe_fail(error_code& ec)
    {
        if (fail_at_ == 0)
            ec = fail_with_;
        else
            --fail_at_;
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

    // I/O
    asio::io_context& ioc_;

    // Out / Read
    std::vector<unsigned char> out_;
    std::size_t n_read_{0};
    std::size_t max_read_some_{std::size_t(-1)};

    // In / Write
    std::vector<unsigned char> in_;

    // Failure
    std::size_t fail_at_{0};
    error_code fail_with_{};
};
} // test
} // socks
} // boost

#endif // BOOST_SOCKS_TEST_UNIT_STREAM_HPP
