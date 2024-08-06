#pragma once

#include <string>

class i_scgi_r{
	public:
	virtual ~i_scgi_r()=default;
	
	//request
	int req_type;
	std::string req_path;
	std::string req_data;
	std::string req_content_type;

	// response
	//https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
	//https://www.iana.org/assignments/media-types/media-types.xhtml
	// application/octet-stream application/json
	unsigned res_status=200;// 200
	std::string res_content_type;// image/webp
	std::string res_data;// {}
	//https://www.boost.org/doc/libs/1_80_0/boost/beast/http/status.hpp
	//unsigned status=0;
	bool processed=false;
	void* context=nullptr;
};

void i_scgi_handle_req(i_scgi_r&);
void* i_scgi_obj_init(std::string_view);

