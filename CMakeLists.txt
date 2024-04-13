cmake_minimum_required(VERSION 3.24)

project(
	Constellations
	VERSION 1.0.0
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
	COMPONENT "Plugin"
	EXCLUDE_FROM_ALL
)

if(SkyrimSE_PATH)
	Papyrus_Add(
		"Papyrus"
		GAME ${SkyrimSE_PATH}
		IMPORTS
			"${PROJECT_SOURCE_DIR}/scripts"
			"${PROJECT_SOURCE_DIR}/scripts/include"
		SOURCES
			"${PROJECT_SOURCE_DIR}/scripts/CNS_MagicAddToFaction.psc"
			"${PROJECT_SOURCE_DIR}/scripts/CNS_MagicEnsureInFaction.psc"
			"${PROJECT_SOURCE_DIR}/scripts/CNS_ModObjects.psc"
			"${PROJECT_SOURCE_DIR}/scripts/CNS_TIF__TrainingEnthir.psc"
			"${PROJECT_SOURCE_DIR}/scripts/CNS_TIF__TrainingErandur.psc"
			"${PROJECT_SOURCE_DIR}/scripts/CNS_TIF__TrainingMauhulakh.psc"
			"${PROJECT_SOURCE_DIR}/scripts/CNS_TIF__TrainingNjadaStonearm.psc"
			"${PROJECT_SOURCE_DIR}/scripts/CNS_TIF__TrainingVipirtheFleet.psc"
			"${PROJECT_SOURCE_DIR}/scripts/CNS_TIF__TrainingWylandriah.psc"
		OPTIMIZE
		ANONYMIZE
		COMPONENT ${CMAKE_INSTALL_DEFAULT_COMPONENT_NAME}
	)
endif()

set(CPACK_GENERATOR "7Z")
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${PROJECT_VERSION})
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)
set(CPACK_INSTALL_CMAKE_PROJECTS
	".;${PROJECT_NAME};ALL;/"
	".;${PROJECT_NAME};Plugin;/"
)
include(CPack)
