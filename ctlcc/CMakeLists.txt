
if (TIFF_FOUND)
add_definitions( -DHAVE_LIBTIFF=1 )
include_directories( ${TIFF_INCLUDE_DIRS} )
endif()

if ( OpenEXR_FOUND )
add_definitions( -DHAVE_OPENEXR=1 )
include_directories( ${OpenEXR_INCLUDE_DIRS} )
endif()

if ( AcesContainer_FOUND )
add_definitions( -DHAVE_ACESFILE=1 )
include_directories( ${AcesContainer_INCLUDE_DIRS} )
endif()

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/lib/IlmCtl" "${PROJECT_SOURCE_DIR}/lib/IlmCtlMath" "${PROJECT_SOURCE_DIR}/lib/CtlCodeEmitter" )

add_executable( ctlcc
  main.cpp
)

include_directories( "${PROJECT_SOURCE_DIR}/lib/dpx" )

add_library( CtlCCDriver STATIC
  driver/file_io.cpp
  driver/main.cpp
)

set_target_properties( CtlCCDriver PROPERTIES COMPILE_FLAGS "-std=c++0x" )

install( TARGETS CtlCCDriver DESTINATION lib )

target_link_libraries( ctlcc CtlCodeEmitter IlmCtlMath IlmCtl ${IlmBase_LIBRARIES} )
target_link_libraries( ctlcc ${IlmBase_LDFLAGS_OTHER} )

install( TARGETS ctlcc DESTINATION bin )
