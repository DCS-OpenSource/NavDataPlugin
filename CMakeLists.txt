# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.20)

include(FetchContent)

project(NavDataPluginNavigraph DESCRIPTION "Lua plugin for getting DCS World API Data")

file(GLOB_RECURSE source_list "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" )
file(GLOB_RECURSE headers_list "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp" )

set(SQLITE3_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext/sqlite3/")
set(SQLITE3_SRC
    sqlite3.c
    shell.c
)

list(TRANSFORM SQLITE3_SRC PREPEND ${SQLITE3_SRC_PATH})

add_library( NavDataPluginNavigraph SHARED
    ${source_list}
    ${SQLITE3_SRC}
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NavDataPluginNavigraph PROPERTY CXX_STANDARD 23)
endif()

set_target_properties( NavDataPluginNavigraph PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" )

target_compile_definitions(NavDataPluginNavigraph PRIVATE LUA_BUILD_AS_DLL )
#target_link_directories( NavDataPluginNavigraph PRIVATE  )

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)


target_link_libraries( NavDataPluginNavigraph 
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/dcs-lua/lua.lib"
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/dcs-lua/lua.lib"
    PRIVATE nlohmann_json::nlohmann_json
)
target_include_directories( NavDataPluginNavigraph
    INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/dcs-lua/include"
    PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/dcs-lua/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/sqlite3"
    INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
)

#MESSAGE(INFO " LUA BIN PATH: ${NavDataPluginNavigraph_BIN_PATH}")
if ( NavDataPluginNavigraph_BIN_PATH )
    MESSAGE(INFO " WILL COPY NavDataPluginNavigraph.dll TO ${NavDataPluginNavigraph_BIN_PATH}")
    add_custom_command(TARGET NavDataPluginNavigraph POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
        -E copy "$<TARGET_FILE:NavDataPluginNavigraph>" "${NavDataPluginNavigraph_BIN_PATH}/$<TARGET_FILE_NAME:NavDataPluginNavigraph>"
        COMMENT "Copying NavDataPluginNavigraph to bin"
    )
endif()