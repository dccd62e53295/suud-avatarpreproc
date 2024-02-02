//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: Advanced server
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/detail/config.hpp>
#include <boost/make_unique.hpp>
#include <boost/optional.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <boost/utility/string_view.hpp>
#include <boost/algorithm/algorithm.hpp>
#include <boost/algorithm/string.hpp>

#include "log4c.h"
// Returns a bad request response
template<class Body, class Allocator>
boost::beast::http::response<boost::beast::http::string_body> 
inline res_bad_request(
boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>& req,
std::string_view why)
    {
        boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::bad_request, req.version()};
        //res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

// Returns a not found response
template<class Body, class Allocator>
boost::beast::http::response<boost::beast::http::string_body> 
inline res_not_found(
boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>& req,
std::string_view target)
    {
        boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::not_found, req.version()};
        //res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
		std::string res_body;
		res_body.append("The resource '");
		res_body.append(target);
		res_body.append("' was not found.");
        res.body() = res_body;
        res.prepare_payload();
        return res;
    };

// Returns a server error response
template<class Body, class Allocator>
boost::beast::http::response<boost::beast::http::string_body> 
inline res_server_error(
boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>& req,
std::string_view what)
    {
        boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::internal_server_error, req.version()};
        //res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
		std::string res_body;
		res_body.append("An error occurred: '");
		res_body.append(what);
		res_body.append("'");
        res.body() = res_body;
        res.prepare_payload();
        return res;
    };

#ifndef COMPILEDEF_PATH_USERBRIDGE
#error "COMPILEDEF_PATH_USERBRIDGE not found"
#endif
#include COMPILEDEF_PATH_USERBRIDGE

#ifndef CONFIG_USER_PTR_T
#define CONFIG_USER_PTR_T void*
#endif

#ifndef CONFIG_USER_PTR_INIT
#define CONFIG_USER_PTR_INIT(x) nullptr
#endif

#ifndef CONFIG_USER_PTR_VALID
#define CONFIG_USER_PTR_VALID(x) true
#endif

#ifndef CONFIG_USER_PTR_TERM
#define CONFIG_USER_PTR_TERM(x)
#endif

#ifndef CONFIG_HTTP_READ_TIMEOUT
#define CONFIG_HTTP_READ_TIMEOUT 30
#endif

#ifndef CONFIG_HTTP_READ_BODY_MAX
#define CONFIG_HTTP_READ_BODY_MAX 16384
#endif

//https://stackoverflow.com/questions/55475896/how-to-convert-booststring-view-to-stdstring-view




// Return a reasonable mime type based on the extension of a file.
/*
boost::beast::string_view
mime_type(boost::beast::string_view path)
{
    using boost::beast::iequals;
    auto const ext = [&path]
    {
        auto const pos = path.rfind(".");
        if(pos == boost::beast::string_view::npos)
            return boost::beast::string_view{};
        return path.substr(pos);
    }();
    if(iequals(ext, ".htm"))  return "text/html";
    if(iequals(ext, ".html")) return "text/html";
    if(iequals(ext, ".php"))  return "text/html";
    if(iequals(ext, ".css"))  return "text/css";
    if(iequals(ext, ".txt"))  return "text/plain";
    if(iequals(ext, ".js"))   return "application/javascript";
    if(iequals(ext, ".json")) return "application/json";
    if(iequals(ext, ".xml"))  return "application/xml";
    if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if(iequals(ext, ".flv"))  return "video/x-flv";
    if(iequals(ext, ".png"))  return "image/png";
    if(iequals(ext, ".jpe"))  return "image/jpeg";
    if(iequals(ext, ".jpeg")) return "image/jpeg";
    if(iequals(ext, ".jpg"))  return "image/jpeg";
    if(iequals(ext, ".gif"))  return "image/gif";
    if(iequals(ext, ".bmp"))  return "image/bmp";
    if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if(iequals(ext, ".tiff")) return "image/tiff";
    if(iequals(ext, ".tif"))  return "image/tiff";
    if(iequals(ext, ".svg"))  return "image/svg+xml";
    if(iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}
*/
// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
/*
std::string
path_cat(
    boost::beast::string_view base,
    boost::beast::string_view path)
{
    if(base.empty())
        return std::string(path);
    std::string result(base);
#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for(auto& c : result)
        if(c == '/')
            c = path_separator;
#else
    char constexpr path_separator = '/';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
    return result;
}
*/
// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.





