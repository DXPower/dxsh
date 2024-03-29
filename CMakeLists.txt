cmake_minimum_required(VERSION 3.20)

project(dxsh
  VERSION 1.0
  DESCRIPTION "DXShell"
  LANGUAGES CXX
)

message(STATUS ${CMAKE_CXX_FLAGS})

add_library(dxsh_core)
target_link_libraries(dxsh_core PRIVATE ${CMAKE_SOURCE_DIR}/deps/yomm2.lib)
target_sources(dxsh_core PRIVATE
  ${CMAKE_SOURCE_DIR}/src/core/AST.cpp
  ${CMAKE_SOURCE_DIR}/src/core/Environment.cpp
  ${CMAKE_SOURCE_DIR}/src/core/ExecutionContext.cpp
  ${CMAKE_SOURCE_DIR}/src/core/Interpreter.cpp
  ${CMAKE_SOURCE_DIR}/src/core/Lexer.cpp
  ${CMAKE_SOURCE_DIR}/src/core/Parser.cpp
  ${CMAKE_SOURCE_DIR}/src/core/Statement.cpp
  ${CMAKE_SOURCE_DIR}/src/core/Tokens.cpp
  ${CMAKE_SOURCE_DIR}/src/core/Value.cpp
  ${CMAKE_SOURCE_DIR}/src/core/AstMethods/Evaluate.cpp
  ${CMAKE_SOURCE_DIR}/src/core/AstMethods/Print.cpp
)
target_include_directories(dxsh_core PUBLIC
  ${CMAKE_SOURCE_DIR}/deps/include
  ${CMAKE_SOURCE_DIR}/src/core/include
)

add_executable(dxsh)
target_link_libraries(dxsh PRIVATE dxsh_core)
target_sources(dxsh PRIVATE
  ${CMAKE_SOURCE_DIR}/src/shell/main.cpp
  ${CMAKE_SOURCE_DIR}/src/shell/InterpreterInterface.cpp
  ${CMAKE_SOURCE_DIR}/src/shell/Terminal.cpp
)

# Include(FetchContent)

# set(CMAKE_C_COMPILER clang)
# set(CMAKE_CXX_COMPILER clang++)

# set(CMAKE_BUILD_TYPE Debug)

# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_EXTENSIONS OFF)

# set(CMAKE_EXPORT_COMPILE_COMMANDS True)

# project(DxPtr)
# Include(CTest)

# # Common interface for compiler settings

# add_library(common_settings INTERFACE)

# if(MSVC)
#   target_compile_options(common_settings INTERFACE /W4 /WX)
#   target_link_options(common_settings INTERFACE /DEBUG)
# else()
#   target_compile_options(common_settings INTERFACE -O0 -Wall -Wextra -Werror -fno-omit-frame-pointer -fcolor-diagnostics -fansi-escape-codes)
#   target_link_options(common_settings INTERFACE -O0 -fno-omit-frame-pointer -fcolor-diagnostics -fansi-escape-codes)
# endif()

# add_compile_definitions($<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=1>)

# # Core DxChess library
# add_library(dxptr INTERFACE)

# # target_sources(dxptr PRIVATE 
# #     src/DxChess/Bitboard.cpp
# #     src/DxChess/Coord.cpp
# # )
# target_include_directories(dxptr INTERFACE inc/DxPtr)
# target_link_libraries(dxptr INTERFACE common_settings)


# add_executable(main)
# target_sources(main PRIVATE src/main.cpp)
# target_link_libraries(main PRIVATE common_settings dxptr)

# add_test(main_test main)

# # # Tests

# FetchContent_Declare(
#   Catch2
#   GIT_REPOSITORY https://github.com/catchorg/Catch2.git
#   GIT_TAG        v3.0.1 # or a later release
# )

# FetchContent_MakeAvailable(Catch2)

# add_executable(tests)

# target_sources(tests PRIVATE
#   tests/AdvancedUse.cpp
#   tests/Alignment.cpp
#   tests/Arrays.cpp
#   tests/BasicUse.cpp
#   tests/Conversions.cpp
#   tests/Inheritance.cpp
#   tests/Weaks.cpp
# )

# set_target_properties(tests PROPERTIES EXCLUDE_FROM_ALL TRUE)
# target_link_libraries(tests PRIVATE common_settings)
# target_link_libraries(tests PRIVATE dxptr)
# target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
 