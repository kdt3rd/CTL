include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/lib/IlmCtl" "${PROJECT_SOURCE_DIR}/lib/IlmCtlMath" )

include_directories( "${CMAKE_CURRENT_BINARY_DIR}" )

add_library( CtlCodeEmitter ${DO_SHARED}
	CtlCodeAddr.cpp
	CtlCodeFunctionCall.cpp
	CtlCodeInterpreter.cpp
	CtlCodeLContext.cpp
	CtlCodeModule.cpp
	CtlCodeType.cpp
	CtlCodeSyntaxTree.cpp
	CtlCodeStdTypes.cpp
	CtlCodeStdLibrary.cpp
	CtlCodeLanguageGenerator.cpp
	CtlCodeCCommonLanguage.cpp
	CtlCodeCPPLanguage.cpp
	CtlCodeCUDALanguage.cpp
	CtlCodeOPENCLLanguage.cpp
	CtlCodeNukeLanguage.cpp
)

target_link_libraries( CtlCodeEmitter IlmCtlMath IlmCtl )

set_target_properties( CtlCodeEmitter PROPERTIES
  VERSION ${CTL_VERSION}
  SOVERSION ${CTL_VERSION}
)

install( FILES CtlCodeInterpreter.h DESTINATION include/CTL )

install( TARGETS CtlCodeEmitter DESTINATION lib )
