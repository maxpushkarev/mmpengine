cmake_minimum_required(VERSION 3.5)
project(mmpengine VERSION 1.0.0 LANGUAGES C CXX)

SET(MMPENGINE_CORE_NAME "mmpengine.core")
SET(MMPENGINE_FRONTEND_NAME "mmpengine.frontend")
SET(MMPENGINE_BACKEND_NAME "mmpengine.backend")

SET(MMPENGINE_DX12_NAME "MMPENGINE_BACKEND_DX12")
SET(MMPENGINE_VULKAN_NAME "MMPENGINE_BACKEND_VULKAN")
SET(MMPENGINE_METAL_NAME "MMPENGINE_BACKEND_METAL")

function(mmpengine_add_source_files_to_proj projName folder)

	file (GLOB sourceFiles ${PROJECT_SOURCE_DIR}/${folder}/*.cpp)
	file (GLOB hFiles ${PROJECT_SOURCE_DIR}/${folder}/*.h)
	file (GLOB hppFiles ${PROJECT_SOURCE_DIR}/${folder}/*.hpp)
	list (APPEND allFiles ${sourceFiles} ${hFiles} ${hppFiles})
	
	foreach( f ${allFiles})
		target_sources(${projName} PRIVATE ${f})
	endforeach( f ${allFiles} )

endfunction()

function(mmpengine_create_proj projName folder)

	add_library(${projName} ${PROJECT_SOURCE_DIR}/${folder}/Base.h)
	mmpengine_add_source_files_to_proj(${projName} ${folder})

	set_target_properties(${projName} PROPERTIES LINKER_LANGUAGE CXX)
	target_compile_features(${projName}  PUBLIC cxx_std_17)

endfunction()

mmpengine_create_proj(${MMPENGINE_CORE_NAME} core)
mmpengine_create_proj(${MMPENGINE_FRONTEND_NAME} frontend)

target_link_libraries(${MMPENGINE_FRONTEND_NAME} ${MMPENGINE_CORE_NAME})

IF (WIN32)

ENDIF()