template<
    class Body, class Allocator,
    class Send>
void
handle_request(
    CONFIG_USER_PTR_T doc_root,
    boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& req,
    Send&& send)
{
/*
    // Make sure we can handle the method
    if( req.method() != boost::beast::http::verb::get &&
        req.method() != boost::beast::http::verb::head)
        return send(res_bad_request(req,"Unknown HTTP-method"));

    // Request path must be absolute and not contain "..".
    if( req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != boost::beast::string_view::npos)
        return send(res_bad_request(req,"Illegal request-target"));

    // Build the path to the requested file
    std::string path = path_cat(doc_root, req.target());
    if(req.target().back() == '/')
        path.append("index.html");
*/


/*
std::string s1=my_test_result(path);

    auto const my_success_1 =
    [&req](std::string_view what)
    {
        boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::ok, req.version()};
        res.set(boost::beast::http::field::content_type, "text/plain");
        res.keep_alive(req.keep_alive());
        res.body() = what;
        res.prepare_payload();
        return res;
    };

return send(my_success_1(s1));
*/

#ifdef CONFIG_USER_CALLBACK
return send(CONFIG_USER_CALLBACK(req,doc_root));
#else
return send(res_not_found(req,req.target()));
#endif

/*
    // Attempt to open the file
    boost::beast::error_code ec;
    boost::beast::http::file_body::value_type body;
    body.open(path.c_str(), boost::beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if(ec == boost::beast::errc::no_such_file_or_directory)
        return send(not_found(req.target()));

    // Handle an unknown error
    if(ec)
        return send(server_error(ec.message()));

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to HEAD request
    if(req.method() == boost::beast::http::verb::head)
    {
        boost::beast::http::response<boost::beast::http::empty_body> res{boost::beast::http::status::ok, req.version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }

    // Respond to GET request
    boost::beast::http::response<boost::beast::http::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(boost::beast::http::status::ok, req.version())};
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return send(std::move(res));
*/
}

//------------------------------------------------------------------------------

