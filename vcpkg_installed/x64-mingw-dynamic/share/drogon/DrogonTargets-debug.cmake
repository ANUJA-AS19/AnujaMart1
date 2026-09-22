#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Drogon::Drogon" for configuration "Debug"
set_property(TARGET Drogon::Drogon APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Drogon::Drogon PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/debug/lib/libdrogon.dll.a"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/bin/libdrogon.dll"
  )

list(APPEND _cmake_import_check_targets Drogon::Drogon )
list(APPEND _cmake_import_check_files_for_Drogon::Drogon "${_IMPORT_PREFIX}/debug/lib/libdrogon.dll.a" "${_IMPORT_PREFIX}/debug/bin/libdrogon.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
