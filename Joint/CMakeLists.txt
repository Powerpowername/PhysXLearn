cmake_minimum_required(VERSION 3.17)

project(joint)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_executable(JointPro)
target_compile_features(JointPro PRIVATE cxx_std_23)
# 为GLM定义拓展宏，否则不能使用
target_compile_definitions(JointPro PRIVATE GLM_ENABLE_EXPERIMENTAL)

# 本项目的源文件是「UTF-8 无 BOM」，而 MSVC 在没有 BOM 时按系统代码页（简中 = 936/GBK）解码源文件，
# 于是 UTF-8 的中文注释凑不成合法 GBK 序列，报 C4819 并可能解析错乱。
#
# 这里只指定「源字符集」，不动「执行字符集」：
#   源码按 UTF-8 正确解码，而窄字符串字面量仍按 GBK 编码进 exe，
#   所以用 printf/cout 往 936 控制台打中文不会乱码。
# 若以后要改用 /utf-8（源码与字面量统一 UTF-8），需同步确认控制台输出编码。
if(MSVC)
    target_compile_options(JointPro PRIVATE /source-charset:utf-8)
endif()

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
