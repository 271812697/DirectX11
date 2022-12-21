cmake_minimum_required(VERSION 3.22)
project("DirectX11")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#收集编译文件
file(GLOB SRC ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB HEAD ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB IMGUI ${PROJECT_SOURCE_DIR}/ImGui/*.*)
file(GLOB UTIL ${PROJECT_SOURCE_DIR}/util/*.*)
file(GLOB CORE ${PROJECT_SOURCE_DIR}/core/*.*)
file(GLOB EXAMPLE ${PROJECT_SOURCE_DIR}/example/*.*)
file(GLOB ASSET ${PROJECT_SOURCE_DIR}/asset/*.*)
file(GLOB SCENE ${PROJECT_SOURCE_DIR}/scene/*.*)
file(GLOB COMPONENT ${PROJECT_SOURCE_DIR}/component/*.*)
#收集字体文件
file(GLOB FONT ${PROJECT_SOURCE_DIR}/*.ttf)
#收集着色器文件
file(GLOB HLSL ${PROJECT_SOURCE_DIR}/*.hlsl)
#整理目录
source_group(imgui FILES ${IMGUI})
source_group(util FILES ${UTIL})
source_group(component FILES ${COMPONENT})
source_group(core FILES ${CORE})
source_group(asset FILES ${ASSET})
source_group(scene FILES ${SCENE})
source_group(example FILES ${EXAMPLE})
add_executable(DirectX11 WIN32 ${SRC} ${HEAD} ${IMGUI} ${UTIL} ${CORE} ${ASSET} ${EXAMPLE} ${SCENE} ${COMPONENT})
#设置工作目录
set_property(TARGET DirectX11 PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")

#设置链接库目录
target_link_directories(DirectX11 PUBLIC
   ${PROJECT_SOURCE_DIR}/lib
)
#设置使用unicode字符集
add_definitions(-DUNICODE -D_UNICODE -DUSE_IMGUI)
#设置包含目录
target_include_directories(DirectX11 PUBLIC ${PROJECT_SOURCE_DIR}/ImGui ${PROJECT_SOURCE_DIR}/include)
#链接输入
target_link_libraries(DirectX11
d3d11.lib
dxgi.lib
dxguid.lib
D3DCompiler.lib
d2d1.lib
dwrite.lib
winmm.lib
kernel32.lib
user32.lib
gdi32.lib
winspool.lib
shell32.lib
ole32.lib
oleaut32.lib
uuid.lib
comdlg32.lib
advapi32.lib
assimp-vc143-mt.lib
)
#拷贝文件到工作目录
#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assimp-vc143-mt.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#拷贝字体
#foreach(list ${FONT})
#file(COPY ${list} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#endforeach(list ${FONT})
#拷贝着色器
#foreach(list ${HLSL})
#file(COPY ${list} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#endforeach(list ${HLSL})
#拷贝资源文件
#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})