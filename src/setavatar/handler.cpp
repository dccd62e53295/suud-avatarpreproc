
#include <cstring>
#include <sstream>

#include "route.hpp"
#include "avatar_proc.hpp"

void i_scgi_handle_req(i_scgi_r& p1){
if(p1.req_data.size()<80){
	return;
}

AvatarProcess v1;
v1.in=p1.req_data;

v1.run();

std::stringstream v2;
union {
	char s[4];
	int32_t i;
}v3;

if(0==v1.error){
	v3.i=htole32(0);
	v2<<v3.s;

	v3.i=htole32((int32_t)v1.hash.size());
	v2<<v3.s;
	v2<<v1.hash;
	v3.i=htole32((int32_t)v1.out.size());
	v2<<v3.s;
	v2<<v1.out;
}else{
	v3.i=htole32(v1.error);
	v2<<v3.s;
}

p1.res_status=200;
p1.processed=true;
p1.res_content_type="";
p1.res_data=v2.str();
};
