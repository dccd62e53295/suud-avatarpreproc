
#include "VarFull.hpp"

#include <fstream>
#include <iostream>

#include <jsoncpp/json/json.h>



//https://www.mongodb.com/docs/drivers/c/
//https://mongoc.org/libmongoc/current/index.html
//https://mongoc.org/libmongoc/current/tutorial.html#making-a-connection
//https://mongoc.org/libmongoc/current/tutorial.html#threading

VarFull::VarFull(std::string_view p1){
if(this->active){
	return;
}
this->active=true;
return;
};

VarFull::~VarFull(){
if(!this->active){
return;
}
this->active=false;
};

