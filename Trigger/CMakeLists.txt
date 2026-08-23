cmake_minimum_required(VERSION 3.17)

project(Trigger)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_executable(TriggerPro)

target_include_directories(TriggerPro PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/PhysXLib/include
    )

target_sources(TriggerPro PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

target_link_libraries(TriggerPro PRIVATE
    ${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/LowLevel_static_32.lib
    ${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/LowLevelAABB_static_32.lib
    ${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/LowLevelDynamics_static_32.lib
    ${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/PhysX_32.lib
    ${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/PhysXCharacterKinematic_static_32.lib
    ${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/PhysXCommon_32.lib
    ${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/PhysXCooking_32.lib
    ${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/PhysXExtensions_static_32.lib
    ${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/PhysXFoundation_32.lib
    ${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/PhysXPvdSDK_static_32.lib
    ${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/PhysXTask_static_32.lib
    ${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/PhysXVehicle_static_32.lib
    ${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/SceneQuery_static_32.lib
    ${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/SimulationController_static_32.lib        
    )
# file(GLOB DLL_PATH ${CMAKE_SOURCE_DIR}/PhysXLib/debug*.dll)
add_custom_command(TARGET TriggerPro POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/PhysX_32.dll"      # 源文件路径
        "${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/PhysXCommon_32.dll"      # 源文件路径
        "${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/PhysXCooking_32.dll"      # 源文件路径
        "${CMAKE_SOURCE_DIR}/PhysXLib/$<IF:$<CONFIG:Debug>,debug,release>/PhysXFoundation_32.dll"      # 源文件路径
        "${CMAKE_CURRENT_SOURCE_DIR}/bin/$<IF:$<CONFIG:Debug>,Debug,Release>"                      # 目标目录（即 exe 所在目录）
    COMMENT "Copying dll to output directory"
)
