add_executable(atop main.cpp atop.cpp util.cpp fifo.cpp)
target_link_libraries(
  atop PRIVATE project_options project_warnings CONAN_PKG::docopt.cpp
  			   CONAN_PKG::fmt CONAN_PKG::spdlog CONAN_PKG::imgui-sfml
		       CONAN_PKG::re2)
