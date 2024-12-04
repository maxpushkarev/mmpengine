cmake_minimum_required(VERSION 3.5)
project(mmpengine VERSION 1.0.0 LANGUAGES C CXX)

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 2.24:
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

set_property(GLOBAL PROPERTY MMPENGINE_CORE_NAME "mmpengine.core")
set_property(GLOBAL PROPERTY MMPENGINE_FRONTEND_NAME "mmpengine.frontend")

set_property(GLOBAL PROPERTY MMPENGINE_BACKEND_DX12_NAME "mmpengine.backend.dx12")
set_property(GLOBAL PROPERTY MMPENGINE_BACKEND_VULKAN_NAME "mmpengine.backend.vulkan")
set_property(GLOBAL PROPERTY MMPENGINE_BACKEND_METAL_NAME "mmpengine.backend.metal")

set_property(GLOBAL PROPERTY MMPENGINE_DX12_MACRO "MMPENGINE_BACKEND_DX12")
set_property(GLOBAL PROPERTY MMPENGINE_VULKAN_MACRO "MMPENGINE_BACKEND_VULKAN")
set_property(GLOBAL PROPERTY MMPENGINE_METAL_MACRO "MMPENGINE_BACKEND_METAL")

set_property(GLOBAL PROPERTY MMPENGINE_WIN_MACRO "MMPENGINE_WIN")
set_property(GLOBAL PROPERTY MMPENGINE_MACOSX_MACRO "MMPENGINE_MAC")

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

	add_library(${projName} ${PROJECT_SOURCE_DIR}/${folder}/Base.hpp)
	mmpengine_add_source_files_to_proj(${projName} ${folder})

	set_target_properties(${projName} PROPERTIES LINKER_LANGUAGE CXX)
	target_compile_features(${projName}  PRIVATE cxx_std_17)
	target_include_directories(${projName} PRIVATE ${PROJECT_SOURCE_DIR}/src)

endfunction()

get_property(core_name_tmp GLOBAL PROPERTY MMPENGINE_CORE_NAME)
get_property(frontend_name_tmp GLOBAL PROPERTY MMPENGINE_FRONTEND_NAME)

mmpengine_create_proj(${core_name_tmp} src/Core)
mmpengine_create_proj(${frontend_name_tmp} src/Frontend)

target_link_libraries(${frontend_name_tmp} PRIVATE ${core_name_tmp} nlohmann_json::nlohmann_json)

IF (WIN32)

	get_property(win_macro_tmp GLOBAL PROPERTY MMPENGINE_WIN_MACRO)
	target_compile_definitions(${core_name_tmp} PRIVATE ${win_macro_tmp}=1)
	target_compile_definitions(${frontend_name_tmp} PRIVATE ${win_macro_tmp}=1)

	#Dx12
	get_property(dx12_name_tmp GLOBAL PROPERTY MMPENGINE_BACKEND_DX12_NAME)
	get_property(dx12_macro_tmp GLOBAL PROPERTY MMPENGINE_DX12_MACRO)

	mmpengine_create_proj(${dx12_name_tmp} src/Backend/Dx12)
	target_compile_definitions(${dx12_name_tmp} PRIVATE ${win_macro_tmp}=1)
	target_link_libraries(${dx12_name_tmp} PRIVATE D3D12.lib dxgi.lib dxguid.lib ${core_name_tmp})
	target_link_libraries(${frontend_name_tmp} PRIVATE ${dx12_name_tmp})
	target_compile_definitions(${frontend_name_tmp} PRIVATE ${dx12_macro_tmp}=1)

	#vulkan (TODO)

elseif(APPLE)

	get_property(macosx_macro_tmp GLOBAL PROPERTY MMPENGINE_MACOSX_MACRO)
	target_compile_definitions(${core_name_tmp} PRIVATE ${macosx_macro_tmp}=1)
	target_compile_definitions(${frontend_name_tmp} PRIVATE ${macosx_macro_tmp}=1)

	#metal (TODO)

ENDIF()
