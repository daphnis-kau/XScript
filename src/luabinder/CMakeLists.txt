include_directories (
	"${PROJECT_SOURCE_DIR}/include" 
	"${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src")
set(ProjectName luabinder)

set(lua_files
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lapi.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lapi.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lauxlib.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lauxlib.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lbaselib.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lcode.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lcode.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/ldblib.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/ldebug.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/ldebug.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/ldo.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/ldo.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/ldump.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lfunc.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lfunc.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lgc.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lgc.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/linit.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/liolib.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/llex.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/llex.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/llimits.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lmathlib.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lmem.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lmem.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/loadlib.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lobject.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lobject.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lopcodes.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lopcodes.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/loslib.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lparser.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lparser.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lstate.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lstate.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lstring.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lstring.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lstrlib.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/ltable.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/ltable.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/ltablib.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/ltm.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/ltm.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lua.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lua.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/luaconf.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lualib.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lundump.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lundump.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lvm.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lvm.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lzio.c
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/lzio.h
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/Makefile
	${PROJECT_SOURCE_DIR}/third_party/lua-5.1.5/src/print.c)	
source_group("third_party" FILES ${lua_files})

set(head_files
	CDebugLua.h 
	CScriptLua.h 
	CTypeLua.h)
source_group("include" FILES ${head_files})

set(source_files
	CDebugLua.cpp 
	CScriptLua.cpp 
	CTypeLua.cpp)	
source_group("source" FILES ${source_files})

add_library(
	${ProjectName} 
	${lua_files} 
	${head_files} 
	${source_files})
set_target_properties(${ProjectName} PROPERTIES FOLDER binder)