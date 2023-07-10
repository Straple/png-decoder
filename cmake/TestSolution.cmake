option(TEST_SOLUTION "Build solution" OFF)

function(patch_include_directories TARGET)
  if (TEST_SOLUTION)
    get_filename_component(TASK_NAME . NAME)
    target_include_directories(${TARGET}
      PRIVATE ../private/${TASK_NAME})
  endif()

  target_include_directories(${TARGET}
    PRIVATE .)
endfunction()

function(prepend VAR PREFIX)
  set(LIST_VAR "")
  foreach(ELEM ${ARGN})
    list(APPEND LIST_VAR "${PREFIX}/${ELEM}")
  endforeach()
  set(${VAR} "${LIST_VAR}" PARENT_SCOPE)
endfunction()

function(add_patched_executable NAME)
  set(MULTI_VALUE_ARGS PRIVATE_TESTS SOLUTION_SRCS)
  cmake_parse_arguments(PATCHED_LIBRARY "" "" "${MULTI_VALUE_ARGS}" ${ARGN})

  get_filename_component(TASK_NAME . NAME)
  if (TEST_SOLUTION)
    prepend(PATCHED_LIBRARY_SOLUTION_SRCS "../private/${TASK_NAME}" ${PATCHED_LIBRARY_SOLUTION_SRCS})
  endif()

  add_executable(${NAME}
    ${PATCHED_LIBRARY_UNPARSED_ARGUMENTS}
    ${PATCHED_LIBRARY_SOLUTION_SRCS}
    ${PATCHED_LIBRARY_PRIVATE_TESTS})

  patch_include_directories(${NAME})
endfunction()

add_custom_target(test-all)

function(add_catch TARGET)
  add_patched_executable(${TARGET}
    ${ARGN})

  target_link_libraries(${TARGET}
    contrib_catch_main)

  if (TEST_SOLUTION)
    add_custom_target(
      run_${TARGET}
      WORKING_DIRECTORY .
      DEPENDS ${TARGET}
      COMMAND ${CMAKE_BINARY_DIR}/${TARGET})

    add_dependencies(test-all run_${TARGET})
  endif()
endfunction()

function(add_benchmark TARGET)
  add_patched_executable(${TARGET}
    ${ARGN})

  target_link_libraries(${TARGET}
    benchmark)

  if (TEST_SOLUTION)
    add_custom_target(
      run_${TARGET}
      WORKING_DIRECTORY .
      DEPENDS ${TARGET}
      COMMAND ${CMAKE_BINARY_DIR}/${TARGET})

    add_dependencies(test-all run_${TARGET})
  endif()
endfunction()

function(add_gtest TARGET)
  add_patched_executable(${TARGET}
    ${ARGN})

  target_link_libraries(${TARGET}
    GTest::gtest_main)

  if (TEST_SOLUTION)
    add_custom_target(
      run_${TARGET}
      WORKING_DIRECTORY .
      DEPENDS ${TARGET}
      COMMAND ${CMAKE_BINARY_DIR}/${TARGET})

    add_dependencies(test-all run_${TARGET})
  endif()
endfunction()
