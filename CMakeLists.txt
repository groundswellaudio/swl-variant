cmake_minimum_required(VERSION 3.20)

project(swl.variant
	VERSION 1.0.0 
	LANGUAGES CXX
	HOMEPAGE_URL https://github.com/groundswellaudio/swl-variant)

add_library(swl-variant INTERFACE)

target_include_directories(swl-variant INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:./include>)

target_compile_features(swl-variant INTERFACE cxx_std_20)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	
	include(CTest)
	
	## tests that are supposed to not compile
	## https://stackoverflow.com/questions/30155619/expected-build-failure-tests-in-cmake
	
	math(EXPR test_idx 0)
	
	file(GLOB_RECURSE test-sources CONFIGURE_DEPENDS tests/*.fail.cpp)
	foreach (source IN LISTS test-sources)
		get_filename_component(name "${source}" NAME_WE)
  		
  		set(test "${PROJECT_NAME}-test-${name}-${test_idx}")
  		MATH(EXPR test_idx "${test_idx} + 1")
  		
  		add_executable(${test} "${source}")
  		
  		## avoid actually building the target
  		set_target_properties(${test} PROPERTIES
                      EXCLUDE_FROM_ALL TRUE
                      EXCLUDE_FROM_DEFAULT_BUILD TRUE)
        
		target_link_libraries(${test} swl-variant)
		target_include_directories(${test} PRIVATE ./tests)
		
		add_test(NAME ${test}
         COMMAND ${CMAKE_COMMAND} --build ./ --target ${test} --config $<CONFIGURATION>
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
        
        set_tests_properties(${test} PROPERTIES WILL_FAIL TRUE)
        
	endforeach()
	
	file(GLOB_RECURSE test-sources CONFIGURE_DEPENDS tests/*.pass.cpp)

	foreach (source IN LISTS test-sources)
		get_filename_component(name "${source}" NAME_WE)
  		
  		set(test "${PROJECT_NAME}-test-${name}-${test_idx}")
  		MATH(EXPR test_idx "${test_idx} + 1")
  		
		add_executable(${test} "${source}")
		target_link_libraries(${test} swl-variant)
		target_include_directories(${test} PRIVATE ./tests)
		add_test(NAME ${test} COMMAND ${test})
	endforeach()
	
endif()