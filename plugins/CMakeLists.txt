# cmake magic to load all folders in plugins

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children ${curdir}/*) # This was changed
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${child}) # This was changed
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
FOREACH(subdir ${SUBDIRS})
  message(STATUS ${subdir})
  get_filename_component(FNAME ${subdir} NAME)
  add_subdirectory(${subdir}/${FNAME})
  include_directories(${subdir}/)
ENDFOREACH()

# macro(add_plugin path)
#   message(STATUS ${path})
#   get_filename_component(FNAME ${path} NAME)
#   add_subdirectory(${path}/${FNAME})
#   include_directories(${path}/)
# endmacro()
# 
# add_plugin("shader_basics")
