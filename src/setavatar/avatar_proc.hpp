#pragma once

#include <string>

//https://www.boost.org/doc/libs/1_74_0/libs/gil/doc/html/design/image.html

class AvatarProcess{
	public:
	std::string in,out,hash;
	int8_t error=0;

	bool run();
};





