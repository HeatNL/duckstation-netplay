add_library(netplay
  netplay.h
  netplay.cpp
)

target_include_directories(netplay PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories(netplay PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(netplay PRIVATE ggpo-x "${CMAKE_DL_LIBS}")

