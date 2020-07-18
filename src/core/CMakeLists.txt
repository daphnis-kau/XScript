include_directories ("${PROJECT_SOURCE_DIR}/include")
set(ProjectName core)

set(export_files
	${PROJECT_SOURCE_DIR}/include/core/XScript.h
	${PROJECT_SOURCE_DIR}/include/core/XScriptDef.h
	${PROJECT_SOURCE_DIR}/include/core/XScriptMacro.h
	${PROJECT_SOURCE_DIR}/include/core/XScriptWrap.h)
source_group("export" FILES ${export_files})

set(head_files
	${PROJECT_SOURCE_DIR}/include/core/CCallInfo.h
	${PROJECT_SOURCE_DIR}/include/core/CClassInfo.h
	${PROJECT_SOURCE_DIR}/include/core/CDebugBase.h
	${PROJECT_SOURCE_DIR}/include/core/CScriptBase.h
	${PROJECT_SOURCE_DIR}/include/core/CTypeBase.h)
source_group("include" FILES ${head_files})

set(source_files
	CCallInfo.cpp 
	CClassInfo.cpp 
	CDebugBase.cpp 
	CScriptBase.cpp 
	CTypeBase.cpp)	
source_group("source" FILES ${source_files})

add_library(
	${ProjectName} 
	${export_files} 
	${head_files} 
	${source_files})	
set_target_properties(${ProjectName} PROPERTIES FOLDER base)