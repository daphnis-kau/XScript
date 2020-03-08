include_directories ("${PROJECT_SOURCE_DIR}/include")
set(ProjectName common)

set(head_files
	${PROJECT_SOURCE_DIR}/include/common/CJson.h
	${PROJECT_SOURCE_DIR}/include/common/CommonType.h
	${PROJECT_SOURCE_DIR}/include/common/CppTypeParser.h
	${PROJECT_SOURCE_DIR}/include/common/CVirtualFun.h
	${PROJECT_SOURCE_DIR}/include/common/Help.h
	${PROJECT_SOURCE_DIR}/include/common/Http.h
	${PROJECT_SOURCE_DIR}/include/common/Memory.h
	${PROJECT_SOURCE_DIR}/include/common/SHA1.h
	${PROJECT_SOURCE_DIR}/include/common/TConstString.h
	${PROJECT_SOURCE_DIR}/include/common/TList.h
	${PROJECT_SOURCE_DIR}/include/common/TRBTree.h
	${PROJECT_SOURCE_DIR}/include/common/TStrStream.h
	${PROJECT_SOURCE_DIR}/include/common/TTinyList.h)
source_group("include" FILES ${head_files})

set(source_files
	CJson.cpp 
	CVirtualFun.cpp 
	Help.cpp 
	Http.cpp 
	Memory.cpp)	
source_group("source" FILES ${source_files})

add_library(
	${ProjectName} 
	${head_files} 
	${source_files})	
set_target_properties(${ProjectName} PROPERTIES FOLDER base)