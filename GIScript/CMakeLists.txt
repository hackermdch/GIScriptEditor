add_library(GIScript SHARED)

if(MSVC)
    set_target_properties(GIScript PROPERTIES
            VS_GLOBAL_BuildStlModules ON
            CXX_SCAN_FOR_MODULES ON
    )
    target_compile_options(GIScript PRIVATE /utf-8)
endif()

target_sources(GIScript
        PRIVATE
        GIScript.cpp
        main.cpp
        script.cpp
        gen/GIScriptBaseListener.cpp
        gen/GIScriptBaseVisitor.cpp
        gen/GIScriptLexer.cpp
        gen/GIScriptListener.cpp
        gen/GIScriptParser.cpp
        gen/GIScriptVisitor.cpp
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        GIScript.ixx
        script.ixx
)

target_include_directories(GIScript PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/external/include
        ${CMAKE_SOURCE_DIR}/external/include/antlr4-runtime
)

target_compile_definitions(GIScript PRIVATE
        ANTLR4CPP_STATIC
        NOMINMAX
)

target_link_libraries(GIScript PRIVATE
        "${CMAKE_SOURCE_DIR}/external/lib/$<IF:$<CONFIG:Debug>,debug,release>/antlr4-runtime-static.lib"
        kernel32 user32 advapi32
)
