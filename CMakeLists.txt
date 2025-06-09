# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.20)

project(NavDataPluginNaviGraph DESCRIPTION "Lua plugin for getting DCS World API Data")

file(GLOB_RECURSE source_list "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" )
file(GLOB_RECURSE headers_list "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp" )

set(SQLITE3_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext/sqlite3/")
set(SQLITE3_SRC
    sqlite3.c
    shell.c
)

list(TRANSFORM SQLITE3_SRC PREPEND ${SQLITE3_SRC_PATH})

add_library( NavDataPluginNaviGraph SHARED
    ${source_list}
    ${SQLITE3_SRC_PATH}
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NavDataPluginNaviGraph PROPERTY CXX_STANDARD 23)
endif()

set_target_properties( NavDataPluginNaviGraph PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" )

target_compile_definitions(NavDataPluginNaviGraph PRIVATE LUA_BUILD_AS_DLL )
#target_link_directories( NavDataPluginNaviGraph PRIVATE  )
target_link_libraries( NavDataPluginNaviGraph 
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/dcs-lua/lua.lib"
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/dcs-lua/lua.lib"
)
target_include_directories( NavDataPluginNaviGraph
    INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/dcs-lua/include"
    PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/dcs-lua/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/sqlite3"
    INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
)

#MESSAGE(INFO " LUA BIN PATH: ${NavDataPluginNaviGraph_BIN_PATH}")
if ( NavDataPluginNaviGraph_BIN_PATH )
    MESSAGE(INFO " WILL COPY NavDataPluginNaviGraph.dll TO ${NavDataPluginNaviGraph_BIN_PATH}")
    add_custom_command(TARGET NavDataPluginNaviGraph POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
        -E copy "$<TARGET_FILE:NavDataPluginNaviGraph>" "${NavDataPluginNaviGraph_BIN_PATH}/$<TARGET_FILE_NAME:NavDataPluginNaviGraph>"
        COMMENT "Copying NavDataPluginNaviGraph to bin"
    )
endif()