// Report a failure
void
fail(boost::beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Echoes back all received WebSocket messages
class websocket_session : public std::enable_shared_from_this<websocket_session>
{
    boost::beast::websocket::stream<boost::beast::tcp_stream> ws_;
    boost::beast::flat_buffer buffer_;

public:
    // Take ownership of the socket
    explicit
    websocket_session(boost::asio::ip::tcp::socket&& socket)
        : ws_(std::move(socket))
    {
    }

    // Start the asynchronous accept operation
    template<class Body, class Allocator>
    void
    do_accept(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req)
    {
        // Set suggested timeout settings for the websocket
        ws_.set_option(
            boost::beast::websocket::stream_base::timeout::suggested(
                boost::beast::role_type::server));

        // Set a decorator to change the Server of the handshake
        ws_.set_option(boost::beast::websocket::stream_base::decorator(
            [](boost::beast::websocket::response_type& res)
            {
                res.set(boost::beast::http::field::server,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " advanced-server");
            }));

        // Accept the websocket handshake
        ws_.async_accept(
            req,
            boost::beast::bind_front_handler(
                &websocket_session::on_accept,
                shared_from_this()));
    }

private:
    void
    on_accept(boost::beast::error_code ec)
    {
        if(ec)
            return fail(ec, "accept");

        // Read a message
        do_read();
    }

    void
    do_read()
    {
        // Read a message into our buffer
        ws_.async_read(
            buffer_,
            boost::beast::bind_front_handler(
                &websocket_session::on_read,
                shared_from_this()));
    }

    void
    on_read(
        boost::beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This indicates that the websocket_session was closed
        if(ec == boost::beast::websocket::error::closed)
            return;

        if(ec)
            fail(ec, "read");

        // Echo the message
        ws_.text(ws_.got_text());
        ws_.async_write(
            buffer_.data(),
            boost::beast::bind_front_handler(
                &websocket_session::on_write,
                shared_from_this()));
    }

    void
    on_write(
        boost::beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        // Clear the buffer
        buffer_.consume(buffer_.size());

        // Do another read
        do_read();
    }
};

//------------------------------------------------------------------------------

// Handles an HTTP server connection
class http_session : public std::enable_shared_from_this<http_session>
{
    // This queue is used for HTTP pipelining.
    class queue
    {
        enum
        {
            // Maximum number of responses we will queue
            limit = 8
        };

        // The type-erased, saved work item
        struct work
        {
            virtual ~work() = default;
            virtual void operator()() = 0;
        };

        http_session& self_;
        std::vector<std::unique_ptr<work>> items_;

    public:
        explicit
        queue(http_session& self)
            : self_(self)
        {
            static_assert(limit > 0, "queue limit must be positive");
            items_.reserve(limit);
        }

        // Returns `true` if we have reached the queue limit
        bool
        is_full() const
        {
            return items_.size() >= limit;
        }

        // Called when a message finishes sending
        // Returns `true` if the caller should initiate a read
        bool
        on_write()
        {
            BOOST_ASSERT(! items_.empty());
            auto const was_full = is_full();
            items_.erase(items_.begin());
            if(! items_.empty())
                (*items_.front())();
            return was_full;
        }

        // Called by the HTTP handler to send a response.
        template<bool isRequest, class Body, class Fields>
        void
        operator()(boost::beast::http::message<isRequest, Body, Fields>&& msg)
        {
            // This holds a work item
            struct work_impl : work
            {
                http_session& self_;
                boost::beast::http::message<isRequest, Body, Fields> msg_;

                work_impl(
                    http_session& self,
                    boost::beast::http::message<isRequest, Body, Fields>&& msg)
                    : self_(self)
                    , msg_(std::move(msg))
                {
                }

                void
                operator()()
                {
                    boost::beast::http::async_write(
                        self_.stream_,
                        msg_,
                        boost::beast::bind_front_handler(
                            &http_session::on_write,
                            self_.shared_from_this(),
                            msg_.need_eof()));
                }
            };

            // Allocate and store the work
            items_.push_back(
                boost::make_unique<work_impl>(self_, std::move(msg)));

            // If there was no previous work, start this one
            if(items_.size() == 1)
                (*items_.front())();
        }
    };

    boost::beast::tcp_stream stream_;
    boost::beast::flat_buffer buffer_;
    CONFIG_USER_PTR_T doc_root_;
    queue queue_;

    // The parser is stored in an optional container so we can
    // construct it from scratch it at the beginning of each new message.
    boost::optional<boost::beast::http::request_parser<boost::beast::http::string_body>> parser_;

public:
    // Take ownership of the socket
    http_session(
        boost::asio::ip::tcp::socket&& socket,
        CONFIG_USER_PTR_T doc_root)
        : stream_(std::move(socket))
        , doc_root_(doc_root)
        , queue_(*this)
    {
    }

    // Start the session
    void
    run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        boost::asio::dispatch(
            stream_.get_executor(),
            boost::beast::bind_front_handler(
                &http_session::do_read,
                this->shared_from_this()));
    }


private:
    void
    do_read()
    {
        // Construct a new parser for each message
        parser_.emplace();

        // Apply a reasonable limit to the allowed size
        // of the body in bytes to prevent abuse.
        parser_->body_limit(CONFIG_HTTP_READ_BODY_MAX);

        // Set the timeout.
        stream_.expires_after(std::chrono::seconds(CONFIG_HTTP_READ_TIMEOUT));

        // Read a request using the parser-oriented interface
        boost::beast::http::async_read(
            stream_,
            buffer_,
            *parser_,
            boost::beast::bind_front_handler(
                &http_session::on_read,
                shared_from_this()));
    }

    void
    on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if(ec == boost::beast::http::error::end_of_stream)
            return do_close();

        if(ec)
            return fail(ec, "read");
/*
        // See if it is a WebSocket Upgrade
        if(boost::beast::websocket::is_upgrade(parser_->get()))
        {
            // Create a websocket session, transferring ownership
            // of both the socket and the HTTP request.
            std::make_shared<websocket_session>(
                stream_.release_socket())->do_accept(parser_->release());
            return;
        }
*/

if(boost::beast::websocket::is_upgrade(parser_->get())){
return do_close();
}

        // Send the response
        handle_request(doc_root_, parser_->release(), queue_);

        // If we aren't at the queue limit, try to pipeline another request
        if(! queue_.is_full())
            do_read();
    }

    void
    on_write(bool close, boost::beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        if(close)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return do_close();
        }

        // Inform the queue that a write completed
        if(queue_.on_write())
        {
            // Read another request
            do_read();
        }
    }

    void
    do_close()
    {
        // Send a TCP shutdown
        boost::beast::error_code ec;
        stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    boost::asio::io_context& ioc_;
    boost::asio::ip::tcp::acceptor acceptor_;
    CONFIG_USER_PTR_T doc_root_;

public:
    listener(
        boost::asio::io_context& ioc,
        boost::asio::ip::tcp::endpoint endpoint,
        CONFIG_USER_PTR_T doc_root)
        : ioc_(ioc)
        , acceptor_(boost::asio::make_strand(ioc))
        , doc_root_(doc_root)
    {
        boost::beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec)
        {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if(ec)
        {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec)
        {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(
            boost::asio::socket_base::max_listen_connections, ec);
        if(ec)
        {
            fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void
    run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        boost::asio::dispatch(
            acceptor_.get_executor(),
            boost::beast::bind_front_handler(
                &listener::do_accept,
                this->shared_from_this()));
    }

private:
    void
    do_accept()
    {
        // The new connection gets its own strand
        acceptor_.async_accept(
            boost::asio::make_strand(ioc_),
            boost::beast::bind_front_handler(
                &listener::on_accept,
                shared_from_this()));
    }

    void
    on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket)
    {
        if(ec)
        {
            fail(ec, "accept");
        }
        else
        {
            // Create the http session and run it
            std::make_shared<http_session>(
                std::move(socket),
                doc_root_)->run();
        }

        // Accept another connection
        do_accept();
    }
};

//------------------------------------------------------------------------------



int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 5)
    {
        std::cerr <<
            "Usage: uucc_setavatar <address> <port> <config> <worker threads>\n" <<
            "Example:\n" <<
            "    advanced-server 0.0.0.0 8080 ./config.json 1\n";
        return EXIT_FAILURE;
    }
    auto const address = boost::asio::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    //auto const doc_root = std::make_shared<std::string>(argv[3]);
	CONFIG_USER_PTR_T doc_root=CONFIG_USER_PTR_INIT(argv[3]);// TODO
    auto const threads = std::max<int>(1, std::atoi(argv[4]));

