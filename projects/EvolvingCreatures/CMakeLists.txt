#--------------------------------------------------------------------------
# EvolvingCreatures project
#--------------------------------------------------------------------------

PROJECT(EvolvingCreatures)
FILE(GLOB example_headers code/*.h)
FILE(GLOB example_sources code/*.cc)

SET(files_example ${example_headers} ${example_sources})
SOURCE_GROUP("EvolvingCreatures" FILES ${files_example})

ADD_EXECUTABLE(EvolvingCreatures ${files_example})
TARGET_LINK_LIBRARIES(EvolvingCreatures core render unofficial::omniverse-physx-sdk::sdk)
ADD_DEPENDENCIES(EvolvingCreatures core render)

# target_link_libraries(EvolvingCreatures PRIVATE unofficial::omniverse-physx-sdk::sdk)
