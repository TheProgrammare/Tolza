
#pragma once

#include "Visitor_Default.hpp"


struct Visitor_Codegen : public Visitor_Default {
	// keep parent constructor
	using Visitor_Default::Visitor_Default;
	// all commented visit are not concerned by the codegen
	// but the default visitor will visit and can execute inferior node in CodegenVisitor

	
};
