
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/lib/IlmCtl" "${PROJECT_SOURCE_DIR}/lib/IlmCtlMath" "${PROJECT_SOURCE_DIR}/lib/CtlCodeEmitter" )

add_executable( ctlcc
  main.cpp
)

target_link_libraries( ctlcc CtlCodeEmitter IlmCtlMath IlmCtl ${IlmBase_LIBRARIES} )
target_link_libraries( ctlcc ${IlmBase_LDFLAGS_OTHER} )

install( TARGETS ctlcc DESTINATION bin )
