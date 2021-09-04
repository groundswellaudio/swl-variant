
#include <iostream>

constexpr auto&& failure_token = "##FAIL##";
constexpr auto&& expected_exit_token = "##SUCCESS_EXIT##";

// replace the good old assert with this to make sure we can detect failure with a string search in the program output

#define SWL_ASSERT(X) \
	do { \
	if(not (X)) std::cout << failure_token << " Assertion failed : [" << #X << "] in " << __FILE__ " at " << __LINE__ << " \n"; \
	} while(0)

#define SWL_END_TEST_SIGNAL std::cout << "\n " << expected_exit_token << " \n";