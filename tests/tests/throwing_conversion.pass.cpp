
#include <swl/variant.hpp>
#include <cassert> 

struct test {
	operator int () { throw 0; return 1; }
};

int main(){

	swl::variant<int, bool> v{true};
	
	test t;
	
	try {
		v.emplace<0>( t );
	}
	catch (...) {}
	
	assert( v.index() == 1 );
	assert( get<1>(v) == true );
}
