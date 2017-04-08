cmake_minimum_required (VERSION 2.8.11)
project (signalrclient)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -fPIC")

set(CPPREST_INCLUDE_DIR "" CACHE FILEPATH "Path to casablanca include dir")
set(OPENSSL_ROOT_DIR "" CACHE FILEPATH "OpenSSL root dir")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
set(BOOST_ROOT_DIR "" CACHE FILEPATH "Boost root dir")

include_directories (
    include
    "${CPPREST_INCLUDE_DIR}"
    "${OPENSSL_INCLUDE_DIR}")

find_library(CPPREST_SO NAMES "libcpprest.dylib" PATHS ${CPPREST_LIB_DIR} REQUIRED)

set(OPENSSL_LIBRARIES
    "${OPENSSL_ROOT_DIR}/lib/libcrypto.a"
    "${OPENSSL_ROOT_DIR}/lib/libssl.a")

set(BOOST_LIBRARIES
    "${BOOST_ROOT_DIR}/lib/libboost_chrono.a"
    "${BOOST_ROOT_DIR}/lib/libboost_filesystem.a"
    "${BOOST_ROOT_DIR}/lib/libboost_random.a"
    "${BOOST_ROOT_DIR}/lib/libboost_regex.a"
    "${BOOST_ROOT_DIR}/lib/libboost_system.a"
    "${BOOST_ROOT_DIR}/lib/libboost_thread-mt.a")


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

add_subdirectory(src/signalrclient)
add_subdirectory(test)




/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++
-Dhttpclient_test_EXPORTS
-I/Users/moozzyk/source/cpprestsdk/Release/include
-I/usr/local/include -I/usr/local/Cellar/openssl/1.0.2h_1/include
-I/Users/moozzyk/source/cpprestsdk/Release/libs/websocketpp
-I/Users/moozzyk/source/cpprestsdk/Release/tests/common/UnitTestpp
-I/Users/moozzyk/source/cpprestsdk/Release/tests/common/utilities/include
-I/Users/moozzyk/source/cpprestsdk/Release/tests/functional/http/client/../utilities/include
-I/Users/moozzyk/source/cpprestsdk/Release/tests/functional/http/client/../utilities/testlistener/include
-stdlib=libc++
-Wno-return-type-c-linkage
-Wno-unneeded-internal-declaration
-std=c++11
-fno-strict-aliasing
-g
-fPIC
-o CMakeFiles/httpclient_test.dir/http_client_fuzz_tests.cpp.o
-c /Users/moozzyk/source/cpprestsdk/Release/tests/functional/http/client/http_client_fuzz_tests.cpp

cd /Users/moozzyk/source/cpprestsdk/build.debug/tests/functional/http/client && /usr/local/Cellar/cmake/3.6.1/bin/cmake -E cmake_link_script CMakeFiles/httpclient_test.dir/link.txt --verbose=1
/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++
-stdlib=libc++
-Wno-return-type-c-linkage
-Wno-unneeded-internal-declaration
-std=c++11 -fno-strict-aliasing
-g
-dynamiclib
-Wl,-headerpad_max_install_names
-o ../../../../Binaries/libhttpclient_test.dylib
-install_name /Users/moozzyk/source/cpprestsdk/build.debug/Binaries/libhttpclient_test.dylib
CMakeFiles/httpclient_test.dir/authentication_tests.cpp.o
CMakeFiles/httpclient_test.dir/building_request_tests.cpp.o
CMakeFiles/httpclient_test.dir/client_construction.cpp.o
CMakeFiles/httpclient_test.dir/connections_and_errors.cpp.o
CMakeFiles/httpclient_test.dir/header_tests.cpp.o
CMakeFiles/httpclient_test.dir/http_client_tests.cpp.o
CMakeFiles/httpclient_test.dir/http_methods_tests.cpp.o
CMakeFiles/httpclient_test.dir/multiple_requests.cpp.o
CMakeFiles/httpclient_test.dir/oauth1_tests.cpp.o
CMakeFiles/httpclient_test.dir/oauth2_tests.cpp.o
CMakeFiles/httpclient_test.dir/outside_tests.cpp.o
CMakeFiles/httpclient_test.dir/pipeline_stage_tests.cpp.o
CMakeFiles/httpclient_test.dir/progress_handler_tests.cpp.o
CMakeFiles/httpclient_test.dir/proxy_tests.cpp.o
CMakeFiles/httpclient_test.dir/request_helper_tests.cpp.o
CMakeFiles/httpclient_test.dir/request_stream_tests.cpp.o
CMakeFiles/httpclient_test.dir/request_uri_tests.cpp.o
CMakeFiles/httpclient_test.dir/response_extract_tests.cpp.o
CMakeFiles/httpclient_test.dir/response_stream_tests.cpp.o
CMakeFiles/httpclient_test.dir/status_code_reason_phrase_tests.cpp.o
CMakeFiles/httpclient_test.dir/to_string_tests.cpp.o
CMakeFiles/httpclient_test.dir/http_client_fuzz_tests.cpp.o
../../../../Binaries/libhttptest_utilities.dylib
../../../../Binaries/libcommon_utilities.dylib
../../../../Binaries/libunittestpp.dylib
../../../../Binaries/libcpprest.2.8.dylib
/usr/local/lib/libboost_filesystem-mt.dylib
/usr/local/lib/libboost_system-mt.dylib
/usr/local/lib/libboost_thread-mt.dylib
/usr/local/lib/libboost_atomic-mt.dylib
/usr/local/lib/libboost_chrono-mt.dylib
/usr/local/lib/libboost_random-mt.dylib
/usr/local/lib/libboost_regex-mt.dylib
-framework CoreFoundation
-framework Security
/usr/local/Cellar/openssl/1.0.2h_1/lib/libssl.dylib
/usr/local/Cellar/openssl/1.0.2h_1/lib/libcrypto.dylib
-framework CoreFoundation
-framework Security
/usr/local/Cellar/openssl/1.0.2h_1/lib/libssl.dylib
/usr/local/Cellar/openssl/1.0.2h_1/lib/libcrypto.dylib
[100%] Built target httpclient_test