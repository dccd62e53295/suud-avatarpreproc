#pragma once

#include <string>

class VarFull{
public:
	bool active=false;

	VarFull(std::string_view);
	virtual ~VarFull();
};
