set(LIBNAME "cacaoAOloopControlDM")
set(SRCNAME "AOloopControl_DM")

message("")
message(" SRCNAME = ${SRCNAME} -> LIBNAME = ${LIBNAME}")


set(SOURCEFILES
	${SRCNAME}.c
    ${SRCNAME}.h
    AOloopControl_DM_comb.c
    DMturbulence.c
    mk3Ddmgrid.c
    pokerndmodes.c
)

set(INCLUDEFILES
	${SRCNAME}.h
    AOloopControl_DM_comb.h
)


set(SCRIPTS
    scripts/cacao-astrogrid-mkgrid
    scripts/cacao-fpslistadd-DMATMTURB
    scripts/cacao-fpslistadd-DMCH2DISP
    scripts/cacao-fpslistadd-DMCH2DISPSIM
    scripts/cacao-fpslistadd-DMSIMDELAY
    scripts/cacao-DMastrogrid
)

set(LINKLIBS
	CLIcore
    milkfft
    milkimagegen
    milkimagefilter
    cacaoAOloopControl
)




# DEFAULT SETTINGS
# Do not change unless needed
# =====================================================================

project(lib_${LIBNAME}_project)

include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/..")

# Library can be compiled from multiple source files
# Convention: the main souce file is named <libname>.c
#
add_library(${LIBNAME} SHARED ${SOURCEFILES})

target_include_directories(${LIBNAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${LIBNAME} PRIVATE ${LINKLIBS})

install(TARGETS ${LIBNAME} DESTINATION lib)
install(FILES ${INCLUDEFILES} DESTINATION include/${SRCNAME})

install(PROGRAMS ${SCRIPTS} DESTINATION bin)
