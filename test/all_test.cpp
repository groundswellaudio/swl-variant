
#include "ext/include/ghc/fs_std.hpp"
#include "pipecmd.hpp"
#include <string_view>
#include <cstdlib>
#include <iostream>
#include <swl_assert.hpp>

#include <cassert>

struct test_result {
	bool has_compiled;
	std::string output;
};

test_result compile_and_run(std::string_view path){
	constexpr auto&& output_name = "./tmp.test";
	
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
		
		auto path = std::string_view(f.path().c_str());
		if (perform_test(path))
			((summary += "Test ") += path) += " : successful. \n";
		else
			((summary += "Test ") += path) += " : failed. \n";
	}
	
	std::cout << "Test summary : \n" << summary << std::endl;
	/* for (auto& f : list)
		perform_test(f);	 */ 
}