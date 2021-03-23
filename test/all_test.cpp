
#include "ext/include/ghc/fs_std.hpp"
#include <string_view>
#include <cstdlib>
#include <iostream>
#include <chrono>

constexpr const char* list[] = 
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
		"variant.variant/variant.assign/conv.pass.cpp", 
		"variant.variant/variant.assign/copy.fail.cpp", 
		"variant.variant/variant.assign/copy.pass.cpp", 
		"variant.variant/variant.assign/move.pass.cpp", 
		"variant.variant/variant.assign/T.pass.cpp", 

		// constructor 
		"variant.variant/variant.ctor/default.pass.cpp",
		"variant.variant/variant.ctor/conv.pass.cpp", 
		"variant.variant/variant.ctor/copy.pass.cpp", 
		"variant.variant/variant.ctor/in_place_index_args.pass.cpp",
		"variant.variant/variant.ctor/in_place_type_args.pass.cpp", 
		// constructor for initializer lists are not implemented 
		//"variant.variant/variant.ctor/in_place_index_init_list_args.pass.cpp",
		//"variant.variant/variant.ctor/in_place_type_init_list_args.pass.cpp",
		"variant.variant/variant.ctor/move.pass.cpp",
		"variant.variant/variant.ctor/T.pass.cpp",

		// destructor 
		"variant.variant/variant.dtor/dtor.pass.cpp",

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

#include <thread>
#include <cassert>

bool compile_and_run(std::string_view path){
	constexpr auto&& output_name = "./tmp.test";
	
	auto time_point = fs::file_time_type::clock::now();
	
	if (not fs::remove(output_name) )
		assert( not fs::exists(output_name) && "failed to remove test binary file.");
	
	
	std::string cmd = "g++ -std=c++20 -o ";
	cmd += output_name;
	cmd += " -I . -I .. ";
	cmd += path;
	
	//using namespace std::chrono_literals;
	//std::this_thread::sleep_for(100ms); // just to give the clock some room...
	
	//std::cout << cmd << std::endl;
	
	std::system(cmd.c_str());
	
	if (fs::exists(output_name)){
		std::string cmd = "./";
		cmd += output_name;
		std::system(cmd.c_str());
		return true;
	}
	else return false;
}

bool is_fail_test(std::string_view filepath){
	return (filepath.find("fail.cpp") != std::string_view::npos);
}

bool perform_fail_test(std::string_view filepath){
	bool res = compile_and_run(filepath);
	if (res)
		std::cout << "Failed test : " << filepath << ", (shouldn't have compiled)." << std::endl;
	return not res;
}

bool perform_test(std::string_view filepath){
	
	if (is_fail_test(filepath)){
		return perform_fail_test(filepath);
	}
	
	bool res = compile_and_run(filepath);
	if (not res)
		std::cout << "Failed test : " << filepath << ", (should have compiled)." << std::endl;
	return res;
}

int main(){
	
	std::string summary;
	
	for (auto& f : list){
		if (perform_test(f))
			((summary += "Test ") += f) += " : passed. \n";
		else
			((summary += "Test ") += f) += " : failed. \n";
	}
	
	std::cout << "Test summary : \n" << summary << std::endl;
	/* for (auto& f : list)
		perform_test(f);	 */ 
}