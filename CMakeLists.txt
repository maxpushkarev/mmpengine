cmake_minimum_required(VERSION 3.5)
project(mmpengine VERSION 1.0.0 LANGUAGES C CXX)

SET(MMPENGINE_CORE_NAME "mmpengine.core")
SET(MMPENGINE_FRONTEND_NAME "mmpengine.frontend")
SET(MMPENGINE_BACKEND_DX12_NAME "mmpengine.backend.dx12")
SET(MMPENGINE_BACKEND_VULKAN_NAME "mmpengine.backend.vulkan")
SET(MMPENGINE_BACKEND_METAL_NAME "mmpengine.backend.metal")

SET(MMPENGINE_DX12_MACRO "MMPENGINE_BACKEND_DX12")
SET(MMPENGINE_VULKAN_MACRO "MMPENGINE_BACKEND_VULKAN")
SET(MMPENGINE_METAL_MACRO "MMPENGINE_BACKEND_METAL")

SET(MMPENGINE_WIN_MACRO "MMPENGINE_WIN")
SET(MMPENGINE_MACOSX_MACRO "MMPENGINE_MAC")

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
	target_compile_features(${projName}  PRIVATE cxx_std_17)
	target_include_directories(${projName} PRIVATE ${PROJECT_SOURCE_DIR}/src)

endfunction()

mmpengine_create_proj(${MMPENGINE_CORE_NAME} src/Core)
mmpengine_create_proj(${MMPENGINE_FRONTEND_NAME} src/Frontend)

target_link_libraries(${MMPENGINE_FRONTEND_NAME} PRIVATE ${MMPENGINE_CORE_NAME})

IF (WIN32)

	target_compile_definitions(${MMPENGINE_CORE_NAME} PRIVATE ${MMPENGINE_WIN_MACRO}=1)
	target_compile_definitions(${MMPENGINE_FRONTEND_NAME} PRIVATE ${MMPENGINE_WIN_MACRO}=1)

	#Dx12
	mmpengine_create_proj(${MMPENGINE_BACKEND_DX12_NAME} src/Backend/Dx12)
	target_compile_definitions(${MMPENGINE_BACKEND_DX12_NAME} PRIVATE ${MMPENGINE_WIN_MACRO}=1)
	target_link_libraries(${MMPENGINE_BACKEND_DX12_NAME} PRIVATE D3D12.lib dxgi.lib dxguid.lib ${MMPENGINE_CORE_NAME})
	target_link_libraries(${MMPENGINE_FRONTEND_NAME} PRIVATE ${MMPENGINE_BACKEND_DX12_NAME})
	target_compile_definitions(${MMPENGINE_FRONTEND_NAME} PRIVATE ${MMPENGINE_DX12_MACRO}=1)

elseif(APPLE)

	target_compile_definitions(${MMPENGINE_CORE_NAME} PRIVATE ${MMPENGINE_MACOSX_MACRO}=1)
	target_compile_definitions(${MMPENGINE_FRONTEND_NAME} PRIVATE ${MMPENGINE_MACOSX_MACRO}=1)

ENDIF()
