cmake_minimum_required(VERSION 3.24)

project(
	Constellations
	VERSION 1.0.2
	LANGUAGES CXX
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CMAKE_IGNORE_PATH "${PROJECT_BINARY_DIR}")
list(APPEND CMAKE_MODULE_PATH
	"${PROJECT_SOURCE_DIR}/tools/SKSE-CMakeModules/Modules"
	"${PROJECT_SOURCE_DIR}/tools/BethesdaCMakeModules/Modules"
)
include(SKSEPlugin)
include(Spriggit)
include(Papyrus)

set(SKSE_USE_XBYAK ON)
set(SKSE_SUPPORT_VR OFF)

SKSEPlugin_Add(
	${PROJECT_NAME}
	INCLUDE_DIR include
	SOURCE_DIR src
	SOURCES
		.clang-format
		.editorconfig
		README.md
		vcpkg.json
	PRECOMPILE_HEADERS
		src/common/PCH.h
)

set_target_properties(CommonLibSSE PROPERTIES
    FOLDER External
)

Spriggit_Deserialize(
	"Spriggit"
	INPUT "${PROJECT_SOURCE_DIR}/data/ConstellationsNewSkills.esp"
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ConstellationsNewSkills.esp"
	PACKAGE Spriggit.Yaml
	COMPONENT "PackageOnly"
	EXCLUDE_FROM_ALL
)

file(GLOB SCRIPT_SOURCES
	LIST_DIRECTORIES false
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/scripts/*.psc"
)

if(SkyrimSE_PATH)
	Papyrus_Add(
		"Papyrus"
		GAME ${SkyrimSE_PATH}
		IMPORTS
			"${PROJECT_SOURCE_DIR}/scripts"
			"${PROJECT_SOURCE_DIR}/scripts/include"
		SOURCES ${SCRIPT_SOURCES}
		OPTIMIZE
		ANONYMIZE
		COMPONENT ${CMAKE_INSTALL_DEFAULT_COMPONENT_NAME}
	)
endif()

file(GLOB_RECURSE DATA_FILES
	LIST_DIRECTORIES false
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/data/Interface/Translations/*.txt"
	"${PROJECT_SOURCE_DIR}/data/Meshes/*.nif"
	"${PROJECT_SOURCE_DIR}/data/Seq/*.seq"
	"${PROJECT_SOURCE_DIR}/data/SKSE/*"
	"${PROJECT_SOURCE_DIR}/data/Sound/*.fuz"
	"${PROJECT_SOURCE_DIR}/data/Textures/*.dds"
)

foreach(FILE ${DATA_FILES})
	cmake_path(RELATIVE_PATH FILE
		BASE_DIRECTORY "${PROJECT_SOURCE_DIR}/data"
		OUTPUT_VARIABLE FILE_RELATIVE_PATH
	)

	cmake_path(GET FILE_RELATIVE_PATH
		PARENT_PATH DESTINATION_PATH
	)

	install(
		FILES "${FILE}"
		DESTINATION "${DESTINATION_PATH}"
		COMPONENT "PackageOnly"
		EXCLUDE_FROM_ALL
	)
endforeach()

set(CPACK_GENERATOR "7Z")
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${PROJECT_VERSION})
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)
set(CPACK_INSTALL_CMAKE_PROJECTS
	".;${PROJECT_NAME};ALL;/"
	".;${PROJECT_NAME};PackageOnly;/"
)
include(CPack)
