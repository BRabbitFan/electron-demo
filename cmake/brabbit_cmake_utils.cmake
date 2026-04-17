
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
  if(NOT DEFINED CMAKE_CXX_FLAGS OR CMAKE_CXX_FLAGS STREQUAL "")
    set(CMAKE_CXX_FLAGS "-DBUILDING_NODE_EXTENSION" PARENT_SCOPE)
    message(STATUS "[brabbit] Setting CMAKE_CXX_FLAGS to '-DBUILDING_NODE_EXTENSION'")
  elseif(NOT CMAKE_CXX_FLAGS MATCHES "-DBUILDING_NODE_EXTENSION")
    string(APPEND CMAKE_CXX_FLAGS " -DBUILDING_NODE_EXTENSION")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" PARENT_SCOPE)
    message(STATUS "[brabbit] Appending '-DBUILDING_NODE_EXTENSION' to CMAKE_CXX_FLAGS, now it is '${CMAKE_CXX_FLAGS}'")
  endif()

  if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug> PARENT_SCOPE)
    message(STATUS "[brabbit] Setting CMAKE_MSVC_RUNTIME_LIBRARY to 'MultiThreaded$<$<CONFIG:Debug>:Debug>' for Windows")

    # The cmake-js set the /DELAYLOAD:node.exe linker flag into CMAKE_SHARED_LINKER_FLAGS on Windows.
    # But clang-cl's toolchain can't handle it, so we need to remove it and add delayimp.lib to the link libraries.
    link_libraries(delayimp.lib)
    message(STATUS "[brabbit] Adding 'delayimp.lib' to link libraries for Windows")
  endif()

  set(_pnpm_dir "${CMAKE_SOURCE_DIR}/node_modules/.pnpm")

  # CMAKE_JS_INC: node-api-headers + node-addon-api include paths
  if(NOT DEFINED CMAKE_JS_INC)
    file(GLOB _NAPI_HEADERS_DIR "${_pnpm_dir}/node-api-headers@*/node_modules/node-api-headers/include")
    file(GLOB _NAPI_ADDON_DIR "${_pnpm_dir}/node-addon-api@*/node_modules/node-addon-api")
    set(CMAKE_JS_INC ${_NAPI_HEADERS_DIR} ${_NAPI_ADDON_DIR} PARENT_SCOPE)
    message(STATUS "[brabbit] Setting CMAKE_JS_INC to '${_NAPI_HEADERS_DIR};${_NAPI_ADDON_DIR}'")
  endif()

  # CMAKE_JS_SRC: delay-load hook source on Windows
  if(NOT DEFINED CMAKE_JS_SRC)
    if(WIN32)
      file(GLOB _CMAKEJS_SRC "${_pnpm_dir}/cmake-js@*/node_modules/cmake-js/lib/cpp/win_delay_load_hook.cc")
      set(CMAKE_JS_SRC ${_CMAKEJS_SRC} PARENT_SCOPE)
    else()
      set(CMAKE_JS_SRC "" PARENT_SCOPE)
    endif()
    message(STATUS "[brabbit] Setting CMAKE_JS_SRC to '${_CMAKEJS_SRC}'")
  endif()

  # CMAKE_JS_LIB: node.lib on Windows
  if(NOT DEFINED CMAKE_JS_LIB)
    if(WIN32)
      file(GLOB _CMAKE_JS_NODELIB_DEF "${_pnpm_dir}/node-api-headers@*/node_modules/node-api-headers/def/node_api.def")
      set(CMAKE_JS_LIB ${CMAKE_SOURCE_DIR}/build/node.lib PARENT_SCOPE)
      set(CMAKE_JS_NODELIB_DEF ${_CMAKE_JS_NODELIB_DEF} PARENT_SCOPE)
      set(CMAKE_JS_NODELIB_TARGET ${CMAKE_SOURCE_DIR}/build/node.lib PARENT_SCOPE)
      message(STATUS "[brabbit] Setting CMAKE_JS_LIB to '${CMAKE_JS_LIB}'")
      message(STATUS "[brabbit] Setting CMAKE_JS_NODELIB_DEF to '${CMAKE_JS_NODELIB_DEF}'")
      message(STATUS "[brabbit] Setting CMAKE_JS_NODELIB_TARGET to '${CMAKE_JS_NODELIB_TARGET}'")
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
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64")
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
endfunction()
