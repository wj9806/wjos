project(utils LANGUAGES C)  

file(GLOB_RECURSE C_LIST "*.c" "*.h" "*.S")
add_library(${PROJECT_NAME} ${C_LIST})