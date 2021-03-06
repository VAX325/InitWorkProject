#pragma once

#include <stdexcept>

namespace struct_mapping
{

class StructMappingException : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
	using std::runtime_error::operator=;
};

} // struct_mapping