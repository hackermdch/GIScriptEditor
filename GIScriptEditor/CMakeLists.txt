add_executable(GIScriptEditor)

if(MSVC)
    set_target_properties(GIScriptEditor PROPERTIES
            VS_GLOBAL_BuildStlModules ON
            WIN32_EXECUTABLE ON
    )
    target_compile_options(GIScriptEditor PRIVATE /utf-8)
endif()

target_sources(GIScriptEditor
        PRIVATE
        main.cpp
        compiler.cpp
        image.cpp
        render.cpp
        widgets.cpp
        window.cpp
        GIScriptEditor.rc
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        compiler.ixx
        image.ixx
        render.ixx
        utils.ixx
        widgets.ixx
        window.ixx
)

target_include_directories(GIScriptEditor PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/external/include
        ${CMAKE_SOURCE_DIR}/external/include/antlr4-runtime
)

target_compile_definitions(GIScriptEditor PRIVATE
        ANTLR4CPP_STATIC
        NOMINMAX
)

target_link_libraries(GIScriptEditor PRIVATE
        GIScript
        "${CMAKE_SOURCE_DIR}/external/lib/$<IF:$<CONFIG:Debug>,debug,release>/antlr4-runtime-static.lib"
        "${CMAKE_SOURCE_DIR}/external/lib/$<IF:$<CONFIG:Debug>,debug,release>/ffi.lib"
        "${CMAKE_SOURCE_DIR}/external/lib/$<IF:$<CONFIG:Debug>,debug,release>/GINodeGraph.lib"
        "${CMAKE_SOURCE_DIR}/external/lib/$<IF:$<CONFIG:Debug>,debug,release>/UgcUtil.lib"
        d3d11.lib
        d2d1.lib
        dxguid.lib
        dcomp.lib
        dwrite.lib
        ole32.lib
        shell32.lib
        uuid.lib
)

add_custom_command(TARGET GIScriptEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/external/dll/$<IF:$<CONFIG:Debug>,debug,release>/"
        "$<TARGET_FILE_DIR:GIScriptEditor>"
        COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:GIScript>"
        "$<TARGET_FILE_DIR:GIScriptEditor>"
        COMMENT "Copying runtime DLLs to EXE directory..."
)
