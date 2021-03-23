
#include "ext/include/ghc/fs_std.hpp"
#include "pipecmd.hpp"
#include <string_view>
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <swl_assert.hpp>

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

struct test_result {
	bool has_compiled;
	std::string output;
};

test_result compile_and_run(std::string_view path){
	constexpr auto&& output_name = "./tmp.test";
	
	auto time_point = fs::file_time_type::clock::now();
	
	if (not fs::remove(output_name) )
		assert( not fs::exists(output_name) && "failed to remove test binary file.");
	
	
	std::string cmd = "g++ -std=c++20 -o ";
	cmd += output_name;
	cmd += " -I . -I .. ";
	cmd += path;
	
	std::system(cmd.c_str());
	
	if (fs::exists(output_name)){
		pipecmd::readable syscmd;
		syscmd.open(output_name);
		return {true, syscmd.read()};
	}
	else return {false, ""};
}

bool is_fail_test(std::string_view filepath){
	return (filepath.find("fail.cpp") != std::string_view::npos);
}

bool perform_fail_test(std::string_view filepath){
	auto res = compile_and_run(filepath);
	if (res.has_compiled)
		std::cout << "Failed test : " << filepath << ", (shouldn't have compiled)." << std::endl;
	return not res.has_compiled;
}

bool contains(const std::string& str, auto&& word){
	return (str.find(word) != std::string::npos);
}

bool perform_test(std::string_view filepath){
	
	if (is_fail_test(filepath)){
		return perform_fail_test(filepath);
	}
	
	auto res = compile_and_run(filepath);
	if (not res.has_compiled){
		std::cout << "Failed test : " << filepath << ", (should have compiled)." << std::endl;
		return false;
	}
	
	if ( contains(res.output, failure_token) ){
		std::cout << "Failed test : " << filepath << ", failed assertion" << std::endl;
		return false;
	}
	if ( not contains(res.output, expected_exit_token) ){
		std::cout << "Failed test : " << filepath << ", unexpected exit." << std::endl;
		return false;
	}
	
	std::cout << "Test " << filepath << " successful. " << std::endl;
	
	return true;
}

int main(){
	
	std::string summary;
	
	unsigned k = 0;
	
	for (auto f : fs::recursive_directory_iterator("./tests")){
		if (f.path().extension() != ".cpp") 
			continue;
		
		++k;
		if (k == 5) break;
		
		auto path = std::string_view(f.path().c_str());
		if (perform_test(path))
			((summary += "Test ") += path) += " : passed. \n";
		else
			((summary += "Test ") += path) += " : failed. \n";
	}
	
	std::cout << "Test summary : \n" << summary << std::endl;
	/* for (auto& f : list)
		perform_test(f);	 */ 
}