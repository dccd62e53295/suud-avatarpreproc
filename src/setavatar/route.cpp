
#include <cstring>

#include "route.hpp"
#include "VarFull.hpp"

void* i_scgi_obj_init(std::string_view p1){
VarFull* v1=new VarFull(p1);// SPEC_ACTION new

if(!v1->active){
	delete v1;
	return nullptr;
}
return reinterpret_cast<void*>(v1);
};

void i_scgi_obj_fini(void* p1){
VarFull* v1=reinterpret_cast<VarFull*>(p1);
delete v1;
};

bool i_scgi_obj_isnull(void* p1){
return nullptr==p1;
};

