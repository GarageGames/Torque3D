cmake_minimum_required (VERSION 2.6)

project(opcode)

add_definitions(-DTORQUE_OPCODE -DICE_NO_DLL)

include_directories(.)

addLibrary("/;/Ice" "opcode" GLOB)
