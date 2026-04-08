cmake_minimum_required(VERSION 3.22)

# 1. 项目基本设置
# 注意：项目名 ${CMAKE_PROJECT_NAME} 必须与 CubeMX 生成的目标名匹配
project(OLED-CubeMX C CXX ASM)

# 2. 包含工具链文件 (如果是交叉编译，通常在命令行指定或在此包含)
# set(CMAKE_TOOLCHAIN_FILE "cmake/gcc-arm-none-eabi.cmake")

# 3. 引入 CubeMX 生成的构建逻辑
# 假设你上传的文件路径是 
add_subdirectory("D:/Program Files/STM32Cube/workspace/JXKJ/Basic_Code/OLED_MX/OLED-CubeMX/cmake/stm32cubemx")

# 4. 定义用户自己的源文件和头文件路径
set(USER_SOURCES
    "User/Src/OLED.c"
    "User/Src/OLED_Data.c"
)

set(USER_INCLUDES
    "User/Inc"
)

# 5. 将用户代码注入到项目中
# CubeMX 脚本中使用了 ${CMAKE_PROJECT_NAME} 作为 target
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${USER_SOURCES})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${USER_INCLUDES})

# 6. 如果你的用户代码需要特定的宏定义，也可以在这里添加
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE 
    DEBUG_MODE
    USER_VERSION_1_0
)