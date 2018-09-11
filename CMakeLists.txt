cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
PROJECT(sioclient)

option(BUILD_SHARED_LIBS "Build the shared library" ON)
option(Boost_USE_STATIC_LIBS "Use Boost static version" OFF)

set(MAJOR 1)
set(MINOR 6)
set(PATCH 0)
set(VERSION ${MAJOR}.${MINOR}.${PATCH})

if(NOT CMAKE_BUILD_TYPE )
MESSAGE(STATUS "not define build type, set to release" )
set(CMAKE_BUILD_TYPE Release )
elseif(NOT (${CMAKE_BUILD_TYPE} STREQUAL "Release" OR ${CMAKE_BUILD_TYPE} STREQUAL "Debug" ))
MESSAGE(SEND_ERROR "CMAKE_BUILD_TYPE must be either Release or Debug")
return()
endif()

set(BOOST_VER "1.55.0" CACHE STRING "boost version" )

set(Boost_USE_MULTITHREADED ON) 
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost ${BOOST_VER} REQUIRED COMPONENTS system date_time random) 

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src ALL_SRC)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src/internal ALL_SRC)
file(GLOB ALL_HEADERS ${CMAKE_CURRENT_LIST_DIR}/src/*.h )
set(SIO_INCLUDEDIR ${CMAKE_CURRENT_LIST_DIR})

add_library(sioclient ${ALL_SRC})
target_include_directories(sioclient PRIVATE ${Boost_INCLUDE_DIRS} 
    ${CMAKE_CURRENT_LIST_DIR}/src 
)

set_property(TARGET sioclient PROPERTY CXX_STANDARD 11)
set_property(TARGET sioclient PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(sioclient PRIVATE ${Boost_LIBRARIES})
if(BUILD_SHARED_LIBS)
set_target_properties(sioclient
	PROPERTIES
		SOVERSION ${MAJOR}
		VERSION ${VERSION}
	)
endif()
list(APPEND TARGET_LIBRARIES sioclient)

find_package(OpenSSL)
if(OPENSSL_FOUND)
add_library(sioclient_tls ${ALL_SRC})
target_include_directories(sioclient_tls PRIVATE ${Boost_INCLUDE_DIRS} 
    ${CMAKE_CURRENT_LIST_DIR}/src 
)

set_property(TARGET sioclient_tls PROPERTY CXX_STANDARD 11)
set_property(TARGET sioclient_tls PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(sioclient_tls PRIVATE ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} )
target_compile_definitions(sioclient_tls PRIVATE -DSIO_TLS)
if(BUILD_SHARED_LIBS)
set_target_properties(sioclient_tls
	PROPERTIES
		SOVERSION ${MAJOR}
		VERSION ${MAJOR}.${MINOR}.${PATCH}
	)
endif()
list(APPEND TARGET_LIBRARIES sioclient_tls)
endif()

set(INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig" CACHE PATH "Installation directory for pkgconfig (.pc) files")

configure_file( ${CMAKE_CURRENT_LIST_DIR}/socketio.pc.cmakein
  ${CMAKE_CURRENT_LIST_DIR}/socketio.pc @ONLY)

install(FILES socketio.pc DESTINATION "${INSTALL_PKGCONFIG_DIR}")
add_executable(sio_console_demo ${CMAKE_CURRENT_LIST_DIR}/examples/Console/main.cpp)
set_property(TARGET sio_console_demo PROPERTY CXX_STANDARD 11)
set_property(TARGET sio_console_demo PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(sio_console_demo sioclient)
target_link_libraries(sio_console_demo pthread )

install(FILES ${ALL_HEADERS}
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(TARGETS ${TARGET_LIBRARIES}
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

install(TARGETS sio_console_demo
    DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
