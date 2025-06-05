# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.20)

project(NavDataPluginExtension DESCRIPTION "Lua plugin for getting DCS World API Data")

file(GLOB_RECURSE source_list "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" )
file(GLOB_RECURSE headers_list "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp" )


add_library( NavDataPluginExtension SHARED
    ${source_list}
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NavDataPluginExtension PROPERTY CXX_STANDARD 23)
endif()

set_target_properties( NavDataPluginExtension PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" )

target_compile_definitions(NavDataPluginExtension PRIVATE LUA_BUILD_AS_DLL )
#target_link_directories( NavDataPluginExtension PRIVATE  )
target_link_libraries( NavDataPluginExtension 
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/dcs-lua/lua.lib"
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/dcs-lua/lua.lib"
)
target_include_directories( NavDataPluginExtension
    INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/dcs-lua/include"
    PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/dcs-lua/include"
    INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
)

#MESSAGE(INFO " LUA BIN PATH: ${NavDataPluginExtension_BIN_PATH}")
if ( NavDataPluginExtension_BIN_PATH )
    MESSAGE(INFO " WILL COPY NavDataPluginExtension.dll TO ${NavDataPluginExtension_BIN_PATH}")
    add_custom_command(TARGET NavDataPluginExtension POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
        -E copy "$<TARGET_FILE:NavDataPluginExtension>" "${NavDataPluginExtension_BIN_PATH}/$<TARGET_FILE_NAME:NavDataPluginExtension>"
        COMMENT "Copying NavDataPluginExtension to bin"
    )
endif()