
#[[
  .brief Search for C++ source files in a directory recursively.
  .result source_files The variable to store the list of found source files.
  .param source_dirs The directories to search for C++ source files.
#]]
function(brabbit_search_cxx_source_files source_files source_dirs)
  if(NOT source_dirs)
    message(FATAL_ERROR "At least one source directory must be provided.")
  endif()

  set(files)

  foreach(source_dir IN LISTS source_dirs)
    if(NOT IS_DIRECTORY ${source_dir})
      message(FATAL_ERROR "The source directory '${source_dir}' does not exist or is not a directory.")
    endif()

    file(GLOB_RECURSE cxx_files
      ${source_dir}/*.c
      ${source_dir}/*.cc
      ${source_dir}/*.cpp
      ${source_dir}/*.cxx
    )

    if(APPLE)
      file(GLOB_RECURSE objcxx_files ${source_dir}/*.mm)
      list(APPEND cxx_files ${objcxx_files})
    endif()

    list(APPEND files ${cxx_files})
  endforeach()

  set(${source_files} ${files} PARENT_SCOPE)
endfunction()



#[[
  .brief Auto set up the Electron build environment.
  .detail Auto set up the CMake environment for electron without using cmake-js to configure,
          and skip the evironment setup if the relevant variables are already defined (e.g. by cmake-js).
]]#
function(brabbit_setup_electron_environment)
  # CMAKE_CXX_FLAGS
  if(NOT DEFINED CMAKE_CXX_FLAGS OR CMAKE_CXX_FLAGS STREQUAL "")
    set(CMAKE_CXX_FLAGS "-DBUILDING_NODE_EXTENSION" PARENT_SCOPE)
    message(STATUS "[brabbit] Setting CMAKE_CXX_FLAGS to '-DBUILDING_NODE_EXTENSION'")
  endif()

  # CMAKE_SHARED_LINKER_FLAGS and platform-specific settings for link
  if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug> PARENT_SCOPE)
    message(STATUS "[brabbit] Setting CMAKE_MSVC_RUNTIME_LIBRARY to 'MultiThreaded$<$<CONFIG:Debug>:Debug>' for Windows")

    if(NOT CMAKE_SHARED_LINKER_FLAGS MATCHES "DELAYLOAD")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DELAYLOAD:NODE.EXE" PARENT_SCOPE)
      message(STATUS "[brabbit] Appended '/DELAYLOAD:NODE.EXE' to CMAKE_SHARED_LINKER_FLAGS for Windows")
    endif()

    # The cmake-js set the /DELAYLOAD:NODE.EXE linker flag into CMAKE_SHARED_LINKER_FLAGS on Windows.
    # But clang-cl's toolchain can't handle it, so we need to add delayimp.lib to the link libraries manually.
    link_libraries(delayimp.lib)
    message(STATUS "[brabbit] Adding 'delayimp.lib' to link libraries for Windows")

  elseif(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "${CMAKE_SYSTEM_PROCESSOR}" PARENT_SCOPE)

    if(NOT CMAKE_SHARED_LINKER_FLAGS MATCHES "-undefined dynamic_lookup")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup" PARENT_SCOPE)
      message(STATUS "[brabbit] Appended '-undefined dynamic_lookup' to CMAKE_SHARED_LINKER_FLAGS for macOS")
    endif()
  endif()

  set(pnpm_dir "${CMAKE_SOURCE_DIR}/node_modules/.pnpm")

  # CMAKE_JS_INC: node-api-headers + node-addon-api include paths
  if(NOT DEFINED CMAKE_JS_INC)
    file(GLOB napi_headers_dir "${pnpm_dir}/node-api-headers@*/node_modules/node-api-headers/include")
    file(GLOB napi_addon_dir "${pnpm_dir}/node-addon-api@*/node_modules/node-addon-api")
    set(CMAKE_JS_INC ${napi_headers_dir} ${napi_addon_dir} PARENT_SCOPE)
    message(STATUS "[brabbit] Setting CMAKE_JS_INC to '${napi_headers_dir};${napi_addon_dir}'")
  endif()

  # CMAKE_JS_SRC: delay-load hook source on Windows
  if(NOT DEFINED CMAKE_JS_SRC)
    if(WIN32)
      file(GLOB cmakejs_src "${pnpm_dir}/cmake-js@*/node_modules/cmake-js/lib/cpp/win_delay_load_hook.cc")
      set(CMAKE_JS_SRC ${cmakejs_src} PARENT_SCOPE)
    else()
      set(CMAKE_JS_SRC "" PARENT_SCOPE)
    endif()
    message(STATUS "[brabbit] Setting CMAKE_JS_SRC to '${cmakejs_src}'")
  endif()

  # CMAKE_JS_LIB: node.lib on Windows
  if(NOT DEFINED CMAKE_JS_LIB)
    if(WIN32)
      set(cmake_js_lib "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/node.lib")
      file(GLOB cmake_js_nodelib_def "${pnpm_dir}/node-api-headers@*/node_modules/node-api-headers/def/node_api.def")
      set(cmake_js_nodelib_target "${cmake_js_lib}")
      set(CMAKE_JS_LIB ${cmake_js_lib} PARENT_SCOPE)
      set(CMAKE_JS_NODELIB_DEF ${cmake_js_nodelib_def} PARENT_SCOPE)
      set(CMAKE_JS_NODELIB_TARGET ${cmake_js_nodelib_target} PARENT_SCOPE)
      message(STATUS "[brabbit] Setting CMAKE_JS_LIB to '${cmake_js_lib}'")
      message(STATUS "[brabbit] Setting CMAKE_JS_NODELIB_DEF to '${cmake_js_nodelib_def}'")
      message(STATUS "[brabbit] Setting CMAKE_JS_NODELIB_TARGET to '${cmake_js_nodelib_target}'")
    else()
      set(CMAKE_JS_LIB "" PARENT_SCOPE)
      message(STATUS "[brabbit] Setting CMAKE_JS_LIB to ''")
    endif()
  endif()

  # NODE_ARCH: map from CMAKE_SYSTEM_PROCESSOR
  if(NOT DEFINED NODE_ARCH)
    set(node_arch)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
      set(node_arch "x64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
      set(node_arch "arm64")
    else()
      message(FATAL_ERROR "Unsupported NODE_ARCH: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()
    set(NODE_ARCH ${node_arch} PARENT_SCOPE)
    message(STATUS "[brabbit] Setting NODE_ARCH to '${node_arch}' based on CMAKE_SYSTEM_PROCESSOR '${CMAKE_SYSTEM_PROCESSOR}'")
  endif()

  file(READ "${CMAKE_SOURCE_DIR}/package.json" package_json)

  # NODE_RUNTIME: from package.json cmake-js config
  if(NOT DEFINED NODE_RUNTIME)
    string(JSON cmakejs_runtime GET ${package_json} "cmake-js" "runtime")
    set(NODE_RUNTIME ${cmakejs_runtime} PARENT_SCOPE)
    message(STATUS "[brabbit] Setting NODE_RUNTIME to '${cmakejs_runtime}' from package.json -> cmake-js -> runtime")
  endif()

  # NODE_RUNTIMEVERSION: from package.json cmake-js config
  if(NOT DEFINED NODE_RUNTIMEVERSION)
    string(JSON cmakejs_runtime_version GET ${package_json} "cmake-js" "runtimeVersion")
    set(NODE_RUNTIMEVERSION ${cmakejs_runtime_version} PARENT_SCOPE)
    message(STATUS "[brabbit] Setting NODE_RUNTIMEVERSION to '${cmakejs_runtime_version}' from package.json -> cmake-js -> runtimeVersion")
  endif()

  # Generate node.lib from .def file on Windows (cmake-js v8 Node-API mode requires this)
  if(WIN32)
    if(CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
      set(nodelib_def "${CMAKE_JS_NODELIB_DEF}")
      set(nodelib_out "${CMAKE_JS_NODELIB_TARGET}")
    elseif(cmake_js_nodelib_def AND cmake_js_nodelib_target)
      set(nodelib_def "${cmake_js_nodelib_def}")
      set(nodelib_out "${cmake_js_nodelib_target}")
    endif()

    if(nodelib_def AND nodelib_out AND NOT EXISTS "${nodelib_out}")
      message(STATUS "[brabbit] Generating node.lib from '${nodelib_def}' to '${nodelib_out}' using ${CMAKE_AR}")

      get_filename_component(nodelib_out_dir "${nodelib_out}" DIRECTORY)
      file(MAKE_DIRECTORY "${nodelib_out_dir}")
      execute_process(
        COMMAND ${CMAKE_AR} /def:${nodelib_def} /out:${nodelib_out} /machine:${CMAKE_SYSTEM_PROCESSOR}
        RESULT_VARIABLE nodelib_result
      )

      if(NOT nodelib_result EQUAL 0)
        message(WARNING "[brabbit] Failed to generate node.lib (exit code: ${nodelib_result})")
      else()
        message(STATUS "[brabbit] Generated node.lib at '${nodelib_out}'")
      endif()

    endif()
  endif()
endfunction()



#[[
  .brief Copy dependent shared libraries from conan_packages to output directory on Windows.
]]#
function(brabbit_copy_conan_packages_to_output_directory)
  if(NOT WIN32)
    return()
  endif()

  set(from_dir "${CMAKE_SOURCE_DIR}/conan_packages/bin")
  set(to_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

  file(GLOB conan_dlls "${from_dir}/*.dll")
  if(conan_dlls)
    file(COPY ${conan_dlls} DESTINATION "${to_dir}")
    message(STATUS "[brabbit] Copied conan DLLs from '${from_dir}' to '${to_dir}'")
  endif()
endfunction()
