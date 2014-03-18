project(opcode)

add_definitions(-DTORQUE_OPCODE -DICE_NO_DLL)

include_directories(.)

addLibrary("/;/Ice" "opcode" GLOB)
