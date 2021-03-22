
/* 
#define CtorTest \
	default.pass \
	in_place_index_args.pass.cpp \
	in_place_index_init_list_args.pass.cpp \
	in_place_index_init_list_args.pass.cpp \ 
#define CtorTest(Folder) Folder() */ 

#include "ext/filesystem/include/ghc/filesystem.hpp"

constexpr const char*[] list = 
{	
	// get
	"variant.get/get_if_index.pass.cpp",
	"variant.get/get_if_type.pass.cpp", 
	"variant.get/get_index.pass.cpp",
	"variant.get/get_type.pass.cpp",
	"variant.get/holds_alternative.pass.cpp",
	
	// bad access
	"variant.bad_variant_access/bad_variant_access.pass.cpp", 
	
	// members functions 
		
		// general
		"variant.variant/variant_void.fail.cpp", 
		"variant.variant/variant_array.fail.cpp", 
		"variant.variant/variant_empty.fail.cpp", 
		"variant.variant/variant_reference.fail.cpp", 

		// assignment 
		"variant.variant/variant.assign/variant.assign",
		"variant.variant/variant.assign/conv.pass.cpp", 
		"variant.variant/variant.assign/copy.fail.cpp", 
		"variant.variant/variant.assign/copy.pass.cpp", 
		"variant.variant/variant.assign/move.pass.cpp", 
		"variant.variant/variant.assign/T.pass.cpp", 

		// constructor 
		"variant.variant/variant.ctor/default.pass.cpp",
		"variant.variant/variant.ctor/in_place_index_args.pass.cpp",
		"variant.variant/variant.ctor/in_place_index_init_list_args.pass.cpp",
		"variant.variant/variant.ctor/move.pass.cpp",
		"variant.variant/variant.ctor/T.pass.cpp",

		// destructor 
		"variant.variant/variant.dtor/variant_dtor.pass.cpp",

		// modifiers 
		"variant.variant/variant.mod/emplace_index_args.pass.cpp",
		//"variant.variant/variant.mod/emplace_index_init_list_args.pass.cpp",
		"variant.variant/variant.mod/emplace_type_args.pass.cpp",
		//"variant.variant/variant.mod/emplace_type_init_list_args.pass.cpp"

		// status
		"variant.variant/variant.status/index.pass.cpp", 
		"variant.variant/variant.status/valueless_by_exception.pass.cpp",
	
	// swap
	"variant.variant/variant.swap/swap.pass.cpp",
	
	// visitation
	"variant.visit/robust_against_adl.pass.cpp",
	"variant.visit/visit_return_type.pass.cpp",
	"variant.visit/visit.pass.cpp",
	
	// monostate
	"variant.monostate/monostate.pass.cpp", 
	"variant.monostate.relops/relops.pass.cpp",
	
	// relops
	"variant.relops/relops_bool_conv.fail.cpp",
	"variant.relops/relops.pass.cpp",
	
	// hash
	"variant.hash/enabled_hash.pass.cpp",
	"variant.hash/hash.pass.cpp"
};


int main(){
	
	
}