
cmake_minimum_required(VERSION 2.6)


project(RunIt)

set(runit_exe RunIt)

include_directories(third_pkgs/sqlite3)

file(GLOB headers src/*.h)
file(GLOB sources src/*.cpp)

# resource
file(GLOB resources src/resource.rc)

# sqlite3
file(GLOB sqlite3_headers third_pkgs/sqlite3/sqlite3.h)
file(GLOB sqlite3_sources third_pkgs/sqlite3/sqlite3.c)

# for IDE
# source_group("Header files" FILES ${headers})
source_group("Source files" FILES ${headers} ${sources})
source_group("Sqlite3 files" FILES ${sqlite3_headers} ${sqlite3_sources})
source_group("Resource files" FILES ${resources})

# unicode
add_definitions(-D_UNICODE -DUNICODE)
# debug
# add_definitions(-DNDEBUG)


add_executable(${runit_exe} WIN32
    ${sources} ${headers}
    ${sqlite3_sources} ${sqlite3_headers}
    ${resources}
    )

target_link_libraries(${runit_exe} mpr comctl32 Setupapi)
