cmake_minimum_required(VERSION 3.8)
project(W2VServer)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(${PROJECT_SOURCE_DIR}/../CxxFramework CxxFramework)

set(HEADER_FILES
        types.h ExpTable.hpp Randomizer.hpp Sampler.hpp Vocabulary.hpp
        Word2VecModel.hpp CBOWModel.hpp SkipGramModel.hpp Word2VecTrainer.hpp)

set(SOURCE_FILES
        main.cpp)

add_executable(W2VServer
        ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(W2VServer
        PRIVATE CxxFramework)