if(!CONFIG_USER_PTR_VALID(doc_root)){
	log4c_log("error 1");
	return EXIT_FAILURE;
}

    // The io_context is required for all I/O
    boost::asio::io_context ioc{1+threads};
    // Create and launch a listening port
    std::make_shared<listener>(
        ioc,
        boost::asio::ip::tcp::endpoint{address, port},
        doc_root)->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
        [&](boost::beast::error_code const&, int)
        {
            // Stop the `io_context`. This will cause `run()`
            // to return immediately, eventually destroying the
            // `io_context` and all of the sockets in it.
            ioc.stop();
        });

std::cout<<"starting: "
#if defined(BOOST_ASIO_SEPARATE_COMPILATION)
"unknown shared"
#else
#if defined(BOOST_ASIO_HAS_EPOLL) || defined(BOOST_ASIO_HAS_EVENTFD)
#if defined(BOOST_ASIO_HAS_EPOLL)
"epoll "
#endif
#if defined(BOOST_ASIO_HAS_EVENTFD)
"eventfd "
#endif
#if defined(BOOST_ASIO_HAS_TIMERFD)
"timerfd "
#endif
"(Linux)"
#elif defined(BOOST_ASIO_HAS_KQUEUE)
"kqueue (BSD/Mac)"
#elif defined(BOOST_ASIO_HAS_IOCP)
"IOCP (Windows)"
#elif defined(BOOST_ASIO_HAS_DEV_POLL)
"/dev/poll (Solaris)"
#else
"unknown static (invalid)"
#warning "Cannot detect event loop while Boost compile as static dependency"
#endif
#endif
" backend"<<std::endl;

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads);
    for(auto i=0;i<threads;++i)
        v.emplace_back(
        [&ioc]
        {
            ioc.run();
        });
    ioc.run();

    // (If we get here, it means we got a SIGINT or SIGTERM)

    // Block until all the threads exit
    for(auto& t : v)
        t.join();

if(CONFIG_USER_PTR_VALID(doc_root)){
CONFIG_USER_PTR_TERM(doc_root);
}

    return EXIT_SUCCESS;
}
