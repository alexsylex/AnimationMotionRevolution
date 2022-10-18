cmake_minimum_required(VERSION 3.21)

########################################################################################################################
## Define project
########################################################################################################################

project(
	AnimationMotionRevolution
	VERSION 1.5.3
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	@ONLY
)

set(headers
	include/RE/B/bhkCharacterController.h
	include/RE/B/bhkCharacterMoveInfo.h
	include/RE/B/bhkCharacterState.h
	include/RE/B/bhkCharacterStateClimbing.h
	include/RE/B/bhkCharacterStateFlying.h
	include/RE/B/bhkCharacterStateInAir.h
	include/RE/B/bhkCharacterStateJumping.h
	include/RE/B/bhkCharacterStateOnGround.h
	include/RE/B/bhkCharacterStateSwimming.h
	include/RE/B/bhkWorld.h
	include/RE/H/hkaAnimatedReferenceFrame.h
	include/RE/H/hkaAnimation.h
	include/RE/H/hkaAnimationBinding.h
	include/RE/H/hkaAnimationControl.h
	include/RE/H/hkaAnnotationTrack.h
	include/RE/H/hkaDefaultAnimationControl.h
	include/RE/H/hkaSplineCompressedAnimation.h
	include/RE/H/hkbBehaviorGraph.h
	include/RE/H/hkbContext.h
	include/RE/H/hkbClipGenerator.h
	include/RE/H/hkbGenerator.h
	include/RE/H/hkbNode.h
	include/RE/H/hkbStateMachine.h
	include/RE/H/hkClass.h
	include/RE/H/hkpCharacterMovementUtil.h
	include/RE/H/hkQsTransform.h
	include/RE/H/hkVector4.h
	include/RE/M/MotionDataContainer.h
	include/RE/RTTI.h
	include/utils/Trampoline.h
	include/utils/INISettingCollection.h
	include/utils/half.h
	include/utils/Logger.h
	include/utils/Setting.h
	include/AnimMotionHandler.h
	include/Hooks.h
	include/Motion.h
	include/pch.h
	include/Settings.h
)

set(sources
	source/utils/INISettingCollection.cpp
	source/Hooks.cpp
	source/main.cpp
	source/pch.cpp
	source/Settings.cpp
)

source_group(
	TREE
		${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		${headers}
		${sources}
)

########################################################################################################################
## Configure target DLL
########################################################################################################################

find_package(CommonLibSSE CONFIG REQUIRED)
find_package(binary_io REQUIRED CONFIG)
find_package(spdlog REQUIRED CONFIG)

add_commonlibsse_plugin(${PROJECT_NAME} SOURCES ${headers} ${sources})
add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		CommonLibSSE::CommonLibSSE
		spdlog::spdlog_header_only
)

if (MSVC)
	target_link_options(
		${PROJECT_NAME}
		PRIVATE
			"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
			"$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF>"
	)
endif()

target_precompile_headers(
	${PROJECT_NAME}
	PRIVATE
		include/PCH.h
)

########################################################################################################################
## Automatic plugin deployment
########################################################################################################################

set(SKYRIM_SE_DIR "C:/Program Files (x86)/Steam/steamapps/common/Skyrim Special Edition")
set(SKYRIM_VR_DIR "C:/Program Files (x86)/Steam/steamapps/common/Skyrim VR")
set(SKYRIM_AE_1_6_353_DIR "C:/Program Files (x86)/Steam/steamapps/common/Skyrim Anniversary Edition 1.6.353")
set(SKYRIM_AE_1_6_640_DIR "C:/Program Files (x86)/Steam/steamapps/common/Skyrim Anniversary Edition 1.6.640")

if ("$ENV{RUNTIME_DISABLE_FLAGS}" STREQUAL "")
	set(COPY_SE_AE YES)
	set(COPY_VR YES)
elseif ("$ENV{RUNTIME_DISABLE_FLAGS}" STREQUAL "-UENABLE_SKYRIM_VR")
	set(COPY_SE_AE YES)
	set(COPY_VR NO)
elseif ("$ENV{RUNTIME_DISABLE_FLAGS}" STREQUAL "-UENABLE_SKYRIM_SE -UENABLE_SKYRIM_AE")
	set(COPY_SE_AE NO)
	set(COPY_VR YES)
endif()

if (COPY_SE_AE)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}>
		"${SKYRIM_SE_DIR}/Data/SKSE/Plugins/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}>
		"${SKYRIM_AE_1_6_353_DIR}/Data/SKSE/Plugins/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}>
		"${SKYRIM_AE_1_6_640_DIR}/Data/SKSE/Plugins/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
	)
endif()
if (COPY_VR)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}>
		"${SKYRIM_VR_DIR}/Data/SKSE/Plugins/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
	)
endif()
