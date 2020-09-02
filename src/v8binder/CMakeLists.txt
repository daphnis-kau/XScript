include_directories (
	"${PROJECT_SOURCE_DIR}/include" 
	"${PROJECT_SOURCE_DIR}/third_party/v8")
set(ProjectName v8binder)

set(head_files
	CDebugJS.h 
	CScriptJS.h 
	CTypeJS.h
	V8Context.h)
source_group("include" FILES ${head_files})

set(source_files
	CDebugJS.cpp 
	CScriptJS.cpp 
	CTypeJS.cpp
	V8Context.cpp)	
source_group("source" FILES ${source_files})

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/libs)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/libs) 
add_library(
	${ProjectName} 
	${head_files} 
	${source_files})
set_target_properties(${ProjectName} PROPERTIES FOLDER binder)