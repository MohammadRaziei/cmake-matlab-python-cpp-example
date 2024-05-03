cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ version selection")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")
include(PyProject)


if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif()



set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)

project(${PyProject_NAME} LANGUAGES CXX VERSION ${PyProject_VERSION})
message(STATUS "Project: ${PROJECT_NAME}@v${PROJECT_VERSION}")

# Download the public suffix list file from the internet if not already exists

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
message(STATUS ${SOURCES})

option(BUILD_PYTHON "export python module" on)
option(BUILD_MEX "export matlab mex file" on)

find_program(PYTHON_EXEC NAMES python python3)
message(STATUS "FOUND python at ${PYTHON_EXEC}")


if(DEFINED SKBUILD)
    set(PYTHON_PROJECT_NAME "${SKBUILD_PROJECT_NAME}")
elseif(BUILD_PYTHON)
    set(PYTHON_PROJECT_NAME "${CMAKE_BINARY_DIR}")
    
    execute_process(
        COMMAND "${PYTHON_EXEC}" -m nanobind --cmake_dir
        OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE NB_DIR)
    message(STATUS "Found NanoBind at ${NB_DIR}")
    list(APPEND CMAKE_PREFIX_PATH "${NB_DIR}")
endif()




if(DEFINED PYTHON_PROJECT_NAME)
    # Try to import all Python components potentially needed by nanobind
    find_package(Python 3.8
        REQUIRED COMPONENTS Interpreter Development.Module
        OPTIONAL_COMPONENTS Development.SABIModule)
    # Import nanobind through CMake's find_package mechanism
    find_package(nanobind CONFIG REQUIRED)

    # We are now ready to compile the actual extension module
    nanobind_add_module(
        # Name of the extension
        _core

        # Target the stable ABI for Python 3.12+, which reduces
        # the number of binary wheels that must be built. This
        # does nothing on older Python versions
        STABLE_ABI

        # Build libnanobind statically and merge it into the
        # extension (which itself remains a shared library)
        #
        # If your project builds multiple extensions, you can
        # replace this flag by NB_SHARED to conserve space by
        # reusing a shared libnanobind across libraries
        NB_STATIC

        # Source code goes here
        ${PROJECT_SOURCE_DIR}/src/binding/bind_arrayProduct.cpp
    )

    target_sources(_core PRIVATE ${SOURCES})
    target_include_directories(_core PRIVATE include src)
    target_compile_definitions(_core PRIVATE VERSION_INFO=${PROJECT_VERSION})


    if(DEFINED SKBUILD)
    install(TARGETS _core
            CONFIGURATIONS Release
            LIBRARY DESTINATION
            ${PYTHON_PROJECT_NAME})
    else()
    file(COPY ${PROJECT_SOURCE_DIR}/src/${PROJECT_NAME} DESTINATION ${PYTHON_PROJECT_NAME})
    set_target_properties(_core PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PYTHON_PROJECT_NAME}/${PROJECT_NAME})
    endif()
endif()

if(DEFINED SKBUILD)
    RETURN()
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCES})


target_include_directories(${PROJECT_NAME}
        PUBLIC
            ${PROJECT_SOURCE_DIR}/include
        PRIVATE
            ${PROJECT_SOURCE_DIR}/src)


# add_executable(example examples/main.cpp)
# target_link_libraries(example PRIVATE )

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)


enable_testing()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest)
file(GLOB TEST_SOURCES ${PROJECT_SOURCE_DIR}/tests/cpp/*.cpp)

add_executable(test_${PROJECT_NAME} ${TEST_SOURCES})
target_link_libraries(test_${PROJECT_NAME} PRIVATE ${PROJECT_NAME} gtest gtest_main pthread)
target_include_directories(test_${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/tests/cpp)

add_test(NAME cppTests COMMAND test_${PROJECT_NAME})



if(BUILD_PYTHON)
add_test(NAME pythonTests
    COMMAND ${PYTHON_EXEC} -m pytest ${PROJECT_SOURCE_DIR}/tests/python
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
endif()




if(BUILD_MEX)
find_package(Matlab COMPONENTS MAIN_PROGRAM ENG_LIBRARY MAT_LIBRARY MX_LIBRARY MEX_COMPILER)

if(Matlab_FOUND)
message(STATUS "Matlab: ${Matlab_VERSION_STRING}")

if(Matlab_VERSION_STRING VERSION_LESS 9.14)
  message(WARNING "Matlab >= 9.14 (R2023a) is required for MEX tests to work due to new buildtool argument syntax.")
endif()

matlab_add_mex(NAME ${PROJECT_NAME}Mex SHARED SRC 
    ${PROJECT_SOURCE_DIR}/src/mex/mex_arrayProduct.cpp
)
target_sources(${PROJECT_NAME}Mex PRIVATE ${SOURCES})
set_target_properties(${PROJECT_NAME}Mex PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}Mex PRIVATE include src)


add_test(NAME mexTests
COMMAND ${Matlab_MAIN_PROGRAM} -sd ${PROJECT_SOURCE_DIR}
  -batch "buildtool test('${PROJECT_SOURCE_DIR}/tests/matlab', 'TestMex/test_arrayProduct', '$<TARGET_FILE_DIR:arrayProductMex>')"
)
endif()
endif()


# Find Doxygen package
find_package(Doxygen)
message(STATUS "Doxygen_FOUND : ${Doxygen_FOUND}")
# Find Doxygen package
if (Doxygen_FOUND)
    # Configure Doxyfile.in template
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

    # Add target to generate documentation
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating Doxygen documentation"
        VERBATIM
    )
endif()




