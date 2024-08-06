#pragma once
//#include "mywait.h"
#include "route.hpp"

#define CONFIG_HTTP_READ_TIMEOUT 30
#define CONFIG_HTTP_READ_BODY_MAX 1048576

#define CONFIG_USER_PTR_T void*

#define CONFIG_USER_PTR_INIT(x) i_scgi_obj_init(x)
#define CONFIG_USER_PTR_VALID(x) true
#define CONFIG_USER_PTR_TERM(x)

#define CONFIG_USER_CALLBACK(a,b) my_workload_1(a,b)

template<class Body, class Allocator>
boost::beast::http::response<boost::beast::http::string_body> 
inline my_workload_1(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>& req,CONFIG_USER_PTR_T dbc)
{

switch(req.method()){
	case boost::beast::http::verb::post:
	{
	i_scgi_r c;
	c.req_path=std::string(req.target());
	c.req_data=req.body();
	c.req_type=static_cast<std::underlying_type<boost::beast::http::verb>::type>(req.method());

    auto content_type = req.find(boost::beast::http::field::content_type);
    if (content_type != req.end()) {
        c.req_content_type=std::string(content_type->value());
    } else {
        c.req_content_type="";
    }

	c.context=dbc;
	
	i_scgi_handle_req(c);
	
	if(!c.processed){
		return res_bad_request(req,"invalid request param");
	}

	boost::beast::http::response<boost::beast::http::string_body> res{static_cast<boost::beast::http::status>(c.res_status),req.version()};
	if(!c.res_content_type.empty()){
	res.set(boost::beast::http::field::content_type,c.res_content_type);
	}
	res.keep_alive(req.keep_alive());
	res.body()=c.res_data;
	res.prepare_payload();
	return res;
	};
	//break;
	case boost::beast::http::verb::get:
	case boost::beast::http::verb::head:
	default:
	return res_bad_request(req,"Unknown HTTP-method");
	//break;
}
};
