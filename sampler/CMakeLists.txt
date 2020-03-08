include_directories (
	"${PROJECT_SOURCE_DIR}/include")
if(WIN32)
	if(CMAKE_CL_64)
		link_directories("${PROJECT_SOURCE_DIR}/third_party/v8/win32/x64")
	else(CMAKE_CL_64)
		link_directories("${PROJECT_SOURCE_DIR}/third_party/v8/win32/x86")
	endif(CMAKE_CL_64)
endif(WIN32)

set(ProjectName sampler)

set(head_files
	CApplication.h 
	sampler.h)
source_group("include" FILES ${head_files})

set(source_files
	CApplication.cpp 
	sampler.cpp)	
source_group("source" FILES ${source_files})

add_executable(
	${ProjectName} 
	${head_files} 
	${source_files})
target_link_libraries(sampler common core luabinder v8binder)