#pragma once

#include <cstdio>
#include <string>

namespace pipecmd{

namespace detail {
	inline std::string read_from_cstream(FILE* cstream){
		std::string res{};
		res.reserve(50);
		int r;
		while ( (r = fgetc(cstream)) != EOF ){
			res.push_back(r);
		}
		return res;
	}

	inline FILE* open(const char* cmd, const char* type){
		#if defined (_WIN32) || (_WIN64)
			return _popen(cmd, type);
		#else
			return popen(cmd, type);
		#endif
	}

	inline void close(FILE* p){
		#if defined (_WIN32) || (_WIN64)
			_pclose(p);
		#else
			pclose(p);
		#endif
	}
}

struct readable{
	
    readable() = default;
	
    readable(const char* cmd){
        open(cmd);
    }
    
    ~readable(){
        detail::close(handle);
    }
    
    void open(const char* cmd){
        handle = detail::open(cmd, "r");
    }
    
    std::string read(){
        return detail::read_from_cstream(handle);
    }
    
    private :
    FILE* handle;
};

} // PIPE
