//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_IMPL_ERROR_IPP
#define BOOST_SOCKS_IMPL_ERROR_IPP

namespace boost {
namespace socks {

error_code
make_error_code(error e)
{
    struct socks_category : error_category
    {
        const char*
        name() const noexcept override
        {
            return "boost.socks";
        }

        std::string
        message(int ev) const override
        {
            switch(static_cast<error>(ev))
            {
            // SOCKS5 replies
            case error::succeeded: return "Succeeded";
            case error::general_failure: return "General SOCKS server failure";
            case error::connection_not_allowed_by_ruleset: return "Connection not allowed by ruleset";
            case error::network_unreachable: return "Network unreachable";
            case error::host_unreachable: return "Host unreachable";
            case error::connection_refused: return "Connection refused";
            case error::ttl_expired: return "TTL expired";
            case error::command_not_supported: return "Command not supported";
            case error::address_type_not_supported: return "Address type not supported";
            // SOCKS4 replies
            case error::request_granted: return "Request granted";
            case error::request_rejected_or_failed: return "General SOCKS server failure";
            case error::cannot_connect_to_identd_on_the_client: return "Connection not allowed by ruleset";
            case error::client_and_identd_report_different_user_ids: return "Network unreachable";
            // Parsing error
            case error::bad_reply_size: return "Bad reply size";
            case error::bad_reply_version: return "Bad reply version";
            case error::bad_server_choice: return "Bad authentication server choice";
            case error::bad_reply_command: return "Bad reply command";
            case error::bad_reserved_component: return "Bad reserved component";
            case error::bad_address_type: return "Bad address type";
            case error::unassigned_reply_code:
            default: return "Unassigned";
            }
        }

        error_condition
        default_error_condition(
            int ev) const noexcept override
        {
            switch(static_cast<error>(ev))
            {
            case error::succeeded:
            case error::request_granted:
                return condition::succeeded;
            case error::general_failure:
            case error::connection_not_allowed_by_ruleset:
            case error::network_unreachable:
            case error::host_unreachable:
            case error::connection_refused:
            case error::ttl_expired:
            case error::command_not_supported:
            case error::address_type_not_supported:
            case error::request_rejected_or_failed:
            case error::cannot_connect_to_identd_on_the_client:
            case error::client_and_identd_report_different_user_ids:
            case error::unassigned_reply_code:
                return condition::reply_error;
            case error::bad_reply_size:
            case error::bad_reply_version:
            case error::bad_server_choice:
            case error::bad_reply_command:
            case error::bad_reserved_component:
            case error::bad_address_type:
                return condition::parse_error;
            default:
                return {ev, *this};
            }
        }

        bool
        failed( int ev ) const noexcept override
        {
            return ev != 0 && ev != 90;
        }
    };

    static socks_category cat{};
    return error_code{static_cast<
        std::underlying_type<error>::type>(e), cat};
}

error_condition
make_error_condition(condition c)
{
    struct condition_category : error_category
    {
        const char*
        name() const noexcept override
        {
            return "boost.socks";
        }

        std::string
        message(int cv) const override
        {
            switch(static_cast<condition>(cv))
            {
            case condition::succeeded:
                return "SOCKS reply successful";
            case condition::reply_error:
                return "SOCKS reply with error";
            case condition::parse_error:
                return "Cannot parse a request or reply";
            default:
                return "Unassigned";
            }
        }
    };
    // on some versions of msvc-14.2 the category is put in RO memory
    // erroneusly, if the category object is const,
    // and that may result in crash
    static condition_category cat{};
    return error_condition{static_cast<
        std::underlying_type<condition>::type>(c), cat};
}

} // socks
} // boost

#endif
