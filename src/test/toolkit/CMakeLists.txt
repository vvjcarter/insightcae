project(test_toolkit)

macro(add_toolkit_test SRC)
    add_executable(${SRC} ${SRC}.cpp)
    linkToolkitVtk(${SRC} Offscreen)
    add_test(NAME ${SRC} COMMAND ${SRC})
endmacro()

add_toolkit_test(test_simplelatex)
add_toolkit_test(test_meminfo)
add_toolkit_test(test_execution)
add_toolkit_test(test_chartrenderer)
add_toolkit_test(test_multiregion)
add_toolkit_test(test_filecontainer)
add_toolkit_test(test_toUnixPath)
add_toolkit_test(test_remoteExecutionConfig)
add_toolkit_test(test_warningbox)
add_toolkit_test(test_integrate_trpz)


add_subdirectory(analysis_parameterstudy)
