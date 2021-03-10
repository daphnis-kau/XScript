include_directories (
	"${PROJECT_SOURCE_DIR}/include")

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bins/debug)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bins/release)
if(WIN32)
	if(CMAKE_CL_64)
		set(v8_library_path "${PROJECT_SOURCE_DIR}/third_party/v8/win32/x64")
	else(CMAKE_CL_64)
		set(v8_library_path "${PROJECT_SOURCE_DIR}/third_party/v8/win32/x86")
	endif(CMAKE_CL_64)
	link_directories(${v8_library_path})
	file(COPY ${v8_library_path}/debug/v8.dll DESTINATION ${PROJECT_BINARY_DIR}/bins/debug)
	file(COPY ${v8_library_path}/debug/v8_libbase.dll DESTINATION ${PROJECT_BINARY_DIR}/bins/debug)
	file(COPY ${v8_library_path}/debug/v8_libplatform.dll DESTINATION ${PROJECT_BINARY_DIR}/bins/debug)
	file(COPY ${v8_library_path}/release/v8.dll DESTINATION ${PROJECT_BINARY_DIR}/bins/release)
	file(COPY ${v8_library_path}/release/v8_libbase.dll DESTINATION ${PROJECT_BINARY_DIR}/bins/release)
	file(COPY ${v8_library_path}/release/v8_libplatform.dll DESTINATION ${PROJECT_BINARY_DIR}/bins/release)
endif(WIN32)

set(ProjectName XScriptDll)

file(GLOB head_files "*.h")
source_group("include" FILES ${head_files})

file(GLOB source_files "*.cpp")	
source_group("source" FILES ${source_files})

link_directories(${PROJECT_BINARY_DIR}/libs)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bins)

add_library(${ProjectName} SHARED ${head_files} ${source_files})
target_link_libraries(${ProjectName} luabinder v8binder core common)