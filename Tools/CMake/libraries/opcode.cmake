project(opcode)

addPath("${libDir}/${PROJECT_NAME}")
addPath("${libDir}/${PROJECT_NAME}/Ice")

addDef(TORQUE_OPCODE)
addDef(ICE_NO_DLL)

finishLibrary()
