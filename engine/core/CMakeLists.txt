#--------------------------------------------------------------------------
# core
#--------------------------------------------------------------------------

SET(files_core
	app.h
	app.cc
	Input.h
	gltf.h
	json.hpp
	MeshData.h)
SOURCE_GROUP("core" FILES ${files_core})

# MATH FILES
# FILE(GLOB math_headers math/*.h)
# FILE(GLOB math_source math/*.cpp)
# SET(files_math ${math_headers} ${math_source})
# SOURCE_GROUP("math" FILES ${files_math})
# ADD_LIBRARY(math STATIC ${files_math})
# ADD_SUBDIRECTORY(math)
	
SET(files_pch ../config.h ../config.cc)
SOURCE_GROUP("pch" FILES ${files_pch})
ADD_LIBRARY(core STATIC ${files_core} ${files_pch})
TARGET_PCH(core ../)
ADD_DEPENDENCIES(core glew)
TARGET_LINK_LIBRARIES(core PUBLIC engine exts glew)
