cmake_minimum_required (VERSION 3.8)

IF (WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
ENDIF()

include_directories(${CMAKE_SOURCE_DIR}/Libs/include)
include_directories(${CMAKE_SOURCE_DIR}/Libs/include/CoreToolkit)
include_directories(${CMAKE_SOURCE_DIR}/Libs/include/WebToolkit)
link_directories(${CMAKE_SOURCE_DIR}/Libs/lib)
link_directories(${CMAKE_SOURCE_DIR}/Libs/bin)

set(HeaderFiles
    "D_DataBase.h"
    "P_HEADER.h"
    "U_DataParser.h"
    "W_WebDispatcher.h"
    "W_WebServer.h"
    "W_WebSession.h"
)

set(SourceFiles
     "D_DataBase.cpp"
     "P_HEADER.cpp"
     "U_DataParser.cpp"
     "U_Main.cpp"
     "W_WebDispatcher.cpp"
     "W_WebServer.cpp"
     "W_WebSession.cpp"
)

set_source_files_properties(${SourceFiles} PROPERTIES LANGUAGE CXX )

add_executable (InitialWorkProject ${SourceFiles} ${HeaderFiles})
IF (WIN32)
target_precompile_headers (InitialWorkProject PRIVATE P_HEADER.h)
ENDIF()

set_property(TARGET InitialWorkProject PROPERTY CXX_STANDARD 17)
set_property(TARGET InitialWorkProject PROPERTY CXX_STANDARD_REQUIRED ON)

list(APPEND LIBS)
IF (WIN32)
    list(APPEND LIBS CoreToolkit sqlite3 WebToolkit ws2_32)

    # COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Libs/bin/sqlite3.dll ${CMAKE_BINARY_DIR}
    add_custom_command(TARGET InitialWorkProject POST_BUILD
                   COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_SOURCE_DIR}/Libs/bin/sqlite3.dll" "${CMAKE_BINARY_DIR}/InitialWorkProject/sqlite3.dll")
ELSE()
    # Do linux native
    list(APPEND LIBS libWebToolkit.a libCoreToolkit.a libsqlite3.a)
    set (CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lpthread -ldl -lnsl ${CMAKE_CSS_STANDARD_LIBRARIES}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -Wl,--no-whole-archive")
ENDIF()

target_link_libraries(InitialWorkProject ${LIBS})