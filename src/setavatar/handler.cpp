
#include <cstring>
#include <iostream>

#include "route.hpp"
#include "avatar_proc.hpp"

void i_scgi_handle_req(i_scgi_r& p1){
if(p1.req_data.size()<80){
	return;
}

AvatarProcess v1;
v1.in=p1.req_data;

try{
v1.run();
}catch(const std::exception& v2){
	std::cout<<"error while process image file:"
	<<typeid(v2).name()<<std::endl<<v2.what()<<std::endl;
	v1.error=1;
}


char* pk_d=nullptr;
std::size_t pk_len=0;

if(0==v1.error){
std::size_t hashlen=v1.hash.size();
std::size_t imglen=v1.out.size();

pk_len=sizeof(int32_t)*3+hashlen+imglen;
pk_d=new char[pk_len];
std::size_t pk_off=0;

int32_t v4=0;

v4=htole32(0);
memcpy(pk_d+pk_off,&v4,sizeof(int32_t));
pk_off+=sizeof(int32_t);

v4=htole32(static_cast<int32_t>(hashlen));
memcpy(pk_d+pk_off,&v4,sizeof(int32_t));
pk_off+=sizeof(int32_t);

memcpy(pk_d+pk_off,v1.hash.data(),hashlen);
pk_off+=hashlen;

v4=htole32(static_cast<int32_t>(imglen));
memcpy(pk_d+pk_off,&v4,sizeof(int32_t));
pk_off+=sizeof(int32_t);

memcpy(pk_d+pk_off,v1.out.data(),imglen);
pk_off+=imglen;
}else{
pk_len=sizeof(int32_t);
pk_d=new char[pk_len];
int32_t v4=htole32(static_cast<int32_t>(v1.error));
memcpy(pk_d,&v4,sizeof(int32_t));
}

p1.res_status=200;
p1.processed=true;
p1.res_content_type="";
p1.res_data=std::string(pk_d,pk_len);
delete[] pk_d;
};
