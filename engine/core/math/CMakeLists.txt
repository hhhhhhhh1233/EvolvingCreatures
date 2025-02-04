#--------------------------------------------------------------------------
# math
#--------------------------------------------------------------------------

FILE(GLOB header_files *.h)
FILE(GLOB source_files *.cpp)

SET(files_math ${header_files} ${source_files})
SOURCE_GROUP("math" FILES ${files_math})

# MATH FILES
# FILE(GLOB math_headers math/*.h)
# FILE(GLOB math_source math/*.cpp)
# SET(files_math ${math_headers} ${math_source})
# SOURCE_GROUP("math" FILES ${files_math})
# ADD_LIBRARY(math STATIC ${files_math})
	
#SET(files_pch ../config.h ../config.cc)
#SOURCE_GROUP("pch" FILES ${files_pch})
ADD_LIBRARY(math STATIC ${files_math})
# TARGET_PCH(core ../)
# ADD_DEPENDENCIES(core glew)
# TARGET_LINK_LIBRARIES(core PUBLIC engine exts glew)
