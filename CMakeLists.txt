cmake_minimum_required(VERSION 3.10)
project(SimpleCalculator)  # 项目名称与你的一致

# 启用C99标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)  # 强制要求C99，不 fallback 到低版本

# 添加可执行文件（你的源文件是main.c）
add_executable(SimpleCalculator main.c)

# 如果用到了数学库（如fabs函数），需要链接
target_link_libraries(SimpleCalculator m)
