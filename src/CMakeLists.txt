CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(Tetris)

# Include the engine CMakeLists for Options 
include(../../engine/src/CMakeLists.txt)

#set(TetrisFolders "Projects/Tetris")
addProject("Tetris" . ${ENGINE_SRC_DIR})
