project(mymillions)

set(HEADERS
	mymillions.hpp
	tables.hpp
    common.hpp
	config.hpp
	logger.hpp
)

set(SOURCES ${HEADERS}
	mymillions.cpp
)

macro(add_contract_project CONTRACT_NAME BINOUTPUT)
	add_executable( ${CONTRACT_NAME}.wasm ${ARGN} )
	target_compile_options( ${CONTRACT_NAME}.wasm PUBLIC -abigen )
	set_target_properties(${CONTRACT_NAME}.wasm PROPERTIES
			ARCHIVE_OUTPUT_DIRECTORY "${BINOUTPUT}/${CONTRACT_NAME}/"
			LIBRARY_OUTPUT_DIRECTORY "${BINOUTPUT}/${CONTRACT_NAME}/"
			RUNTIME_OUTPUT_DIRECTORY "${BINOUTPUT}/${CONTRACT_NAME}/")
	get_target_property(BINOUTPUT ${CONTRACT_NAME}.wasm BINARY_DIR)
	target_compile_options( ${CONTRACT_NAME}.wasm PUBLIC -abigen_output=${BINOUTPUT}/${CONTRACT_NAME}/${CONTRACT_NAME}.abi)
	target_compile_options( ${CONTRACT_NAME}.wasm PUBLIC -contract ${CONTRACT_NAME} )
	if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/)
		target_compile_options( ${CONTRACT_NAME}.wasm PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/resources/)
	endif()
endmacro(add_contract_project)

add_contract_project(mymillions ${OUTPUT_ROOT} ${SOURCES})

target_include_directories(mymillions.wasm PUBLIC ${SOURCES_ROOT})