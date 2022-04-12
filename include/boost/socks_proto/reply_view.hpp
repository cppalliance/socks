//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef BOOST_SOCKS_PROTO_REQUEST_VIEW_HPP
#define BOOST_SOCKS_PROTO_REQUEST_VIEW_HPP

namespace boost {
namespace socks_proto {

/** A read-only view to a SOCKS reply

    Objects of this type represent valid SOCKS
    reply byte sequences whose storage is
    managed externally.

    Callers are responsible for ensuring that
    the lifetime of the underlying bytes extends
    until the view is no longer in use.

    The constructor parses a reply and throws
    an exception on error.

    The parsing free functions offer different
    choices of grammar and can indicate failure
    using an error code.

    @par Specification
    @li <a href="https://datatracker.ietf.org/doc/html/rfc1928#section-6"
        >Replies. SOCKS Protocol Version 5 (rfc1928)</a>

*/
class reply_view
{

};

} // socks_proto
} // boost

#endif // BOOST_SOCKS_PROTO_REQUEST_VIEW_HPP
