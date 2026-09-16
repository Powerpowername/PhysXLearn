cmake_minimum_required(VERSION 3.17)

project(joint)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_executable(JointPro)
target_compile_features(JointPro PRIVATE cxx_std_23)
# 为GLM定义拓展宏，否则不能使用
target_compile_definitions(JointPro PRIVATE GLM_ENABLE_EXPERIMENTAL)

target_include_directories(JointPro PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/thirdPart/glad/include
    ${CMAKE_SOURCE_DIR}/thirdPart/PhysX/include
    ${CMAKE_SOURCE_DIR}/thirdPart/imgui/imgui
    ${CMAKE_SOURCE_DIR}/thirdPart/imgui/backends
    ${CMAKE_SOURCE_DIR}/thirdPart/glfw
    ${CMAKE_SOURCE_DIR}/thirdPart/glm
    )

target_sources(JointPro PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    )

target_link_libraries(JointPro PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/LowLevel_static_32.lib
    ${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/LowLevelAABB_static_32.lib
    ${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/LowLevelDynamics_static_32.lib
    ${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysX_32.lib
    ${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysXCharacterKinematic_static_32.lib
    ${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysXCommon_32.lib
    ${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysXCooking_32.lib
    ${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysXExtensions_static_32.lib
    ${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysXFoundation_32.lib
    ${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysXPvdSDK_static_32.lib
    ${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysXTask_static_32.lib
    ${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysXVehicle_static_32.lib
    ${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/SceneQuery_static_32.lib
    ${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/SimulationController_static_32.lib   
    ${CMAKE_SOURCE_DIR}/lib/imgui/$<IF:$<CONFIG:Debug>,Debug,Release>/imgui.lib   
    ${CMAKE_SOURCE_DIR}/lib/glfw/$<IF:$<CONFIG:Debug>,Debug,Release>/glfw3.lib   
    ${CMAKE_SOURCE_DIR}/lib/glad/$<IF:$<CONFIG:Debug>,Debug,Release>/glad.lib   
    )
# file(GLOB DLL_PATH ${CMAKE_SOURCE_DIR}/PhysXLib/Debug*.dll)
add_custom_command(TARGET JointPro POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysX_32.dll"      # 源文件路径
        "${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysXCommon_32.dll"      # 源文件路径
        "${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysXCooking_32.dll"      # 源文件路径
        "${CMAKE_SOURCE_DIR}/lib/PhysXLib/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysXFoundation_32.dll"      # 源文件路径
        "${CMAKE_CURRENT_SOURCE_DIR}/bin/$<IF:$<CONFIG:Debug>,Debug,Release>"                      # 目标目录（即 exe 所在目录）
    COMMENT "Copying dll to output directory"
)
