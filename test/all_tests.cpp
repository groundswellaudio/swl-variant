
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

test_result 
compile_and_run(std::string_view path, std::string cmd, const std::string& output_path){
	
	if (not fs::remove(output_path) )
		assert( not fs::exists(output_path) && "failed to remove test binary file.");
	
	cmd += path;
	
	std::system(cmd.c_str());
	
	if (fs::exists(output_path)){
		pipecmd::readable syscmd;
		syscmd.open(output_path.c_str());
		return {syscmd.read(), true};
	}
	else return {"", false};
}

bool is_fail_test(std::string_view filepath){
	return (filepath.find("fail.cpp") != std::string_view::npos);
}

bool perform_fail_test(std::string_view filepath, const std::string& cmd, const std::string& output_path){
	auto res = compile_and_run(filepath, cmd, output_path);
	if (res.has_compiled)
		std::cout << "Failed test : " << filepath << ", (shouldn't have compiled). \n";
	return not res.has_compiled;
}

template <class W>
bool contains(const std::string& str, W&& word){
	return (str.find(word) != std::string::npos);
}

bool perform_test(std::string_view filepath, const std::string& cmd, const std::string& output_path){
	
	if (is_fail_test(filepath)){
		return perform_fail_test(filepath, cmd, output_path);
	}
	
	auto res = compile_and_run(filepath, cmd, output_path);
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

// the command lines arguments must contain the command
// apt to compile a .cpp file, in C++20, adding 
// the ./test directory and its parent to the header 
// search paths, and specifying the output file at the *end* 
// of the command line (important!)

// for example : 
// clang++ -std=c++20 -I . -I .. -o ./test_bin_tmp

int main(int argc, char** argv){
	
	std::string command;
	std::string output_path = argv[argc - 1];
	for (int k = 1; k < argc; ++k){
		command += argv[k];	
		command += " ";
	}
	
	std::string summary;
	unsigned num_test = 0;
	unsigned num_success = 0;
	
	for (auto f : fs::recursive_directory_iterator("./tests")){
		if (f.path().extension() != ".cpp") 
			continue;
		
		++num_test;
		
		auto path = std::string_view(f.path().c_str());
		if (perform_test(path, command, output_path)){
			((summary += "Test ") += path) += " : successful. \n";
			++num_success;
		}
		else
			((summary += "Test ") += path) += " : failed. \n";
	}
	
	std::cout << "Test summary : \n" << summary << "\n";
	std::cout << "\n " << num_success << " success out of " << num_test << " tests. \n";
}