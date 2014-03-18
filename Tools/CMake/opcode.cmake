project(opcode)

addLibrary("${libDir}/opcode;${libDir}/opcode/Ice" opcode GLOB)

set_property(TARGET opcode PROPERTY COMPILE_DEFINITIONS TORQUE_OPCODE ICE_NO_DLL)
