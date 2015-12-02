
cmake_minimum_required(VERSION 2.6)


project(runit)

include_directories(third_pkgs/sqlite3)
aux_source_directory(src SOURCES)

# resource
set(SOURCES ${SOURCES} src/resource.rc)

# sqlite3
set(SOURCES ${SOURCES} third_pkgs/sqlite3/sqlite3.c)

# unicode
add_definitions(-D_UNICODE -DUNICODE)
# debug
# add_definitions(-DNDEBUG)


add_executable(runit WIN32 ${SOURCES})

target_link_libraries(runit mpr comctl32 Setupapi)
