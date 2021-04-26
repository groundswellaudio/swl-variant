
#include "ext/include/ghc/fs_std.hpp"
#include "pipecmd.hpp"
#include <string_view>
#include <cstdlib>
#include <iostream>
#include "swl_assert.hpp"

#include <cassert>

struct test_result {
	std::string output;
	bool has_compiled;
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
		return {syscmd.read(), true};
	}
	else return {"", false};
}

bool is_fail_test(std::string_view filepath){
	return (filepath.find("fail.cpp") != std::string_view::npos);
}

bool perform_fail_test(std::string_view filepath){
	auto res = compile_and_run(filepath);
	if (res.has_compiled)
		std::cout << "Failed test : " << filepath << ", (shouldn't have compiled). \n";
	return not res.has_compiled;
}

template <class W>
bool contains(const std::string& str, W&& word){
	return (str.find(word) != std::string::npos);
}

bool perform_test(std::string_view filepath){
	
	if (is_fail_test(filepath)){
		return perform_fail_test(filepath);
	}
	
	auto res = compile_and_run(filepath);
	if (not res.has_compiled){
		std::cout << "Failed test : " << filepath << ", (should have compiled). \n";
		return false;
	}
	
	if ( contains(res.output, failure_token) ){
		std::cout << "Failed test : " << filepath << ", failed assertion \n";
		return false;
	}
	if ( not contains(res.output, expected_exit_token) ){
		std::cout << "Failed test : " << filepath << ", unexpected exit. \n";
		return false;
	}
	
	std::cout << "Test " << filepath << " successful. \n";
	
	return true;
}

int main(){
	
	std::string summary;
	unsigned num_test = 0;
	unsigned num_success = 0;
	
	unsigned k = 0;
	
	for (auto f : fs::recursive_directory_iterator("./tests")){
		if (f.path().extension() != ".cpp") 
			continue;
		
		++num_test;
		
		auto path = std::string_view(f.path().c_str());
		if (perform_test(path)){
			((summary += "Test ") += path) += " : successful. \n";
			++num_success;
		}
		else
			((summary += "Test ") += path) += " : failed. \n";
	}
	
	std::cout << "Test summary : \n" << summary << "\n";
	std::cout << "\n " << num_success << " success out of " << num_test << " tests. \n";
}