#
# cMake configuration file
#
# cmake required (http://www.cmake.org/)
#

#Dushan - Elaboration
#       - I have increased minimum required version of cMake because
#       - of new version of Visual Studio 2013 Preview
CMAKE_MINIMUM_REQUIRED( VERSION 2.8.11 )
MESSAGE( STATUS "Using cmake version ${CMAKE_VERSION}" )

SET( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" )
SET( ENGINE_DIR ${CMAKE_SOURCE_DIR}/Engine/ )

# Options
OPTION( USE_BULLETPHYSICS          "Use Bullet Physics"                          OFF )
OPTION( USE_NVIDIAPHYSX            "Use nVidia PhysX"                            OFF )
OPTION( USE_OPENGL                 "Use OpenGL renderer"                         OFF )
OPTION( USE_LEAPMOTIONINPUT        "Use Leap Motion Input"                       OFF )
OPTION( USE_RAZERHYDRAINPUT        "Use Razer Hydra Input"                       OFF )
OPTION( USE_FMODSOUNDSYSTEM        "Use FMOD Sound System"                       OFF )
OPTION( USE_OCULUSRIFT             "Use Oculus Rift"                             OFF )
OPTION( USE_RECASTNAVITATION       "Use Recast Navigation"                       OFF )
OPTION( TORQUE_HIFI_NET            "Use HiFi Net"                                OFF )
OPTION( TORQUE_EXTENDED_MOVE       "Use Extended Move"                           OFF )
#TODO - Dushan - Do we really need this????
#              - I am not sure so I didn't completed that, but
#              - if needed it isn't problem to be completed
#OPTION( USE_WEBPLUGINS             "Use WebPlugins"                              OFF )

SET( NETAPP_DIR ${CMAKE_SOURCE_DIR}/Engine/source/app/net )
SET( SRCAPP_DIR ${CMAKE_SOURCE_DIR}/Engine/source/app )
SET( CINT_DIR ${CMAKE_SOURCE_DIR}/Engine/source/cinterface )
SET( COLL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/collision )
SET( COMP_DIR ${CMAKE_SOURCE_DIR}/Engine/source/component )
SET( COMPINT_DIR ${CMAKE_SOURCE_DIR}/Engine/source/component/interfaces )
SET( CONSOLE_DIR ${CMAKE_SOURCE_DIR}/Engine/source/console )
SET( CORE_DIR ${CMAKE_SOURCE_DIR}/Engine/source/core )
SET( COREOGG_DIR ${CMAKE_SOURCE_DIR}/Engine/source/core/ogg )
SET( CORESTREAM_DIR ${CMAKE_SOURCE_DIR}/Engine/source/core/stream )
SET( CORESTRINGS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/core/strings )
SET( COREUTIL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/core/util )
SET( COREUTILJOURNAL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/core/util/journal )
SET( COREUTILJOURNALTEST_DIR ${CMAKE_SOURCE_DIR}/Engine/source/core/util/journal/test )
SET( COREUTILTEST_DIR ${CMAKE_SOURCE_DIR}/Engine/source/core/util/test )
SET( COREUTILZIP_DIR ${CMAKE_SOURCE_DIR}/Engine/source/core/util/zip )
SET( COREUTILZIPCOMPRESSORS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/core/util/zip/compressors )
SET( COREUTILZIPUNITTESTS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/core/util/zip/unitTests )
SET( ENVIRONMENT_DIR ${CMAKE_SOURCE_DIR}/Engine/source/environment )
SET( ENVIRONMENTEDITORS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/environment/editors )
SET( FOREST_DIR ${CMAKE_SOURCE_DIR}/Engine/source/forest )
SET( FORESTEDITOR_DIR ${CMAKE_SOURCE_DIR}/Engine/source/forest/editor )

IF( USE_OPENGL )
    SET( FORESTGLSL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/forest/glsl )
ENDIF( USE_OPENGL )

SET( FORESTHLSL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/forest/hlsl )
SET( FORESTTS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/forest/ts )
SET( GFX_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gfx )
SET( GFXBITMAP_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gfx/bitmap )
SET( GFXBITMAPLOADER_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gfx/bitmap/loaders )
SET( D3D9_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gfx/d3d9 )
SET( D3D9PC_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gfx/d3d9/pc )

IF( USE_OPENGL )
    SET( GL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gfx/gl )
	SET( GLGGL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gfx/gl/ggl )
	SET( GLGGLWIN32_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gfx/gl/ggl/win32 )
ENDIF( USE_OPENGL )

SET( NULL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gfx/Null )
SET( GFXSIM_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gfx/sim )
SET( GFXTEST_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gfx/test )
SET( GFXUTIL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gfx/util )
SET( GFXVIDEO_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gfx/video )
SET( GUI3D_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gui/3d )
SET( GUIBUTTONS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gui/buttons )
SET( GUICONTAINERS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gui/containers )
SET( GUICONTROLS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gui/controls )
SET( GUICORE_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gui/core )
SET( GUIEDITOR_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gui/editor )
SET( GUIEDITORINSPECTOR_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gui/editor/inspector )
SET( GUIGAME_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gui/game )
SET( GUISHINY_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gui/shiny )
SET( GUITHEORA_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gui/theora )
SET( GUIUTILITY_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gui/utility )
SET( GUIWORDLEDITOR_DIR ${CMAKE_SOURCE_DIR}/Engine/source/gui/worldEditor )
SET( I18N_DIR ${CMAKE_SOURCE_DIR}/Engine/source/i18n )
SET( LIGHTING_DIR ${CMAKE_SOURCE_DIR}/Engine/source/lighting )
SET( LIGHTINGADVANCED_DIR ${CMAKE_SOURCE_DIR}/Engine/source/lighting/advanced )

IF( USE_OPENGL )
    SET( LIGHTINGADVANCEDGLSL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/lighting/advanced/glsl )
ENDIF( USE_OPENGL )

SET( LIGHTINGADVANCEDHLSL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/lighting/advanced/hlsl )
SET( LIGHTINGCOMMON_DIR ${CMAKE_SOURCE_DIR}/Engine/source/lighting/common )
SET( LIGHTINGBASIC_DIR ${CMAKE_SOURCE_DIR}/Engine/source/lighting/basic )
SET( LIGHTINGSHADOWMAP_DIR ${CMAKE_SOURCE_DIR}/Engine/source/lighting/shadowMap )
SET( MATERIALS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/materials )
SET( MATH_DIR ${CMAKE_SOURCE_DIR}/Engine/source/math )

IF( USE_RECASTNAVITATION )
    SET( NAVIGATION_DIR ${CMAKE_SOURCE_DIR}/Engine/source/navigation )
ENDIF ( USE_RECASTNAVITATION )

SET( MATHTEST_DIR ${CMAKE_SOURCE_DIR}/Engine/source/math/test )
SET( MATHUTIL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/math/util )
SET( PLATFORM_DIR ${CMAKE_SOURCE_DIR}/Engine/source/platform )
SET( PLATFORMASYNC_DIR ${CMAKE_SOURCE_DIR}/Engine/source/platform/async )
SET( PLATFORMINPUT_DIR ${CMAKE_SOURCE_DIR}/Engine/source/platform/input )

IF( USE_LEAPMOTIONINPUT )
    SET( PLATFORMINPUTLEAPMOTION_DIR ${CMAKE_SOURCE_DIR}/Engine/source/platform/input/leapMotion )
ENDIF( USE_LEAPMOTIONINPUT )

IF( USE_OCULUSRIFT )
    SET( PLATFORMOCULUS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/platform/input/oculusVR )
ENDIF( USE_OCULUSRIFT )

IF( USE_RAZERHYDRAINPUT )
    SET( PLATFORMINPUTRAZERHYDRA_DIR ${CMAKE_SOURCE_DIR}/Engine/source/platform/input/razerHydra )
ENDIF( USE_RAZERHYDRAINPUT )

SET( PLATFORMMENUS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/platform/menus )
SET( PLATFORMNATIVEDIALOGS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/platform/nativeDialogs )
SET( PLATFORMOUTPUT_DIR ${CMAKE_SOURCE_DIR}/Engine/source/platform/output )
SET( PLATFORMTEST_DIR ${CMAKE_SOURCE_DIR}/Engine/source/platform/test )
SET( PLATFORMTHREADS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/platform/threads )
SET( PLATFORMWIN32_DIR ${CMAKE_SOURCE_DIR}/Engine/source/platformWin32 )
SET( PLATFORMWIN32MENUS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/platformWin32/menus )
SET( PLATFORMWIN32MINIDUMP_DIR ${CMAKE_SOURCE_DIR}/Engine/source/platformWin32/minidump )
SET( PLATFORMWIN32NATIVEDIALOGS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/platformWin32/nativeDialogs )
SET( PLATFORMWIN32THREADS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/platformWin32/threads )
SET( PLATFORMWIN32VIDEOINFO_DIR ${CMAKE_SOURCE_DIR}/Engine/source/platformWin32/videoInfo )
SET( POSTFX_DIR ${CMAKE_SOURCE_DIR}/Engine/source/postFx )
SET( RENDERINSTANCE_DIR ${CMAKE_SOURCE_DIR}/Engine/source/renderInstance )
SET( SCENE_DIR ${CMAKE_SOURCE_DIR}/Engine/source/scene )
SET( SCENECULLING_DIR ${CMAKE_SOURCE_DIR}/Engine/source/scene/culling )
SET( SCENEMIXIN_DIR ${CMAKE_SOURCE_DIR}/Engine/source/scene/mixin )
SET( SCENEZONES_DIR ${CMAKE_SOURCE_DIR}/Engine/source/scene/zones )
SET( SFX_DIR ${CMAKE_SOURCE_DIR}/Engine/source/sfx )
SET( SFXDSOUND_DIR ${CMAKE_SOURCE_DIR}/Engine/source/sfx/dsound )

IF( USE_FMODSOUNDSYSTEM )
    SET( SFXFMOD_DIR ${CMAKE_SOURCE_DIR}/Engine/source/sfx/fmod )
ENDIF( USE_FMODSOUNDSYSTEM )

SET( SFXMEDIA_DIR ${CMAKE_SOURCE_DIR}/Engine/source/sfx/media )
SET( SFXNULL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/sfx/null )
SET( SFXOPENAL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/sfx/openal )
SET( SFXOPENALWIN32_DIR ${CMAKE_SOURCE_DIR}/Engine/source/sfx/openal/win32 )
SET( SFXXAUDIO_DIR ${CMAKE_SOURCE_DIR}/Engine/source/sfx/xaudio )
SET( SHADERGEN_DIR ${CMAKE_SOURCE_DIR}/Engine/source/shaderGen )

IF( USE_OPENGL )
    SET( SHADERGENGLSL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/shaderGen/GLSL )
ENDIF( USE_OPENGL )

SET( SHADERGENHLSL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/shaderGen/HLSL )
SET( SIM_DIR ${CMAKE_SOURCE_DIR}/Engine/source/sim )
SET( T3D_DIR ${CMAKE_SOURCE_DIR}/Engine/source/T3D )
SET( T3DDECAL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/T3D/decal )
SET( T3DEXAMPLES_DIR ${CMAKE_SOURCE_DIR}/Engine/source/T3D/examples )
SET( T3DFPS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/T3D/fps )
SET( T3DFX_DIR ${CMAKE_SOURCE_DIR}/Engine/source/T3D/fx )
SET( T3DGAMEBASE_DIR ${CMAKE_SOURCE_DIR}/Engine/source/T3D/gameBase )

IF( TORQUE_EXTENDED_MOVE )
    SET( T3DGAMEBASEEXTENDED_DIR ${CMAKE_SOURCE_DIR}/Engine/source/T3D/gameBase/extended )
ENDIF( TORQUE_EXTENDED_MOVE )

IF( TORQUE_HIFI_NET )
    SET( T3DGAMEBASEHIFI_DIR ${CMAKE_SOURCE_DIR}/Engine/source/T3D/gameBase/hifi )
ENDIF( TORQUE_HIFI_NET )

SET( T3DGAMEBASESTD_DIR ${CMAKE_SOURCE_DIR}/Engine/source/T3D/gameBase/std )
SET( T3DPHYSICS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/T3D/physics )

IF( USE_BULLETPHYSICS )
    SET( T3DPHYSICSBULLET_DIR ${CMAKE_SOURCE_DIR}/Engine/source/T3D/physics/bullet )
ENDIF( USE_BULLETPHYSICS )

IF( USE_NVIDIAPHYSX )
    SET( T3DPHYSICSPHYSX_DIR ${CMAKE_SOURCE_DIR}/Engine/source/T3D/physics/physx )
ENDIF( USE_NVIDIAPHYSX )

# Dushan - this isn't in original project
#SET( T3DSCENECOMPONENT_DIR ${CMAKE_SOURCE_DIR}/Engine/source/T3D/sceneComponent )

SET( T3DSFX_DIR ${CMAKE_SOURCE_DIR}/Engine/source/T3D/sfx )
SET( T3DTURRET_DIR ${CMAKE_SOURCE_DIR}/Engine/source/T3D/turret )
SET( T3DVEHICLES_DIR ${CMAKE_SOURCE_DIR}/Engine/source/T3D/vehicles )
SET( TERRAIN_DIR ${CMAKE_SOURCE_DIR}/Engine/source/terrain )

IF( USE_OPENGL )
    SET( TERRAINGLSL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/terrain/GLSL )
ENDIF( USE_OPENGL )

SET( TERRAINHLSL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/terrain/HLSL )
SET( TS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/ts )
SET( TSARCH_DIR ${CMAKE_SOURCE_DIR}/Engine/source/ts/arch )
SET( TSCOLLADA_DIR ${CMAKE_SOURCE_DIR}/Engine/source/ts/collada )
SET( TSLOADER_DIR ${CMAKE_SOURCE_DIR}/Engine/source/ts/loader )
SET( UNIT_DIR ${CMAKE_SOURCE_DIR}/Engine/source/unit )
SET( UNITTESTS_DIR ${CMAKE_SOURCE_DIR}/Engine/source/unit/tests )
SET( UTIL_DIR ${CMAKE_SOURCE_DIR}/Engine/source/util )
SET( UTILMESSAGING_DIR ${CMAKE_SOURCE_DIR}/Engine/source/util/messaging )
SET( WINDOWMANAGER_DIR ${CMAKE_SOURCE_DIR}/Engine/source/windowManager )
SET( WINDOWMANAGERDEDICATED_DIR ${CMAKE_SOURCE_DIR}/Engine/source/windowManager/dedicated )
SET( WINDOWMANAGERTEST_DIR ${CMAKE_SOURCE_DIR}/Engine/source/windowManager/test )
SET( WINDOWMANAGERWIN32_DIR ${CMAKE_SOURCE_DIR}/Engine/source/windowManager/win32 )

#Dushan - not finished
#IF( USE_WEBPLUGINS )
#    SET( WEBACTIVEXACTIVEX_DIR ${CMAKE_SOURCE_DIR}/Engine/source/web/activex )
#    SET( WEBACTIVEXCOMMON_DIR ${CMAKE_SOURCE_DIR}/Engine/source/common )
#    SET( WEBACTIVEXNPPLUGIN_DIR ${CMAKE_SOURCE_DIR}/Engine/source/web/npplugin )
#    SET( WEBACTIVEXNPPLUGINWIN_DIR ${CMAKE_SOURCE_DIR}/Engine/source/web/npplugin/windows )
#ENDIF( USE_WEBPLUGINS )

SET( APP_HEADERS
    ${SRCAPP_DIR}/auth.h
	${SRCAPP_DIR}/badWordFilter.h
	${SRCAPP_DIR}/banList.h
	${SRCAPP_DIR}/game.h
	${SRCAPP_DIR}/mainLoop.h
	${SRCAPP_DIR}/version.h
)

SET( APP_SOURCES
    ${SRCAPP_DIR}/badWordFilter.cpp
	${SRCAPP_DIR}/banList.cpp
	${SRCAPP_DIR}/game.cpp
	${SRCAPP_DIR}/mainLoop.cpp
	${SRCAPP_DIR}/version.cpp
)

SET( NET_HEADERS
	${NETAPP_DIR}/httpObject.h
	${NETAPP_DIR}/serverQuery.h
	${NETAPP_DIR}/tcpObject.h
)

SET( NET_SOURCES
	${NETAPP_DIR}/httpObject.cpp
	${NETAPP_DIR}/net.cpp
	${NETAPP_DIR}/netTest.cpp
	${NETAPP_DIR}/serverQuery.cpp
	${NETAPP_DIR}/tcpObject.cpp
)

SET( CINT_SOURCES
    ${CINT_DIR}/c_consoleInterface.cpp
    ${CINT_DIR}/c_scripting.cpp
	${CINT_DIR}/cinterface.cpp
)

SET( CINT_HEADERS
    ${CINT_DIR}/cinterface.h
)

SET( COLL_HEADERS
    ${COLL_DIR}/abstractPolyList.h
	${COLL_DIR}/boxConvex.h
	${COLL_DIR}/clippedPolyList.h
	${COLL_DIR}/collision.h
	${COLL_DIR}/concretePolyList.h
	${COLL_DIR}/convex.h
	${COLL_DIR}/depthSortList.h
	${COLL_DIR}/earlyOutPolyList.h
	${COLL_DIR}/extrudedPolyList.h
	${COLL_DIR}/gjk.h
	${COLL_DIR}/optimizedPolyList.h
	${COLL_DIR}/planeExtractor.h
	${COLL_DIR}/polytope.h
	${COLL_DIR}/vertexPolyList.h	
)

SET( COLL_SOURCES
    ${COLL_DIR}/abstractPolyList.cpp
	${COLL_DIR}/boxConvex.cpp
	${COLL_DIR}/clippedPolyList.cpp
	${COLL_DIR}/concretePolyList.cpp
	${COLL_DIR}/convex.cpp
	${COLL_DIR}/depthSortList.cpp
	${COLL_DIR}/earlyOutPolyList.cpp
	${COLL_DIR}/extrudedPolyList.cpp
	${COLL_DIR}/gjk.cpp
	${COLL_DIR}/optimizedPolyList.cpp
	${COLL_DIR}/planeExtractor.cpp
	${COLL_DIR}/polytope.cpp
	${COLL_DIR}/vertexPolyList.cpp
)

SET( COMPINT_HEADERS
    ${COMPINT_DIR}/IProcessInput.h
)

SET( COMP_HEADERS
    ${COMP_DIR}/componentInterface.h
    ${COMP_DIR}/dynamicConsoleMethodComponent.h
	${COMP_DIR}/moreAdvancedComponent.h
	${COMP_DIR}/simComponent.h
	${COMP_DIR}/simpleComponent.h
)

SET( COMP_SOURCES
	${COMP_DIR}/componentInterface.cpp
	${COMP_DIR}/dynamicConsoleMethodComponent.cpp
	${COMP_DIR}/moreAdvancedComponent.cpp
	${COMP_DIR}/simComponent.cpp
	${COMP_DIR}/simpleComponent.cpp
)

SET( CONSOLE_HEADERS
    ${CONSOLE_DIR}/arrayObject.h
	${CONSOLE_DIR}/ast.h
	${CONSOLE_DIR}/cmdgram.h
	${CONSOLE_DIR}/codeBlock.h
	${CONSOLE_DIR}/compiler.h
	${CONSOLE_DIR}/console.h
	${CONSOLE_DIR}/consoleDoc.h
	${CONSOLE_DIR}/consoleInternal.h
	${CONSOLE_DIR}/consoleLogger.h
	${CONSOLE_DIR}/consoleObject.h
	${CONSOLE_DIR}/consoleParser.h
	${CONSOLE_DIR}/consoleTypes.h
	${CONSOLE_DIR}/consoleXMLExport.h
	${CONSOLE_DIR}/debugOutputConsumer.h
	${CONSOLE_DIR}/dynamicTypes.h
	${CONSOLE_DIR}/engineAPI.h
	${CONSOLE_DIR}/engineExports.h
	${CONSOLE_DIR}/engineFunctions.h
	${CONSOLE_DIR}/engineObject.h
	${CONSOLE_DIR}/enginePrimitives.h
	${CONSOLE_DIR}/engineStructs.h
	${CONSOLE_DIR}/engineTypeInfo.h
	${CONSOLE_DIR}/engineTypes.h
	${CONSOLE_DIR}/fieldBrushObject.h
	${CONSOLE_DIR}/ICallMethod.h
	${CONSOLE_DIR}/persistenceManager.h
	${CONSOLE_DIR}/propertyParsing.h
	${CONSOLE_DIR}/runtimeClassRep.h
	${CONSOLE_DIR}/scriptFilename.h
	${CONSOLE_DIR}/scriptObjects.h
	${CONSOLE_DIR}/sim.h
	${CONSOLE_DIR}/simDatablock.h
	${CONSOLE_DIR}/simDictionary.h
	${CONSOLE_DIR}/simEvents.h
	${CONSOLE_DIR}/simFieldDictionary.h
	${CONSOLE_DIR}/simObject.h
	${CONSOLE_DIR}/simObjectList.h
	${CONSOLE_DIR}/simObjectMemento.h
	${CONSOLE_DIR}/simObjectRef.h
	${CONSOLE_DIR}/simPersistID.h
	${CONSOLE_DIR}/simPersistSet.h
	${CONSOLE_DIR}/simSet.h
	${CONSOLE_DIR}/SimXMLDocument.h
	${CONSOLE_DIR}/stringStack.h
	${CONSOLE_DIR}/telnetConsole.h
	${CONSOLE_DIR}/telnetDebugger.h
	${CONSOLE_DIR}/typeValidators.h
)

SET( CONSOLE_SOURCES
	${CONSOLE_DIR}/arrayObject.cpp
	${CONSOLE_DIR}/astAlloc.cpp
	${CONSOLE_DIR}/astNodes.cpp
	${CONSOLE_DIR}/cmdgram.cpp
	${CONSOLE_DIR}/CMDscan.cpp
	${CONSOLE_DIR}/codeBlock.cpp
	${CONSOLE_DIR}/compiledEval.cpp
	${CONSOLE_DIR}/compiler.cpp
	${CONSOLE_DIR}/console.cpp
	${CONSOLE_DIR}/consoleDoc.cpp
	${CONSOLE_DIR}/consoleFunctions.cpp
	${CONSOLE_DIR}/consoleInternal.cpp
	${CONSOLE_DIR}/consoleLogger.cpp
	${CONSOLE_DIR}/consoleObject.cpp
	${CONSOLE_DIR}/consoleParser.cpp
	${CONSOLE_DIR}/consoleTypes.cpp
	${CONSOLE_DIR}/consoleXMLExport.cpp
	${CONSOLE_DIR}/debugOutputConsumer.cpp
	${CONSOLE_DIR}/dynamicTypes.cpp
	${CONSOLE_DIR}/engineDoc.cpp
	${CONSOLE_DIR}/engineExports.cpp
	${CONSOLE_DIR}/engineFunctions.cpp
	${CONSOLE_DIR}/engineObject.cpp
	${CONSOLE_DIR}/enginePrimitives.cpp
	${CONSOLE_DIR}/engineStructs.cpp
	${CONSOLE_DIR}/engineTypeInfo.cpp
	${CONSOLE_DIR}/engineTypes.cpp
	${CONSOLE_DIR}/engineXMLExport.cpp
	${CONSOLE_DIR}/fileSystemFunctions.cpp
	${CONSOLE_DIR}/persistenceManager.cpp
	${CONSOLE_DIR}/propertyParsing.cpp
	${CONSOLE_DIR}/runtimeClassRep.cpp
	${CONSOLE_DIR}/scriptFilename.cpp
	${CONSOLE_DIR}/scriptObjects.cpp
	${CONSOLE_DIR}/sim.cpp
	${CONSOLE_DIR}/simDatablock.cpp
	${CONSOLE_DIR}/simDictionary.cpp
	${CONSOLE_DIR}/simEvents.cpp
	${CONSOLE_DIR}/simFieldDictionary.cpp
	${CONSOLE_DIR}/simManager.cpp
	${CONSOLE_DIR}/simObject.cpp
	${CONSOLE_DIR}/simObjectList.cpp
	${CONSOLE_DIR}/simObjectMemento.cpp
	${CONSOLE_DIR}/simPersistID.cpp
	${CONSOLE_DIR}/simPersistSet.cpp
	${CONSOLE_DIR}/simSerialize.cpp
	${CONSOLE_DIR}/simSet.cpp
	${CONSOLE_DIR}/SimXMLDocument.cpp
	${CONSOLE_DIR}/stringStack.cpp
	${CONSOLE_DIR}/telnetConsole.cpp
	${CONSOLE_DIR}/telnetDebugger.cpp
	${CONSOLE_DIR}/typeValidators.cpp
)

SET( CORE_HEADERS
    ${CORE_DIR}/bitMatrix.h
	${CORE_DIR}/bitRender.h
	${CORE_DIR}/bitSet.h
	${CORE_DIR}/bitVector.h
	${CORE_DIR}/bitVectorW.h
	${CORE_DIR}/color.h
	${CORE_DIR}/crc.h
	${CORE_DIR}/dataChunker.h
	${CORE_DIR}/dnet.h
	${CORE_DIR}/fileio.h
	${CORE_DIR}/fileObject.h
	${CORE_DIR}/filterStream.h
	${CORE_DIR}/frameAllocator.h
	${CORE_DIR}/idGenerator.h
	${CORE_DIR}/iTickable.h
	${CORE_DIR}/memVolume.h
	${CORE_DIR}/module.h
	${CORE_DIR}/resizeStream.h
	${CORE_DIR}/resource.h
	${CORE_DIR}/resourceManager.h
	${CORE_DIR}/stringBuffer.h
	${CORE_DIR}/stringTable.h
	${CORE_DIR}/tagDictionary.h
	${CORE_DIR}/tAlgorithm.h
	${CORE_DIR}/threadStatic.h
	${CORE_DIR}/tokenizer.h
	${CORE_DIR}/tSimpleHashTable.h
	${CORE_DIR}/tSparseArray.h
	${CORE_DIR}/virtualMountSystem.h
	${CORE_DIR}/volume.h	
)

SET( CORE_SOURCES
    ${CORE_DIR}/bitRender.cpp
	${CORE_DIR}/bitVector.cpp
	${CORE_DIR}/color.cpp
	${CORE_DIR}/crc.cpp
	${CORE_DIR}/dataChunker.cpp
	${CORE_DIR}/dnet.cpp
	${CORE_DIR}/fileObject.cpp
	${CORE_DIR}/filterStream.cpp
	${CORE_DIR}/frameAllocator.cpp
	${CORE_DIR}/idGenerator.cpp
	${CORE_DIR}/iTickable.cpp
	${CORE_DIR}/memVolume.cpp
	${CORE_DIR}/module.cpp
	${CORE_DIR}/resizeStream.cpp
	${CORE_DIR}/resource.cpp
	${CORE_DIR}/resourceManager.cpp
	${CORE_DIR}/stringBuffer.cpp
	${CORE_DIR}/stringTable.cpp
	${CORE_DIR}/tagDictionary.cpp
	${CORE_DIR}/threadStatic.cpp
	${CORE_DIR}/tokenizer.cpp
	${CORE_DIR}/virtualMountSystem.cpp
	${CORE_DIR}/volume.cpp
)

SET( COREOGG_HEADERS
    ${COREOGG_DIR}/oggInputStream.h
    ${COREOGG_DIR}/oggTheoraDecoder.h
	${COREOGG_DIR}/oggVorbisDecoder.h
)

SET( COREOGG_SOURCES
    ${COREOGG_DIR}/oggInputStream.cpp
    ${COREOGG_DIR}/oggTheoraDecoder.cpp
	${COREOGG_DIR}/oggVorbisDecoder.cpp
)

SET( CORESTREAM_HEADERS
    ${CORESTREAM_DIR}/bitStream.h
	${CORESTREAM_DIR}/fileStream.h
	${CORESTREAM_DIR}/fileStreamObject.h
	${CORESTREAM_DIR}/ioHelper.h
	${CORESTREAM_DIR}/memStream.h
	${CORESTREAM_DIR}/stream.h
	${CORESTREAM_DIR}/streamObject.h
	${CORESTREAM_DIR}/tStream.h
)

SET( CORESTREAM_SOURCES
    ${CORESTREAM_DIR}/bitStream.cpp
	${CORESTREAM_DIR}/fileStream.cpp
	${CORESTREAM_DIR}/fileStreamObject.cpp
	${CORESTREAM_DIR}/memStream.cpp
	${CORESTREAM_DIR}/stream.cpp
	${CORESTREAM_DIR}/streamObject.cpp
)

SET( CORESTRINGS_HEADERS
    ${CORESTRINGS_DIR}/findMatch.h
	${CORESTRINGS_DIR}/stringFunctions.h
	${CORESTRINGS_DIR}/stringUnit.h
	${CORESTRINGS_DIR}/unicode.h
)

SET( CORESTRINGS_SOURCES
    ${CORESTRINGS_DIR}/findMatch.cpp
	${CORESTRINGS_DIR}/stringFunctions.cpp
	${CORESTRINGS_DIR}/stringUnit.cpp
	${CORESTRINGS_DIR}/unicode.cpp
)

SET( COREUTIL_HEADERS
    ${COREUTIL_DIR}/autoPtr.h
    ${COREUTIL_DIR}/byteBuffer.h
    ${COREUTIL_DIR}/byteswap.h
    ${COREUTIL_DIR}/delegate.h
    ${COREUTIL_DIR}/dxt5nmSwizzle.h
    ${COREUTIL_DIR}/endian.h
    ${COREUTIL_DIR}/FastDelegate.h
    ${COREUTIL_DIR}/fourcc.h
    ${COREUTIL_DIR}/hashFunction.h
    ${COREUTIL_DIR}/md5.h
    ${COREUTIL_DIR}/namedSingleton.h
    ${COREUTIL_DIR}/noncopyable.h
    ${COREUTIL_DIR}/path.h
    ${COREUTIL_DIR}/preprocessorHelpers.h
    ${COREUTIL_DIR}/rawData.h
    ${COREUTIL_DIR}/refBase.h
    ${COREUTIL_DIR}/rgb2luv.h
    ${COREUTIL_DIR}/rgb2xyz.h
    ${COREUTIL_DIR}/safeCast.h
    ${COREUTIL_DIR}/safeDelete.h
    ${COREUTIL_DIR}/safeRelease.h
    ${COREUTIL_DIR}/str.h
    ${COREUTIL_DIR}/swizzle.h
    ${COREUTIL_DIR}/swizzleSpec.h
    ${COREUTIL_DIR}/tAlignedArray.h
    ${COREUTIL_DIR}/tDictionary.h
    ${COREUTIL_DIR}/tFixedSizeDeque.h
    ${COREUTIL_DIR}/tFixedSizeVector.h
    ${COREUTIL_DIR}/timeClass.h
    ${COREUTIL_DIR}/timeSource.h
    ${COREUTIL_DIR}/tList.h
    ${COREUTIL_DIR}/tNamedFactory.h
    ${COREUTIL_DIR}/tSignal.h
    ${COREUTIL_DIR}/tSingleton.h
    ${COREUTIL_DIR}/tUnmanagedVector.h
    ${COREUTIL_DIR}/tVector.h
    ${COREUTIL_DIR}/tVectorSpecializations.h
    ${COREUTIL_DIR}/uuid.h
)

SET( COREUTIL_SOURCES
    ${COREUTIL_DIR}/byteBuffer.cpp
    ${COREUTIL_DIR}/commonSwizzles.cpp
    ${COREUTIL_DIR}/hashFunction.cpp
    ${COREUTIL_DIR}/md5.cpp
    ${COREUTIL_DIR}/path.cpp
    ${COREUTIL_DIR}/rgb2luv.cpp
    ${COREUTIL_DIR}/rgb2xyz.cpp
    ${COREUTIL_DIR}/str.cpp
    ${COREUTIL_DIR}/tDictionary.cpp
    ${COREUTIL_DIR}/timeClass.cpp
    ${COREUTIL_DIR}/tSignal.cpp
    ${COREUTIL_DIR}/tVector.cpp
    ${COREUTIL_DIR}/uuid.cpp
)

SET( COREUTILJOURNAL_HEADERS
    ${COREUTILJOURNAL_DIR}/journal.h
	${COREUTILJOURNAL_DIR}/journaledSignal.h
	${COREUTILJOURNAL_DIR}/process.h
)

SET( COREUTILJOURNAL_SOURCES
    ${COREUTILJOURNAL_DIR}/journal.cpp
	${COREUTILJOURNAL_DIR}/process.cpp
)

SET( COREUTILJOURNALTEST_SOURCES
    ${COREUTILJOURNALTEST_DIR}/testJournal.cpp
	${COREUTILJOURNALTEST_DIR}/testProcess.cpp
)

SET( COREUTILTEST_SOURCES
    ${COREUTILTEST_DIR}/testFixedSizeDeque.cpp
	${COREUTILTEST_DIR}/testPath.cpp
	${COREUTILTEST_DIR}/testString.cpp
	${COREUTILTEST_DIR}/testVector2.cpp
)

SET( COREUTILTESTZIP_HEADERS
    ${COREUTILZIP_DIR}/centralDir.h
	${COREUTILZIP_DIR}/compressor.h
	${COREUTILZIP_DIR}/crctab.h
	${COREUTILZIP_DIR}/extraField.h
	${COREUTILZIP_DIR}/fileHeader.h
	${COREUTILZIP_DIR}/zipArchive.h
	${COREUTILZIP_DIR}/zipCryptStream.h
	${COREUTILZIP_DIR}/zipObject.h
	${COREUTILZIP_DIR}/zipStatFilter.h
	${COREUTILZIP_DIR}/zipSubStream.h
	${COREUTILZIP_DIR}/zipTempStream.h
	${COREUTILZIP_DIR}/zipVolume.h
)

SET( COREUTILTESTZIP_SOURCES
    ${COREUTILZIP_DIR}/centralDir.cpp
	${COREUTILZIP_DIR}/compressor.cpp
	${COREUTILZIP_DIR}/extraField.cpp
	${COREUTILZIP_DIR}/fileHeader.cpp
	${COREUTILZIP_DIR}/zipArchive.cpp
	${COREUTILZIP_DIR}/zipCryptStream.cpp
	${COREUTILZIP_DIR}/zipObject.cpp
	${COREUTILZIP_DIR}/zipSubStream.cpp
	${COREUTILZIP_DIR}/zipTempStream.cpp
	${COREUTILZIP_DIR}/zipVolume.cpp
)

SET( COREUTILZIPCOMPRESSORS_SOURCES
    ${COREUTILZIPCOMPRESSORS_DIR}/deflate.cpp
	${COREUTILZIPCOMPRESSORS_DIR}/stored.cpp
)

SET( COREUTILTESTZIPUNITTESTS_HEADERS
	${COREUTILZIPUNITTESTS_DIR}/zipTest.h
)

SET( COREUTILTESTZIPUNITTESTS_SOURCES
    ${COREUTILZIPUNITTESTS_DIR}/zipTestMisc.cpp
	${COREUTILZIPUNITTESTS_DIR}/zipTestRead.cpp
	${COREUTILZIPUNITTESTS_DIR}/zipTestWrite.cpp
)
	
SET( ENVIRONMENT_HEADERS
    ${ENVIRONMENT_DIR}/basicClouds.h
	${ENVIRONMENT_DIR}/cloudLayer.h
	${ENVIRONMENT_DIR}/decalRoad.h
	${ENVIRONMENT_DIR}/meshRoad.h
	${ENVIRONMENT_DIR}/nodeListManager.h
	${ENVIRONMENT_DIR}/river.h
	${ENVIRONMENT_DIR}/scatterSky.h
	${ENVIRONMENT_DIR}/skyBox.h
	${ENVIRONMENT_DIR}/sun.h
	${ENVIRONMENT_DIR}/timeOfDay.h
	${ENVIRONMENT_DIR}/waterBlock.h
	${ENVIRONMENT_DIR}/waterObject.h
	${ENVIRONMENT_DIR}/waterPlane.h
)

SET( ENVIRONMENT_SOURCES
    ${ENVIRONMENT_DIR}/basicClouds.cpp
	${ENVIRONMENT_DIR}/cloudLayer.cpp
	${ENVIRONMENT_DIR}/decalRoad.cpp
	${ENVIRONMENT_DIR}/meshRoad.cpp
	${ENVIRONMENT_DIR}/nodeListManager.cpp
	${ENVIRONMENT_DIR}/river.cpp
	${ENVIRONMENT_DIR}/scatterSky.cpp
	${ENVIRONMENT_DIR}/skyBox.cpp
	${ENVIRONMENT_DIR}/sun.cpp
	${ENVIRONMENT_DIR}/timeOfDay.cpp
	${ENVIRONMENT_DIR}/waterBlock.cpp
	${ENVIRONMENT_DIR}/waterObject.cpp
	${ENVIRONMENT_DIR}/waterPlane.cpp
)

SET( ENVIRONMENTEDITOR_HEADERS
    ${ENVIRONMENTEDITORS_DIR}/guiMeshRoadEditorCtrl.h
    ${ENVIRONMENTEDITORS_DIR}/guiRiverEditorCtrl.h
	${ENVIRONMENTEDITORS_DIR}/guiRoadEditorCtrl.h
)

SET( ENVIRONMENTEDITOR_SOURCES
    ${ENVIRONMENTEDITORS_DIR}/guiMeshRoadEditorCtrl.cpp
    ${ENVIRONMENTEDITORS_DIR}/guiRiverEditorCtrl.cpp
	${ENVIRONMENTEDITORS_DIR}/guiRoadEditorCtrl.cpp
)

SET( FOREST_HEADERS
    ${FOREST_DIR}/forest.h
	${FOREST_DIR}/forestCell.h
	${FOREST_DIR}/forestCellBatch.h
	${FOREST_DIR}/forestCollision.h
	${FOREST_DIR}/forestDataFile.h
	${FOREST_DIR}/forestItem.h
	${FOREST_DIR}/forestWindAccumulator.h
	${FOREST_DIR}/forestWindEmitter.h
	${FOREST_DIR}/forestWindMgr.h
	${FOREST_DIR}/windDeformation.h
)

SET( FOREST_SOURCES
    ${FOREST_DIR}/forest.cpp
	${FOREST_DIR}/forestCell.cpp
	${FOREST_DIR}/forestCellBatch.cpp
	${FOREST_DIR}/forestCollision.cpp
	${FOREST_DIR}/forestDataFile.cpp
	${FOREST_DIR}/forestItem.cpp
	${FOREST_DIR}/forestRender.cpp
	${FOREST_DIR}/forestWindAccumulator.cpp
	${FOREST_DIR}/forestWindEmitter.cpp
	${FOREST_DIR}/forestWindMgr.cpp
	${FOREST_DIR}/windDeformation.cpp
)

SET( FORESTEDITOR_HEADERS
    ${FORESTEDITOR_DIR}/forestBrushElement.h
    ${FORESTEDITOR_DIR}/forestBrushTool.h
    ${FORESTEDITOR_DIR}/forestEditorCtrl.h
    ${FORESTEDITOR_DIR}/forestSelectionTool.h
    ${FORESTEDITOR_DIR}/forestTool.h
	${FORESTEDITOR_DIR}/forestUndo.h
)
	
SET( FORESTEDITOR_SOURCES
    ${FORESTEDITOR_DIR}/forestBrushElement.cpp
    ${FORESTEDITOR_DIR}/forestBrushTool.cpp
    ${FORESTEDITOR_DIR}/forestEditorCtrl.cpp
    ${FORESTEDITOR_DIR}/forestSelectionTool.cpp
    ${FORESTEDITOR_DIR}/forestTool.cpp
	${FORESTEDITOR_DIR}/forestUndo.cpp
)

IF( USE_OPENGL )
    SET( FORESTGLSL_HEADERS
        ${FORESTGLSL_DIR}/windDeformationGLSL.h   
    )

    SET( FORESTGLSL_SOURCES
        ${FORESTGLSL_DIR}/windDeformationGLSL.cpp
    )
ENDIF( USE_OPENGL )

SET( FORESTHLSL_HEADERS
    ${FORESTHLSL_DIR}/windDeformationHLSL.h   
)

SET( FORESTHLSL_SOURCES
    ${FORESTHLSL_DIR}/windDeformationHLSL.cpp
)

SET( FORESTTS_HEADERS
    ${FORESTTS_DIR}/tsForestCellBatch.h
	${FORESTTS_DIR}/tsForestItemData.h
)

SET( FORESTTS_SOURCES
    ${FORESTTS_DIR}/tsForestCellBatch.cpp
	${FORESTTS_DIR}/tsForestItemData.cpp
)

SET( GFX_HEADERS
    ${GFX_DIR}/genericConstBuffer.h
    ${GFX_DIR}/gFont.h
    ${GFX_DIR}/gfxAdapter.h
    ${GFX_DIR}/gfxAPI.h
    ${GFX_DIR}/gfxCardProfile.h
    ${GFX_DIR}/gfxCubemap.h
    ${GFX_DIR}/gfxDebugEvent.h	
    ${GFX_DIR}/gfxDevice.h
    ${GFX_DIR}/gfxDeviceStatistics.h
    ${GFX_DIR}/gfxDrawUtil.h
    ${GFX_DIR}/gfxEnums.h
    ${GFX_DIR}/gfxFence.h
    ${GFX_DIR}/gfxFontRenderBatcher.h
    ${GFX_DIR}/gfxFormatUtils.h
    ${GFX_DIR}/gfxInit.h
    ${GFX_DIR}/gfxOcclusionQuery.h
    ${GFX_DIR}/gfxPrimitiveBuffer.h
    ${GFX_DIR}/gfxResource.h
    ${GFX_DIR}/gfxShader.h
    ${GFX_DIR}/gfxStateBlock.h
    ${GFX_DIR}/gfxStringEnumTranslate.h
    ${GFX_DIR}/gfxStructs.h
    ${GFX_DIR}/gfxTarget.h
    ${GFX_DIR}/gfxTextureHandle.h
    ${GFX_DIR}/gfxTextureManager.h
    ${GFX_DIR}/gfxTextureObject.h
    ${GFX_DIR}/gfxTextureProfile.h
    ${GFX_DIR}/gfxTransformSaver.h
    ${GFX_DIR}/gfxVertexBuffer.h
    ${GFX_DIR}/gfxVertexColor.h
    ${GFX_DIR}/gfxVertexFormat.h
    ${GFX_DIR}/gfxVertexTypes.h
    ${GFX_DIR}/primBuilder.h
    ${GFX_DIR}/screenshot.h
)

SET( GFX_SOURCES
    ${GFX_DIR}/genericConstBuffer.cpp
    ${GFX_DIR}/gFont.cpp
    ${GFX_DIR}/gfxAPI.cpp
    ${GFX_DIR}/gfxCardProfile.cpp
    ${GFX_DIR}/gfxCubemap.cpp
    ${GFX_DIR}/gfxDevice.cpp
    ${GFX_DIR}/gfxDeviceStatistics.cpp
    ${GFX_DIR}/gfxDrawUtil.cpp
    ${GFX_DIR}/gfxFence.cpp
    ${GFX_DIR}/gfxFontRenderBatcher.cpp
    ${GFX_DIR}/gfxFormatUtils.cpp
    ${GFX_DIR}/gfxInit.cpp
    ${GFX_DIR}/gfxOcclusionQuery.cpp
    ${GFX_DIR}/gfxPrimitiveBuffer.cpp
    ${GFX_DIR}/gfxResource.cpp
    ${GFX_DIR}/gfxShader.cpp
    ${GFX_DIR}/gfxStateBlock.cpp
	${GFX_DIR}/gfxStringEnumTranslate.cpp
	${GFX_DIR}/gfxStructs.cpp
	${GFX_DIR}/gfxTarget.cpp
	${GFX_DIR}/gfxTextureHandle.cpp
	${GFX_DIR}/gfxTextureManager.cpp
	${GFX_DIR}/gfxTextureObject.cpp
	${GFX_DIR}/gfxTextureProfile.cpp
	${GFX_DIR}/gfxVertexBuffer.cpp
	${GFX_DIR}/gfxVertexColor.cpp
	${GFX_DIR}/gfxVertexFormat.cpp
	${GFX_DIR}/gfxVertexTypes.cpp
	${GFX_DIR}/primBuilder.cpp
	${GFX_DIR}/screenshot.cpp
)

SET( GFXBITMAP_HEADERS
    ${GFXBITMAP_DIR}/bitmapUtils.h
	${GFXBITMAP_DIR}/ddsFile.h
	${GFXBITMAP_DIR}/ddsUtils.h
	${GFXBITMAP_DIR}/gBitmap.h
	${GFXBITMAP_DIR}/pngUtils.h
)

SET( GFXBITMAP_SOURCES
    ${GFXBITMAP_DIR}/bitmapUtils.cpp
	${GFXBITMAP_DIR}/ddsLoader.cpp
	${GFXBITMAP_DIR}/ddsUtils.cpp
	${GFXBITMAP_DIR}/gBitmap.cpp
)

SET( GFXBITMAPLOADER_SOURCES
    ${GFXBITMAPLOADER_DIR}/bitmapBmp.cpp
	${GFXBITMAPLOADER_DIR}/bitmapGif.cpp
	${GFXBITMAPLOADER_DIR}/bitmapJpeg.cpp
	${GFXBITMAPLOADER_DIR}/bitmapMng.cpp
	${GFXBITMAPLOADER_DIR}/bitmapPng.cpp
	${GFXBITMAPLOADER_DIR}/bitmapTga.cpp
)

SET( D3D9_HEADERS
    ${D3D9_DIR}/d3dx9Functions.h
	${D3D9_DIR}/gfxD3D9CardProfiler.h
	${D3D9_DIR}/gfxD3D9Cubemap.h
	${D3D9_DIR}/gfxD3D9Device.h
	${D3D9_DIR}/gfxD3D9EnumTranslate.h
	${D3D9_DIR}/gfxD3D9OcclusionQuery.h
	${D3D9_DIR}/gfxD3D9PrimitiveBuffer.h
	${D3D9_DIR}/gfxD3D9QueryFence.h
	${D3D9_DIR}/gfxD3D9Shader.h
	${D3D9_DIR}/gfxD3D9StateBlock.h
	${D3D9_DIR}/gfxD3D9TextureManager.h
	${D3D9_DIR}/gfxD3D9TextureObject.h
	${D3D9_DIR}/gfxD3D9VertexBuffer.h
	${D3D9_DIR}/screenshotD3D9.h
	${D3D9_DIR}/videoCaptureD3D9.h
)

SET( D3D9_SOURCES
    ${D3D9_DIR}/gfxD3D9CardProfiler.cpp
	${D3D9_DIR}/gfxD3D9Cubemap.cpp
	${D3D9_DIR}/gfxD3D9Device.cpp
	${D3D9_DIR}/gfxD3D9Device.regen-states.cpp
	${D3D9_DIR}/gfxD3D9OcclusionQuery.cpp
	${D3D9_DIR}/gfxD3D9PrimitiveBuffer.cpp
	${D3D9_DIR}/gfxD3D9QueryFence.cpp
	${D3D9_DIR}/gfxD3D9Shader.cpp
	${D3D9_DIR}/gfxD3D9StateBlock.cpp
	${D3D9_DIR}/gfxD3D9TextureManager.cpp
	${D3D9_DIR}/gfxD3D9TextureObject.cpp
	${D3D9_DIR}/gfxD3D9VertexBuffer.cpp
	${D3D9_DIR}/screenshotD3D9.cpp
	${D3D9_DIR}/videoCaptureD3D9.cpp
)

SET( D3D9PC_HEADERS
	${D3D9PC_DIR}/gfxPCD3D9Device.h
	${D3D9PC_DIR}/gfxPCD3D9Target.h
)

SET( D3D9PC_SOURCES
	${D3D9PC_DIR}/gfxD3D9Device.pc.cpp
	${D3D9PC_DIR}/gfxD3D9EnumTranslate.pc.cpp
	${D3D9PC_DIR}/gfxD3D9PrimitiveBuffer.pc.cpp
	${D3D9PC_DIR}/gfxPCD3D9Device.cpp
	${D3D9PC_DIR}/gfxPCD3D9Target.cpp
)

IF( USE_OPENGL )
    SET( GL_HEADERS
        ${GL_DIR}/gfxGLAppleFence.h
    	${GL_DIR}/gfxGLCardProfiler.h
    	${GL_DIR}/gfxGLCubemap.h
    	${GL_DIR}/gfxGLDevice.h
    	${GL_DIR}/gfxGLEnumTranslate.h
    	${GL_DIR}/gfxGLOcclusionQuery.h
    	${GL_DIR}/gfxGLPrimitiveBuffer.h
    	${GL_DIR}/gfxGLShader.h
    	${GL_DIR}/gfxGLStateBlock.h
    	${GL_DIR}/gfxGLTextureManager.h
    	${GL_DIR}/gfxGLTextureObject.h
    	${GL_DIR}/gfxGLTextureTarget.h
    	${GL_DIR}/gfxGLUtils.h
    	${GL_DIR}/gfxGLVertexBuffer.h
    	${GL_DIR}/gfxGLWindowTarget.h
    )

    SET( GL_SOURCES
        ${GL_DIR}/gfxGLAppleFence.cpp
    	${GL_DIR}/gfxGLCardProfiler.cpp
    	${GL_DIR}/gfxGLCubemap.cpp
    	${GL_DIR}/gfxGLDevice.cpp
    	${GL_DIR}/gfxGLDevice.win.cpp
    	${GL_DIR}/gfxGLEnumTranslate.cpp
    	${GL_DIR}/gfxGLOcclusionQuery.cpp
    	${GL_DIR}/gfxGLPrimitiveBuffer.cpp
    	${GL_DIR}/gfxGLShader.cpp
    	${GL_DIR}/gfxGLStateBlock.cpp
    	${GL_DIR}/gfxGLTextureManager.cpp
    	${GL_DIR}/gfxGLTextureObject.cpp
    	${GL_DIR}/gfxGLTextureTarget.cpp
    	${GL_DIR}/gfxGLVertexBuffer.cpp
    	${GL_DIR}/gfxGLWindowTarget.cpp
    )
    
	SET( GLGGL_HEADERS
	    ${GLGGL_DIR}/ggl.h
		${GLGGL_DIR}/gglConfig.h
	)

	SET( GLGGL_SOURCES
	    ${GLGGL_DIR}/ggl.cpp
	)
	
	SET( GLGGLWIN32_HEADERS
	    ${GLGGLWIN32_DIR}/wgl.h
	)
	
	SET( GLGGLWIN32_SOURCES
	    ${GLGGLWIN32_DIR}/wglBind.cpp
	)
	
ENDIF( USE_OPENGL)

SET( GFXNULL_HEADERS
    ${NULL_DIR}/gfxNullDevice.h
)

SET( GFXNULL_SOURCES
    ${NULL_DIR}/gfxNullDevice.cpp
)

SET( GFXSIM_HEADERS
    ${GFXSIM_DIR}/cubemapData.h
	${GFXSIM_DIR}/debugDraw.h
	${GFXSIM_DIR}/gfxStateBlockData.h
)

SET( GFXSIM_SOURCES
    ${GFXSIM_DIR}/cubemapData.cpp
	${GFXSIM_DIR}/debugDraw.cpp
	${GFXSIM_DIR}/gfxStateBlockData.cpp
)

SET( GFXTEST_SOURCES
    ${GFXTEST_DIR}/stanfordBunny.cpp
	${GFXTEST_DIR}/testGfx.cpp
)

SET( GFXUTIL_HEADERS
    ${GFXUTIL_DIR}/distanceField.h
	${GFXUTIL_DIR}/gfxFrustumSaver.h
	${GFXUTIL_DIR}/screenspace.h
	${GFXUTIL_DIR}/triListOpt.h
)

SET( GFXUTIL_SOURCES
    ${GFXUTIL_DIR}/distanceField.cpp
	${GFXUTIL_DIR}/gfxFrustumSaver.cpp
	${GFXUTIL_DIR}/screenspace.cpp
	${GFXUTIL_DIR}/triListOpt.cpp
)

SET( GFXVIDEO_HEADERS
    ${GFXVIDEO_DIR}/theoraTexture.h
	${GFXVIDEO_DIR}/videoCapture.h
)

SET( GFXVIDEO_SOURCES
    ${GFXVIDEO_DIR}/theoraTexture.cpp
	${GFXVIDEO_DIR}/videoCapture.cpp
	${GFXVIDEO_DIR}/videoEncoderPNG.cpp
	${GFXVIDEO_DIR}/videoEncoderTheora.cpp
)

SET( GUI3D_HEADERS
    ${GUI3D_DIR}/guiTSControl.h
)

SET( GUI3D_SOURCES
    ${GUI3D_DIR}/guiTSControl.cpp
)

SET( GUIBUTTONS_HEADERS
    ${GUIBUTTONS_DIR}/guiBitmapButtonCtrl.h
	${GUIBUTTONS_DIR}/guiButtonBaseCtrl.h
	${GUIBUTTONS_DIR}/guiButtonCtrl.h
	${GUIBUTTONS_DIR}/guiCheckBoxCtrl.h
	${GUIBUTTONS_DIR}/guiIconButtonCtrl.h
	${GUIBUTTONS_DIR}/guiRadioCtrl.h
	${GUIBUTTONS_DIR}/guiSwatchButtonCtrl.h
	${GUIBUTTONS_DIR}/guiToggleButtonCtrl.h
	${GUIBUTTONS_DIR}/guiToolboxButtonCtrl.h
)

SET( GUIBUTTONS_SOURCES
    ${GUIBUTTONS_DIR}/guiBitmapButtonCtrl.cpp
	${GUIBUTTONS_DIR}/guiBorderButton.cpp
	${GUIBUTTONS_DIR}/guiButtonBaseCtrl.cpp
	${GUIBUTTONS_DIR}/guiButtonCtrl.cpp
	${GUIBUTTONS_DIR}/guiCheckBoxCtrl.cpp
	${GUIBUTTONS_DIR}/guiIconButtonCtrl.cpp
	${GUIBUTTONS_DIR}/guiRadioCtrl.cpp
	${GUIBUTTONS_DIR}/guiSwatchButtonCtrl.cpp
	${GUIBUTTONS_DIR}/guiToggleButtonCtrl.cpp
	${GUIBUTTONS_DIR}/guiToolboxButtonCtrl.cpp
)

SET( GUICONTAINERS_HEADERS
    ${GUICONTAINERS_DIR}/guiAutoScrollCtrl.h
	${GUICONTAINERS_DIR}/guiContainer.h
	${GUICONTAINERS_DIR}/guiCtrlArrayCtrl.h
	${GUICONTAINERS_DIR}/guiDragAndDropCtrl.h
	${GUICONTAINERS_DIR}/guiDynamicCtrlArrayCtrl.h
	${GUICONTAINERS_DIR}/guiFormCtrl.h
	${GUICONTAINERS_DIR}/guiFrameCtrl.h
	${GUICONTAINERS_DIR}/guiPaneCtrl.h
	${GUICONTAINERS_DIR}/guiPanel.h
	${GUICONTAINERS_DIR}/guiRolloutCtrl.h
	${GUICONTAINERS_DIR}/guiScrollCtrl.h
	${GUICONTAINERS_DIR}/guiSplitContainer.h
	${GUICONTAINERS_DIR}/guiStackCtrl.h
	${GUICONTAINERS_DIR}/guiTabBookCtrl.h
	${GUICONTAINERS_DIR}/guiWindowCollapseCtrl.h
	${GUICONTAINERS_DIR}/guiWindowCtrl.h
)

SET( GUICONTAINERS_SOURCES
    ${GUICONTAINERS_DIR}/guiAutoScrollCtrl.cpp
	${GUICONTAINERS_DIR}/guiContainer.cpp
	${GUICONTAINERS_DIR}/guiCtrlArrayCtrl.cpp
	${GUICONTAINERS_DIR}/guiDragAndDropCtrl.cpp
	${GUICONTAINERS_DIR}/guiDynamicCtrlArrayCtrl.cpp
	${GUICONTAINERS_DIR}/guiFormCtrl.cpp
	${GUICONTAINERS_DIR}/guiFrameCtrl.cpp
	${GUICONTAINERS_DIR}/guiPaneCtrl.cpp
	${GUICONTAINERS_DIR}/guiPanel.cpp
	${GUICONTAINERS_DIR}/guiRolloutCtrl.cpp
	${GUICONTAINERS_DIR}/guiScrollCtrl.cpp
	${GUICONTAINERS_DIR}/guiSplitContainer.cpp
	${GUICONTAINERS_DIR}/guiStackCtrl.cpp
	${GUICONTAINERS_DIR}/guiTabBookCtrl.cpp
	${GUICONTAINERS_DIR}/guiWindowCollapseCtrl.cpp
	${GUICONTAINERS_DIR}/guiWindowCtrl.cpp
)

SET( GUICONTROLS_HEADERS
    ${GUICONTROLS_DIR}/guiBackgroundCtrl.h
	${GUICONTROLS_DIR}/guiBitmapCtrl.h
	${GUICONTROLS_DIR}/guiColorPicker.h
	${GUICONTROLS_DIR}/guiConsole.h
	${GUICONTROLS_DIR}/guiConsoleEditCtrl.h
	${GUICONTROLS_DIR}/guiConsoleTextCtrl.h
	${GUICONTROLS_DIR}/guiDecoyCtrl.h
	${GUICONTROLS_DIR}/guiDirectoryFileListCtrl.h
	${GUICONTROLS_DIR}/guiFileTreeCtrl.h
	${GUICONTROLS_DIR}/guiGameListMenuCtrl.h
	${GUICONTROLS_DIR}/guiGameListOptionsCtrl.h
	${GUICONTROLS_DIR}/guiGradientCtrl.h
	${GUICONTROLS_DIR}/guiListBoxCtrl.h
	${GUICONTROLS_DIR}/guiMaterialCtrl.h
	${GUICONTROLS_DIR}/guiMLTextCtrl.h
	${GUICONTROLS_DIR}/guiMLTextEditCtrl.h
	${GUICONTROLS_DIR}/guiPopUpCtrl.h
	${GUICONTROLS_DIR}/guiPopUpCtrlEx.h
	${GUICONTROLS_DIR}/guiSliderCtrl.h
	${GUICONTROLS_DIR}/guiTabPageCtrl.h
	${GUICONTROLS_DIR}/guiTextCtrl.h
	${GUICONTROLS_DIR}/guiTextEditCtrl.h
	${GUICONTROLS_DIR}/guiTextEditSliderBitmapCtrl.h
	${GUICONTROLS_DIR}/guiTextEditSliderCtrl.h
	${GUICONTROLS_DIR}/guiTextListCtrl.h
	${GUICONTROLS_DIR}/guiTreeViewCtrl.h
)

SET( GUICONTROLS_SOURCES
    ${GUICONTROLS_DIR}/guiBackgroundCtrl.cpp
	${GUICONTROLS_DIR}/guiBitmapBorderCtrl.cpp
	${GUICONTROLS_DIR}/guiBitmapCtrl.cpp
	${GUICONTROLS_DIR}/guiColorPicker.cpp
	${GUICONTROLS_DIR}/guiConsole.cpp
	${GUICONTROLS_DIR}/guiConsoleEditCtrl.cpp
	${GUICONTROLS_DIR}/guiConsoleTextCtrl.cpp
	${GUICONTROLS_DIR}/guiDecoyCtrl.cpp
	${GUICONTROLS_DIR}/guiDirectoryFileListCtrl.cpp
	${GUICONTROLS_DIR}/guiFileTreeCtrl.cpp
	${GUICONTROLS_DIR}/guiGameListMenuCtrl.cpp
	${GUICONTROLS_DIR}/guiGameListOptionsCtrl.cpp
	${GUICONTROLS_DIR}/guiGradientCtrl.cpp
	${GUICONTROLS_DIR}/guiListBoxCtrl.cpp
	${GUICONTROLS_DIR}/guiMaterialCtrl.cpp
	${GUICONTROLS_DIR}/guiMLTextCtrl.cpp
	${GUICONTROLS_DIR}/guiMLTextEditCtrl.cpp
	${GUICONTROLS_DIR}/guiPopUpCtrl.cpp
	${GUICONTROLS_DIR}/guiPopUpCtrlEx.cpp
	${GUICONTROLS_DIR}/guiSliderCtrl.cpp
	${GUICONTROLS_DIR}/guiTabPageCtrl.cpp
	${GUICONTROLS_DIR}/guiTextCtrl.cpp
	${GUICONTROLS_DIR}/guiTextEditCtrl.cpp
	${GUICONTROLS_DIR}/guiTextEditSliderBitmapCtrl.cpp
	${GUICONTROLS_DIR}/guiTextEditSliderCtrl.cpp
	${GUICONTROLS_DIR}/guiTextListCtrl.cpp
	${GUICONTROLS_DIR}/guiTreeViewCtrl.cpp
)

SET( GUICORE_HEADERS
    ${GUICORE_DIR}/guiArrayCtrl.h
	${GUICORE_DIR}/guiCanvas.h
	${GUICORE_DIR}/guiControl.h
	${GUICORE_DIR}/guiDefaultControlRender.h
	${GUICORE_DIR}/guiScriptNotifyControl.h
	${GUICORE_DIR}/guiTypes.h
)

SET( GUICORE_SOURCES
    ${GUICORE_DIR}/guiArrayCtrl.cpp
	${GUICORE_DIR}/guiCanvas.cpp
	${GUICORE_DIR}/guiControl.cpp
	${GUICORE_DIR}/guiDefaultControlRender.cpp
	${GUICORE_DIR}/guiScriptNotifyControl.cpp
	${GUICORE_DIR}/guiTypes.cpp
)

SET( GUIEDITOR_HEADERS
    ${GUIEDITOR_DIR}/editorFunctions.h
	${GUIEDITOR_DIR}/guiDebugger.h
	${GUIEDITOR_DIR}/guiEaseViewCtrl.h
	${GUIEDITOR_DIR}/guiEditCtrl.h
	${GUIEDITOR_DIR}/guiFilterCtrl.h
	${GUIEDITOR_DIR}/guiGraphCtrl.h
	${GUIEDITOR_DIR}/guiImageList.h
	${GUIEDITOR_DIR}/guiInspector.h
	${GUIEDITOR_DIR}/guiInspectorTypes.h
	${GUIEDITOR_DIR}/guiMenuBar.h
	${GUIEDITOR_DIR}/guiParticleGraphCtrl.h
	${GUIEDITOR_DIR}/guiRectHandles.h
	${GUIEDITOR_DIR}/guiSeparatorCtrl.h
	${GUIEDITOR_DIR}/guiShapeEdPreview.h
)

SET( GUIEDITOR_SOURCES
    ${GUIEDITOR_DIR}/editorFunctions.cpp
	${GUIEDITOR_DIR}/guiDebugger.cpp
	${GUIEDITOR_DIR}/guiEaseViewCtrl.cpp
	${GUIEDITOR_DIR}/guiEditCtrl.cpp
	${GUIEDITOR_DIR}/guiFilterCtrl.cpp
	${GUIEDITOR_DIR}/guiGraphCtrl.cpp
	${GUIEDITOR_DIR}/guiImageList.cpp
	${GUIEDITOR_DIR}/guiInspector.cpp
	${GUIEDITOR_DIR}/guiInspectorTypes.cpp
	${GUIEDITOR_DIR}/guiMenuBar.cpp
	${GUIEDITOR_DIR}/guiParticleGraphCtrl.cpp
	${GUIEDITOR_DIR}/guiRectHandles.cpp
	${GUIEDITOR_DIR}/guiSeparatorCtrl.cpp
	${GUIEDITOR_DIR}/guiShapeEdPreview.cpp
)

SET( GUIEDITORINSPECTOR_HEADERS
    ${GUIEDITORINSPECTOR_DIR}/customField.h
	${GUIEDITORINSPECTOR_DIR}/datablockField.h
	${GUIEDITORINSPECTOR_DIR}/dynamicField.h
	${GUIEDITORINSPECTOR_DIR}/dynamicGroup.h
	${GUIEDITORINSPECTOR_DIR}/field.h
	${GUIEDITORINSPECTOR_DIR}/group.h
	${GUIEDITORINSPECTOR_DIR}/variableField.h
	${GUIEDITORINSPECTOR_DIR}/variableGroup.h
	${GUIEDITORINSPECTOR_DIR}/variableInspector.h
)

SET( GUIEDITORINSPECTOR_SOURCES
    ${GUIEDITORINSPECTOR_DIR}/customField.cpp
	${GUIEDITORINSPECTOR_DIR}/datablockField.cpp
	${GUIEDITORINSPECTOR_DIR}/dynamicField.cpp
	${GUIEDITORINSPECTOR_DIR}/dynamicGroup.cpp
	${GUIEDITORINSPECTOR_DIR}/field.cpp
	${GUIEDITORINSPECTOR_DIR}/group.cpp
	${GUIEDITORINSPECTOR_DIR}/variableField.cpp
	${GUIEDITORINSPECTOR_DIR}/variableGroup.cpp
	${GUIEDITORINSPECTOR_DIR}/variableInspector.cpp
)

SET( GUIGAME_HEADERS
    ${GUIGAME_DIR}/guiFadeinBitmapCtrl.h
	${GUIGAME_DIR}/guiMessageVectorCtrl.h
	${GUIGAME_DIR}/guiProgressBitmapCtrl.h
	${GUIGAME_DIR}/guiProgressCtrl.h
)

SET( GUIGAME_SOURCES
    ${GUIGAME_DIR}/guiChunkedBitmapCtrl.cpp
	${GUIGAME_DIR}/guiFadeinBitmapCtrl.cpp
	${GUIGAME_DIR}/guiIdleCamFadeBitmapCtrl.cpp
	${GUIGAME_DIR}/guiMessageVectorCtrl.cpp
	${GUIGAME_DIR}/guiProgressBitmapCtrl.cpp
	${GUIGAME_DIR}/guiProgressCtrl.cpp
)

SET( GUISHINY_HEADERS
    ${GUISHINY_DIR}/guiTickCtrl.h
)

SET( GUISHINY_SOURCES
    ${GUISHINY_DIR}/guiTickCtrl.cpp
)

SET( GUITHEORA_HEADERS
    ${GUITHEORA_DIR}/guiTheoraCtrl.h
)

SET( GUITHEORA_SOURCES
    ${GUITHEORA_DIR}/guiTheoraCtrl.cpp
)

SET( GUIUTILITY_HEADERS
    ${GUIUTILITY_DIR}/guiBubbleTextCtrl.h
	${GUIUTILITY_DIR}/guiInputCtrl.h
	${GUIUTILITY_DIR}/guiMouseEventCtrl.h
	${GUIUTILITY_DIR}/messageVector.h
)

SET( GUIUTILITY_SOURCES
    ${GUIUTILITY_DIR}/guiBubbleTextCtrl.cpp
	${GUIUTILITY_DIR}/guiInputCtrl.cpp
	${GUIUTILITY_DIR}/guiMouseEventCtrl.cpp
	${GUIUTILITY_DIR}/messageVector.cpp
)

SET( GUIWORLDEDITOR_HEADERS
    ${GUIWORDLEDITOR_DIR}/creator.h
	${GUIWORDLEDITOR_DIR}/editor.h
	${GUIWORDLEDITOR_DIR}/editorIconRegistry.h
	${GUIWORDLEDITOR_DIR}/editTSCtrl.h
	${GUIWORDLEDITOR_DIR}/gizmo.h
	${GUIWORDLEDITOR_DIR}/guiConvexShapeEditorCtrl.h
	${GUIWORDLEDITOR_DIR}/guiDecalEditorCtrl.h
	${GUIWORDLEDITOR_DIR}/guiMissionArea.h
	${GUIWORDLEDITOR_DIR}/guiMissionAreaEditor.h
	${GUIWORDLEDITOR_DIR}/guiTerrPreviewCtrl.h
	${GUIWORDLEDITOR_DIR}/terrainActions.h
	${GUIWORDLEDITOR_DIR}/terrainEditor.h
	${GUIWORDLEDITOR_DIR}/tSelection.h
	${GUIWORDLEDITOR_DIR}/undoActions.h
	${GUIWORDLEDITOR_DIR}/worldEditor.h
	${GUIWORDLEDITOR_DIR}/worldEditorSelection.h
)

SET( GUIWORLDEDITOR_SOURCES
    ${GUIWORDLEDITOR_DIR}/creator.cpp
	${GUIWORDLEDITOR_DIR}/editor.cpp
	${GUIWORDLEDITOR_DIR}/editorIconRegistry.cpp
	${GUIWORDLEDITOR_DIR}/editTSCtrl.cpp
	${GUIWORDLEDITOR_DIR}/gizmo.cpp
	${GUIWORDLEDITOR_DIR}/guiConvexShapeEditorCtrl.cpp
	${GUIWORDLEDITOR_DIR}/guiDecalEditorCtrl.cpp
	${GUIWORDLEDITOR_DIR}/guiMissionArea.cpp
	${GUIWORDLEDITOR_DIR}/guiMissionAreaEditor.cpp
	${GUIWORDLEDITOR_DIR}/guiTerrPreviewCtrl.cpp
	${GUIWORDLEDITOR_DIR}/terrainActions.cpp
	${GUIWORDLEDITOR_DIR}/terrainEditor.cpp
	${GUIWORDLEDITOR_DIR}/tSelection.cpp
	${GUIWORDLEDITOR_DIR}/undoActions.cpp
	${GUIWORDLEDITOR_DIR}/worldEditor.cpp
	${GUIWORDLEDITOR_DIR}/worldEditorSelection.cpp
)

SET( I18N_HEADERS
    ${I18N_DIR}/i18n.h
	${I18N_DIR}/lang.h
)

SET( I18N_SOURCES
    ${I18N_DIR}/i18n.cpp
	${I18N_DIR}/lang.cpp
)

SET( LIGHTING_HEADERS
    ${LIGHTING_DIR}/lightInfo.h
	${LIGHTING_DIR}/lightingInterfaces.h
	${LIGHTING_DIR}/lightManager.h
	${LIGHTING_DIR}/lightQuery.h
	${LIGHTING_DIR}/shadowManager.h
)

SET( LIGHTING_SOURCES
    ${LIGHTING_DIR}/lightInfo.cpp
	${LIGHTING_DIR}/lightingInterfaces.cpp
	${LIGHTING_DIR}/lightManager.cpp
	${LIGHTING_DIR}/lightQuery.cpp
	${LIGHTING_DIR}/shadowManager.cpp
)

SET( LIGHTINGADVANCED_HEADERS
    ${LIGHTINGADVANCED_DIR}/advancedLightBinManager.h
	${LIGHTINGADVANCED_DIR}/advancedLightBufferConditioner.h
	${LIGHTINGADVANCED_DIR}/advancedLightingFeatures.h
	${LIGHTINGADVANCED_DIR}/advancedLightManager.h
)

SET( LIGHTINGADVANCED_SOURCES
    ${LIGHTINGADVANCED_DIR}/advancedLightBinManager.cpp
	${LIGHTINGADVANCED_DIR}/advancedLightBufferConditioner.cpp
	${LIGHTINGADVANCED_DIR}/advancedLightingFeatures.cpp
	${LIGHTINGADVANCED_DIR}/advancedLightManager.cpp
)

IF( USE_OPENGL )
    SET( LIGHTINGADVANCEDGLSL_HEADERS
        ${LIGHTINGADVANCEDGLSL_DIR}/advancedLightingFeaturesGLSL.h
		${LIGHTINGADVANCEDGLSL_DIR}/gBufferConditionerGLSL.h
    )

    SET( LIGHTINGADVANCEDGLSL_SOURCES
        ${LIGHTINGADVANCEDGLSL_DIR}/advancedLightingFeaturesGLSL.cpp
		${LIGHTINGADVANCEDGLSL_DIR}/gBufferConditionerGLSL.cpp
    )
ENDIF( USE_OPENGL)

SET( LIGHTINGADVANCEDHLSL_HEADERS
    ${LIGHTINGADVANCEDHLSL_DIR}/advancedLightingFeaturesHLSL.h
	${LIGHTINGADVANCEDHLSL_DIR}/advancedLightingFeaturesHLSL.h
)

SET( LIGHTINGADVANCEDHLSL_SOURCES
    ${LIGHTINGADVANCEDHLSL_DIR}/advancedLightingFeaturesHLSL.cpp
	${LIGHTINGADVANCEDHLSL_DIR}/gBufferConditionerHLSL.cpp
)

SET( LIGHTINGBASIC_HEADERS
    ${LIGHTINGBASIC_DIR}/basicLightManager.h
	${LIGHTINGBASIC_DIR}/basicSceneObjectLightingPlugin.h
	${LIGHTINGBASIC_DIR}/blTerrainSystem.h
)

SET( LIGHTINGBASIC_SOURCES
    ${LIGHTINGBASIC_DIR}/basicLightManager.cpp
	${LIGHTINGBASIC_DIR}/basicSceneObjectLightingPlugin.cpp
	${LIGHTINGBASIC_DIR}/blTerrainSystem.cpp
)

SET( LIGHTINGCOMMON_HEADERS
	${LIGHTINGCOMMON_DIR}/blobShadow.h
	${LIGHTINGCOMMON_DIR}/lightMapParams.h
	${LIGHTINGCOMMON_DIR}/projectedShadow.h
	${LIGHTINGCOMMON_DIR}/sceneLighting.h
	${LIGHTINGCOMMON_DIR}/sceneLightingGlobals.h
	${LIGHTINGCOMMON_DIR}/scenePersist.h
	${LIGHTINGCOMMON_DIR}/shadowBase.h
	${LIGHTINGCOMMON_DIR}/shadowVolumeBSP.h
)

SET( LIGHTINGCOMMON_SOURCES
    ${LIGHTINGCOMMON_DIR}/blobShadow.cpp
	${LIGHTINGCOMMON_DIR}/lightMapParams.cpp
	${LIGHTINGCOMMON_DIR}/projectedShadow.cpp
	${LIGHTINGCOMMON_DIR}/sceneLighting.cpp
	${LIGHTINGCOMMON_DIR}/scenePersist.cpp
	${LIGHTINGCOMMON_DIR}/shadowVolumeBSP.cpp
)

SET( LIGHTINGSHADOWMAP_HEADERS
    ${LIGHTINGSHADOWMAP_DIR}/cubeLightShadowMap.h
	${LIGHTINGSHADOWMAP_DIR}/dualParaboloidLightShadowMap.h
	${LIGHTINGSHADOWMAP_DIR}/lightShadowMap.h
	${LIGHTINGSHADOWMAP_DIR}/paraboloidLightShadowMap.h
	${LIGHTINGSHADOWMAP_DIR}/pssmLightShadowMap.h
	${LIGHTINGSHADOWMAP_DIR}/shadowCommon.h
	${LIGHTINGSHADOWMAP_DIR}/shadowMapManager.h
	${LIGHTINGSHADOWMAP_DIR}/shadowMapPass.h
	${LIGHTINGSHADOWMAP_DIR}/shadowMatHook.h
	${LIGHTINGSHADOWMAP_DIR}/singleLightShadowMap.h
)

SET( LIGHTINGSHADOWMAP_SOURCES
    ${LIGHTINGSHADOWMAP_DIR}/cubeLightShadowMap.cpp
	${LIGHTINGSHADOWMAP_DIR}/dualParaboloidLightShadowMap.cpp
	${LIGHTINGSHADOWMAP_DIR}/lightShadowMap.cpp
	${LIGHTINGSHADOWMAP_DIR}/paraboloidLightShadowMap.cpp
	${LIGHTINGSHADOWMAP_DIR}/pssmLightShadowMap.cpp
	${LIGHTINGSHADOWMAP_DIR}/shadowMapManager.cpp
	${LIGHTINGSHADOWMAP_DIR}/shadowMapPass.cpp
	${LIGHTINGSHADOWMAP_DIR}/shadowMatHook.cpp
	${LIGHTINGSHADOWMAP_DIR}/singleLightShadowMap.cpp
)

SET( MATERIALS_HEADERS
    ${MATERIALS_DIR}/baseMaterialDefinition.h
	${MATERIALS_DIR}/baseMatInstance.h
	${MATERIALS_DIR}/customMaterialDefinition.h
	${MATERIALS_DIR}/materialDefinition.h
	${MATERIALS_DIR}/materialFeatureData.h
	${MATERIALS_DIR}/materialFeatureTypes.h
	${MATERIALS_DIR}/materialList.h
	${MATERIALS_DIR}/materialManager.h
	${MATERIALS_DIR}/materialParameters.h
	${MATERIALS_DIR}/matInstance.h
	${MATERIALS_DIR}/matInstanceHook.h
	${MATERIALS_DIR}/matStateHint.h
	${MATERIALS_DIR}/matTextureTarget.h
	${MATERIALS_DIR}/miscShdrDat.h
	${MATERIALS_DIR}/processedCustomMaterial.h
	${MATERIALS_DIR}/processedFFMaterial.h
	${MATERIALS_DIR}/processedMaterial.h
	${MATERIALS_DIR}/processedShaderMaterial.h
	${MATERIALS_DIR}/sceneData.h
	${MATERIALS_DIR}/shaderData.h
	${MATERIALS_DIR}/shaderMaterialParameters.h
)

SET( MATERIALS_SOURCES
    ${MATERIALS_DIR}/baseMatInstance.cpp
	${MATERIALS_DIR}/customMaterialDefinition.cpp
	${MATERIALS_DIR}/materialDefinition.cpp
	${MATERIALS_DIR}/materialFeatureData.cpp
	${MATERIALS_DIR}/materialFeatureTypes.cpp
	${MATERIALS_DIR}/materialList.cpp
	${MATERIALS_DIR}/materialManager.cpp
	${MATERIALS_DIR}/matInstance.cpp
	${MATERIALS_DIR}/matInstanceHook.cpp
	${MATERIALS_DIR}/matStateHint.cpp
	${MATERIALS_DIR}/matTextureTarget.cpp
	${MATERIALS_DIR}/processedCustomMaterial.cpp
	${MATERIALS_DIR}/processedFFMaterial.cpp
	${MATERIALS_DIR}/processedMaterial.cpp
	${MATERIALS_DIR}/processedShaderMaterial.cpp
	${MATERIALS_DIR}/shaderData.cpp
	${MATERIALS_DIR}/shaderMaterialParameters.cpp
)

SET( MATH_HEADERS
    ${MATH_DIR}/mAngAxis.h
	${MATH_DIR}/mathIO.h
	${MATH_DIR}/mathTypes.h
	${MATH_DIR}/mathUtils.h
	${MATH_DIR}/mBox.h
	${MATH_DIR}/mBoxBase.h
	${MATH_DIR}/mConstants.h
	${MATH_DIR}/mEase.h
	${MATH_DIR}/mIntersector.h
	${MATH_DIR}/mMath.h
	${MATH_DIR}/mMathFn.h
	${MATH_DIR}/mMatrix.h
	${MATH_DIR}/mOrientedBox.h
	${MATH_DIR}/mPlane.h
	${MATH_DIR}/mPlaneSet.h
	${MATH_DIR}/mPlaneTransformer.h
	${MATH_DIR}/mPoint2.h
	${MATH_DIR}/mPoint3.h
	${MATH_DIR}/mPoint4.h
	${MATH_DIR}/mPolyhedron.h
	${MATH_DIR}/mPolyhedron.impl.h
	${MATH_DIR}/mQuadPatch.h
	${MATH_DIR}/mQuat.h
	${MATH_DIR}/mRandom.h
	${MATH_DIR}/mRandomDeck.h
	${MATH_DIR}/mRandomSet.h
	${MATH_DIR}/mRect.h
	${MATH_DIR}/mSilhouetteExtractor.h
	${MATH_DIR}/mSphere.h
	${MATH_DIR}/mSplinePatch.h
	${MATH_DIR}/mTransform.h
)

SET( MATH_SOURCES
    ${MATH_DIR}/mAngAxis.cpp
	${MATH_DIR}/mathTypes.cpp
	${MATH_DIR}/mathUtils.cpp
	${MATH_DIR}/mBox.cpp
	${MATH_DIR}/mConsoleFunctions.cpp
	${MATH_DIR}/mEase.cpp
	${MATH_DIR}/mMath_C.cpp
	${MATH_DIR}/mMathAltivec.cpp
	${MATH_DIR}/mMathAMD.cpp
	${MATH_DIR}/mMathSSE.cpp
	${MATH_DIR}/mMatrix.cpp
	${MATH_DIR}/mOrientedBox.cpp
	${MATH_DIR}/mPlane.cpp
	${MATH_DIR}/mPlaneTransformer.cpp
	${MATH_DIR}/mPoint.cpp
	${MATH_DIR}/mPolyhedron.cpp
	${MATH_DIR}/mQuadPatch.cpp
	${MATH_DIR}/mQuat.cpp
	${MATH_DIR}/mRandom.cpp
	${MATH_DIR}/mRect.cpp
	${MATH_DIR}/mSolver.cpp
	${MATH_DIR}/mSphere.cpp
	${MATH_DIR}/mSplinePatch.cpp
	mMath_ASM.obj
	mMathAMD_ASM.obj
	mMathSSE_ASM.obj
)

SET( MATHTEST_SOURCES
    ${MATHTEST_DIR}/testMathPlane.cpp
	${MATHTEST_DIR}/testPolyhedron.cpp
)

SET( MATHUTIL_HEADERS
    ${MATHUTIL_DIR}/frustum.h
	${MATHUTIL_DIR}/matrixSet.h
	${MATHUTIL_DIR}/matrixSetDelegateMethods.h
	${MATHUTIL_DIR}/quadTransforms.h
	${MATHUTIL_DIR}/sphereMesh.h
	${MATHUTIL_DIR}/tResponseCurve.h
)

SET( MATHUTIL_SOURCES
    ${MATHUTIL_DIR}/frustum.cpp
	${MATHUTIL_DIR}/matrixSet.cpp
	${MATHUTIL_DIR}/quadTransforms.cpp
	${MATHUTIL_DIR}/sphereMesh.cpp
	${MATHUTIL_DIR}/tResponseCurve.cpp
)

IF( USE_RECASTNAVITATION )
	SET( RECASTNAVITATION_SOURCES
    	${NAVIGATION_DIR}/duDebugDrawTorque.cpp
		${NAVIGATION_DIR}/navMesh.cpp
		${NAVIGATION_DIR}/navPath.cpp
		${NAVIGATION_DIR}/recastPolyList.cpp
	)

	SET( RECASTNAVITATION_HEADERS
    	${NAVIGATION_DIR}/duDebugDrawTorque.h
		${NAVIGATION_DIR}/navMesh.h
		${NAVIGATION_DIR}/navPath.h
		${NAVIGATION_DIR}/recastPolyList.h
		${NAVIGATION_DIR}/torqueRecast.h
    )
ENDIF( USE_RECASTNAVITATION )

SET( PLATFORM_HEADERS
    ${PLATFORM_DIR}/platform.h
	${PLATFORM_DIR}/platformAssert.h
	${PLATFORM_DIR}/platformCPUCount.h
	${PLATFORM_DIR}/platformDlibrary.h
	${PLATFORM_DIR}/platformFont.h
	${PLATFORM_DIR}/platformInput.h
	${PLATFORM_DIR}/platformIntrinsics.h
	${PLATFORM_DIR}/platformIntrinsics.gcc.h
	${PLATFORM_DIR}/platformIntrinsics.visualc.h
	${PLATFORM_DIR}/platformMemory.h
	${PLATFORM_DIR}/platformNet.h
	${PLATFORM_DIR}/platformNetAsync.h
	${PLATFORM_DIR}/platformRedBook.h
	${PLATFORM_DIR}/platformTimer.h
	${PLATFORM_DIR}/platformTLS.h
	${PLATFORM_DIR}/platformVFS.h
	${PLATFORM_DIR}/platformVideoInfo.h
	${PLATFORM_DIR}/platformVolume.h
	${PLATFORM_DIR}/profiler.h
	${PLATFORM_DIR}/tmm_off.h
	${PLATFORM_DIR}/tmm_on.h
	${PLATFORM_DIR}/types.h
	${PLATFORM_DIR}/types.codewarrior.h
	${PLATFORM_DIR}/types.gcc.h
	${PLATFORM_DIR}/types.lint.h
	${PLATFORM_DIR}/types.mac.h
	${PLATFORM_DIR}/types.posix.h
	${PLATFORM_DIR}/types.ppc.h
	${PLATFORM_DIR}/types.visualc.h
	${PLATFORM_DIR}/types.win32.h
	${PLATFORM_DIR}/types.xenon.h
	${PLATFORM_DIR}/typesLinux.h
	${PLATFORM_DIR}/typesPPC.h
	${PLATFORM_DIR}/typesWin32.h
	${PLATFORM_DIR}/typesX86UNIX.h
	${PLATFORM_DIR}/typetraits.h
)

SET( PLATFORM_SOURCES
    ${PLATFORM_DIR}/platform.cpp
	${PLATFORM_DIR}/platformAssert.cpp
	${PLATFORM_DIR}/platformCPU.cpp
	${PLATFORM_DIR}/platformCPUCount.cpp
	${PLATFORM_DIR}/platformFileIO.cpp
	${PLATFORM_DIR}/platformFont.cpp
	${PLATFORM_DIR}/platformMemory.cpp
	${PLATFORM_DIR}/platformNet.cpp
	${PLATFORM_DIR}/platformNetAsync.cpp
	${PLATFORM_DIR}/platformRedBook.cpp
	${PLATFORM_DIR}/platformTimer.cpp
	${PLATFORM_DIR}/platformVideoInfo.cpp
	${PLATFORM_DIR}/platformVolume.cpp
	${PLATFORM_DIR}/profiler.cpp
	platformCPUInfo.obj
)

SET( PLATFORMASYNC_HEADERS
    ${PLATFORMASYNC_DIR}/asyncBufferedStream.h
	${PLATFORMASYNC_DIR}/asyncPacketQueue.h
	${PLATFORMASYNC_DIR}/asyncPacketStream.h
	${PLATFORMASYNC_DIR}/asyncUpdate.h
)

SET( PLATFORMASYNC_SOURCES
    ${PLATFORMASYNC_DIR}/asyncUpdate.cpp
)

SET( PLATFORMINPUT_HEADERS
    ${PLATFORMINPUT_DIR}/event.h
	${PLATFORMINPUT_DIR}/IInputDevice.h
)

SET( PLATFORMINPUT_SOURCES
    ${PLATFORMINPUT_DIR}/event.cpp
)

IF( USE_LEAPMOTIONINPUT )
    SET( PLATFORMLEAPMOTION_HEADERS
        ${PLATFORMINPUTLEAPMOTION_DIR}/leapMotionConstants.h
		${PLATFORMINPUTLEAPMOTION_DIR}/leapMotionData.h
		${PLATFORMINPUTLEAPMOTION_DIR}/leapMotionDevice.h
		${PLATFORMINPUTLEAPMOTION_DIR}/leapMotionFrame.h
		${PLATFORMINPUTLEAPMOTION_DIR}/leapMotionFrameStore.h
		${PLATFORMINPUTLEAPMOTION_DIR}/leapMotionUtil.h
    )

    SET( PLATFORMLEAPMOTION_SOURCES
        ${PLATFORMINPUTLEAPMOTION_DIR}/leapMotionData.cpp
		${PLATFORMINPUTLEAPMOTION_DIR}/leapMotionDevice.cpp
		${PLATFORMINPUTLEAPMOTION_DIR}/leapMotionFrame.cpp
		${PLATFORMINPUTLEAPMOTION_DIR}/leapMotionFrameStore.cpp
		${PLATFORMINPUTLEAPMOTION_DIR}/leapMotionUtil.cpp
    )
ENDIF( USE_LEAPMOTIONINPUT )

IF( USE_OCULUSRIFT )
    SET( PLATFORMOCULUSVR_HEADERS
	    ${PLATFORMOCULUS_DIR}/barrelDistortionPostEffect.h
		${PLATFORMOCULUS_DIR}/oculusVRConstants.h
		${PLATFORMOCULUS_DIR}/oculusVRDevice.h
		${PLATFORMOCULUS_DIR}/oculusVRHMDDevice.h
		${PLATFORMOCULUS_DIR}/oculusVRSensorData.h
		${PLATFORMOCULUS_DIR}/oculusVRSensorDevice.h
		${PLATFORMOCULUS_DIR}/oculusVRUtil.h
	)
	
	SET( PLATFORMOCULUSVR_SOURCES
	    ${PLATFORMOCULUS_DIR}/barrelDistortionPostEffect.cpp
		${PLATFORMOCULUS_DIR}/oculusVRDevice.cpp
		${PLATFORMOCULUS_DIR}/oculusVRHMDDevice.cpp
		${PLATFORMOCULUS_DIR}/oculusVRSensorData.cpp
		${PLATFORMOCULUS_DIR}/oculusVRSensorDevice.cpp
		${PLATFORMOCULUS_DIR}/oculusVRUtil.cpp
	)
ENDIF ( USE_OCULUSRIFT )

IF( USE_RAZERHYDRAINPUT )
    SET( PLATFORMRAZERHYDRA_HEADERS
        ${PLATFORMINPUTRAZERHYDRA_DIR}/razerHydraConstants.h
		${PLATFORMINPUTRAZERHYDRA_DIR}/razerHydraData.h
		${PLATFORMINPUTRAZERHYDRA_DIR}/razerHydraDevice.h
		${PLATFORMINPUTRAZERHYDRA_DIR}/razerHydraFrame.h
		${PLATFORMINPUTRAZERHYDRA_DIR}/razerHydraFrameStore.h
		${PLATFORMINPUTRAZERHYDRA_DIR}/razerHydraUtil.h
    )

    SET( PLATFORMRAZERHYDRA_SOURCES
        ${PLATFORMINPUTRAZERHYDRA_DIR}/razerHydraData.cpp
		${PLATFORMINPUTRAZERHYDRA_DIR}/razerHydraDevice.cpp
		${PLATFORMINPUTRAZERHYDRA_DIR}/razerHydraFrame.cpp
		${PLATFORMINPUTRAZERHYDRA_DIR}/razerHydraFrameStore.cpp
		${PLATFORMINPUTRAZERHYDRA_DIR}/razerHydraUtil.cpp
    )
ENDIF( USE_RAZERHYDRAINPUT )

SET( PLATFORMMENUS_HEADERS
    ${PLATFORMMENUS_DIR}/menuBar.h
	${PLATFORMMENUS_DIR}/popupMenu.h
)

SET( PLATFORMMENUS_SOURCES
    ${PLATFORMMENUS_DIR}/menuBar.cpp
	${PLATFORMMENUS_DIR}/popupMenu.cpp
)

SET( PLATFORMNATIVEDIALOGS_HEADERS
    ${PLATFORMNATIVEDIALOGS_DIR}/fileDialog.h
	${PLATFORMNATIVEDIALOGS_DIR}/msgBox.h
)

SET( PLATFORMOUTPUT_HEADERS
    ${PLATFORMOUTPUT_DIR}/IDisplayDevice.h
)

SET( PLATFORMNATIVEDIALOGS_SOURCES
    ${PLATFORMNATIVEDIALOGS_DIR}/msgBox.cpp
)

SET( PLATFORMTEST_SOURCES
    ${PLATFORMTEST_DIR}/testAlerts.cpp
	${PLATFORMTEST_DIR}/testAsyncPacketQueue.cpp
	${PLATFORMTEST_DIR}/testBasicTypes.cpp
	${PLATFORMTEST_DIR}/testFile.cpp
	${PLATFORMTEST_DIR}/testNet.cpp
	${PLATFORMTEST_DIR}/testThreading.cpp
	${PLATFORMTEST_DIR}/testThreadPool.cpp
	${PLATFORMTEST_DIR}/testThreadSafeDeque.cpp
	${PLATFORMTEST_DIR}/testThreadSafePriorityQueue.cpp
	${PLATFORMTEST_DIR}/testThreadSafeRefCount.cpp
	${PLATFORMTEST_DIR}/testTimeManager.cpp
)

SET( PLATFORMTHREADS_HEADERS
    ${PLATFORMTHREADS_DIR}/mutex.h
	${PLATFORMTHREADS_DIR}/semaphore.h
	${PLATFORMTHREADS_DIR}/thread.h
	${PLATFORMTHREADS_DIR}/threadPool.h
	${PLATFORMTHREADS_DIR}/threadPoolAsyncIO.h
	${PLATFORMTHREADS_DIR}/threadSafeDeque.h
	${PLATFORMTHREADS_DIR}/threadSafeFreeList.h
	${PLATFORMTHREADS_DIR}/threadSafePriorityQueue.h
	${PLATFORMTHREADS_DIR}/threadSafeRefCount.h
)

SET( PLATFORMTHREADS_SOURCES
    ${PLATFORMTHREADS_DIR}/threadPool.cpp
)

SET( PLATFORMWIN32_HEADERS
    ${PLATFORMWIN32_DIR}/platformWin32.h
	${PLATFORMWIN32_DIR}/VFSRes.h
	${PLATFORMWIN32_DIR}/win_common_prefix.h
	${PLATFORMWIN32_DIR}/win_debug_prefix.h
	${PLATFORMWIN32_DIR}/win_release_prefix.h
	${PLATFORMWIN32_DIR}/winConsole.h
	${PLATFORMWIN32_DIR}/winDInputDevice.h
	${PLATFORMWIN32_DIR}/winDirectInput.h
	${PLATFORMWIN32_DIR}/winFont.h
	${PLATFORMWIN32_DIR}/winVolume.h
)

SET( PLATFORMWIN32_SOURCES
    ${PLATFORMWIN32_DIR}/cardProfile.cpp
	${PLATFORMWIN32_DIR}/winAsmBlit.cpp
	${PLATFORMWIN32_DIR}/winAsync.cpp
	${PLATFORMWIN32_DIR}/winConsole.cpp
	${PLATFORMWIN32_DIR}/winCPUInfo.cpp
	${PLATFORMWIN32_DIR}/winDInputDevice.cpp
	${PLATFORMWIN32_DIR}/winDirectInput.cpp
	${PLATFORMWIN32_DIR}/winDlibrary.cpp
	${PLATFORMWIN32_DIR}/winExec.cpp
	${PLATFORMWIN32_DIR}/winFileio.cpp
	${PLATFORMWIN32_DIR}/winFont.cpp
	${PLATFORMWIN32_DIR}/winInput.cpp
	${PLATFORMWIN32_DIR}/winMath.cpp
	${PLATFORMWIN32_DIR}/winMath_ASM.cpp
	${PLATFORMWIN32_DIR}/winMemory.cpp
	${PLATFORMWIN32_DIR}/winProcessControl.cpp
	${PLATFORMWIN32_DIR}/winRedbook.cpp
	${PLATFORMWIN32_DIR}/winSemaphore.cpp
	${PLATFORMWIN32_DIR}/winTime.cpp
	${PLATFORMWIN32_DIR}/winTimer.cpp
	${PLATFORMWIN32_DIR}/winTLS.cpp
	${PLATFORMWIN32_DIR}/winUser.cpp
	${PLATFORMWIN32_DIR}/winVFS.cpp
	${PLATFORMWIN32_DIR}/winVolume.cpp
	${PLATFORMWIN32_DIR}/winWindow.cpp
)

SET( PLATFORMWIN32MENUS_SOURCES
    ${PLATFORMWIN32MENUS_DIR}/menuBarWin32.cpp
	${PLATFORMWIN32MENUS_DIR}/popupMenuWin32.cpp
)

SET( PLATFORMWIN32MINIDUMP_HEADERS
    ${PLATFORMWIN32MINIDUMP_DIR}/winStackWalker.h
)

SET( PLATFORMWIN32MINIDUMP_SOURCES
    ${PLATFORMWIN32MINIDUMP_DIR}/winMiniDump.cpp
	${PLATFORMWIN32MINIDUMP_DIR}/winStackWalker.cpp
)

SET( PLATFORMWIN32NATIVEDIALOGS_SOURCES
    ${PLATFORMWIN32NATIVEDIALOGS_DIR}/fileDialog.cpp
	${PLATFORMWIN32NATIVEDIALOGS_DIR}/win32MsgBox.cpp
)

SET( PLATFORMWIN32THREADS_SOURCES
    ${PLATFORMWIN32THREADS_DIR}/mutex.cpp
	${PLATFORMWIN32THREADS_DIR}/thread.cpp
)

SET( PLATFORMWIN32VIDEOINFO_HEADERS
    ${PLATFORMWIN32VIDEOINFO_DIR}/wmiVideoInfo.h
)

SET( PLATFORMWIN32VIDEOINFO_SOURCES
    ${PLATFORMWIN32VIDEOINFO_DIR}/wmiVideoInfo.cpp
)

SET( POSTFX_HEADERS
    ${POSTFX_DIR}/postEffect.h
	${POSTFX_DIR}/postEffectCommon.h
	${POSTFX_DIR}/postEffectManager.h
	${POSTFX_DIR}/postEffectVis.h
)

SET( POSTFX_SOURCES
    ${POSTFX_DIR}/postEffect.cpp
	${POSTFX_DIR}/postEffectManager.cpp
	${POSTFX_DIR}/postEffectVis.cpp
)

SET( RENDERINSTANCE_HEADERS
    ${RENDERINSTANCE_DIR}/forcedMaterialMeshMgr.h
	${RENDERINSTANCE_DIR}/renderBinManager.h
	${RENDERINSTANCE_DIR}/renderFormatChanger.h
	${RENDERINSTANCE_DIR}/renderGlowMgr.h
	${RENDERINSTANCE_DIR}/renderImposterMgr.h
	${RENDERINSTANCE_DIR}/renderMeshMgr.h
	${RENDERINSTANCE_DIR}/renderObjectMgr.h
	${RENDERINSTANCE_DIR}/renderOcclusionMgr.h
	${RENDERINSTANCE_DIR}/renderParticleMgr.h
	${RENDERINSTANCE_DIR}/renderPassManager.h
	${RENDERINSTANCE_DIR}/renderPassStateToken.h
	${RENDERINSTANCE_DIR}/renderPrePassMgr.h
	${RENDERINSTANCE_DIR}/renderTerrainMgr.h
	${RENDERINSTANCE_DIR}/renderTexTargetBinManager.h
	${RENDERINSTANCE_DIR}/renderTranslucentMgr.h
)

SET( RENDERINSTANCE_SOURCES
    ${RENDERINSTANCE_DIR}/forcedMaterialMeshMgr.cpp
	${RENDERINSTANCE_DIR}/renderBinManager.cpp
	${RENDERINSTANCE_DIR}/renderFormatChanger.cpp
	${RENDERINSTANCE_DIR}/renderGlowMgr.cpp
	${RENDERINSTANCE_DIR}/renderImposterMgr.cpp
	${RENDERINSTANCE_DIR}/renderMeshMgr.cpp
	${RENDERINSTANCE_DIR}/renderObjectMgr.cpp
	${RENDERINSTANCE_DIR}/renderOcclusionMgr.cpp
	${RENDERINSTANCE_DIR}/renderParticleMgr.cpp
	${RENDERINSTANCE_DIR}/renderPassManager.cpp
	${RENDERINSTANCE_DIR}/renderPassStateToken.cpp
	${RENDERINSTANCE_DIR}/renderPrePassMgr.cpp
	${RENDERINSTANCE_DIR}/renderTerrainMgr.cpp
	${RENDERINSTANCE_DIR}/renderTexTargetBinManager.cpp
	${RENDERINSTANCE_DIR}/renderTranslucentMgr.cpp
)

SET( SCENE_HEADERS
    ${SCENE_DIR}/fogStructs.h
	${SCENE_DIR}/pathManager.h
	${SCENE_DIR}/reflectionManager.h
	${SCENE_DIR}/reflectionMatHook.h
	${SCENE_DIR}/reflector.h
	${SCENE_DIR}/sceneCameraState.h
	${SCENE_DIR}/sceneContainer.h
	${SCENE_DIR}/sceneManager.h
	${SCENE_DIR}/sceneObject.h
	${SCENE_DIR}/sceneObjectLightingPlugin.h
	${SCENE_DIR}/scenePolyhedralSpace.h
	${SCENE_DIR}/sceneRenderState.h
	${SCENE_DIR}/sceneSpace.h
	${SCENE_DIR}/sceneTracker.h
	${SCENE_DIR}/sgUtil.h
	${SCENE_DIR}/simPath.h
)

SET( SCENE_SOURCES
    ${SCENE_DIR}/pathManager.cpp
	${SCENE_DIR}/reflectionManager.cpp
	${SCENE_DIR}/reflectionMatHook.cpp
	${SCENE_DIR}/reflector.cpp
	${SCENE_DIR}/sceneCameraState.cpp
	${SCENE_DIR}/sceneContainer.cpp
	${SCENE_DIR}/sceneManager.cpp
	${SCENE_DIR}/sceneObject.cpp
	${SCENE_DIR}/scenePolyhedralSpace.cpp
	${SCENE_DIR}/sceneRenderState.cpp
	${SCENE_DIR}/sceneSpace.cpp
	${SCENE_DIR}/sceneTracker.cpp
	${SCENE_DIR}/sgUtil.cpp
	${SCENE_DIR}/simPath.cpp
)

SET( SCENECULLING_HEADERS
    ${SCENECULLING_DIR}/sceneCullingState.h
	${SCENECULLING_DIR}/sceneCullingVolume.h
	${SCENECULLING_DIR}/sceneZoneCullingState.h
)

SET( SCENECULLING_SOURCES
    ${SCENECULLING_DIR}/sceneCullingState.cpp
	${SCENECULLING_DIR}/sceneCullingVolume.cpp
	${SCENECULLING_DIR}/sceneZoneCullingState.cpp
)

SET( SCENEMIXIN_HEADERS
    ${SCENEMIXIN_DIR}/sceneAmbientSoundObject.h
	${SCENEMIXIN_DIR}/sceneAmbientSoundObject.impl.h
	${SCENEMIXIN_DIR}/scenePolyhedralObject.h
	${SCENEMIXIN_DIR}/scenePolyhedralObject.impl.h
)

SET( SCENEZONES_HEADERS
    ${SCENEZONES_DIR}/scenePolyhedralZone.h
	${SCENEZONES_DIR}/sceneRootZone.h
	${SCENEZONES_DIR}/sceneSimpleZone.h
	${SCENEZONES_DIR}/sceneTraversalState.h
	${SCENEZONES_DIR}/sceneZoneSpace.h
	${SCENEZONES_DIR}/sceneZoneSpaceManager.h
)

SET( SCENEZONES_SOURCES
    ${SCENEZONES_DIR}/scenePolyhedralZone.cpp
	${SCENEZONES_DIR}/sceneRootZone.cpp
	${SCENEZONES_DIR}/sceneSimpleZone.cpp
	${SCENEZONES_DIR}/sceneTraversalState.cpp
	${SCENEZONES_DIR}/sceneZoneSpace.cpp
	${SCENEZONES_DIR}/sceneZoneSpaceManager.cpp
)

SET( SFX_HEADERS
    ${SFX_DIR}/sfxAmbience.h
	${SFX_DIR}/sfxBuffer.h
	${SFX_DIR}/sfxCommon.h
	${SFX_DIR}/sfxController.h
	${SFX_DIR}/sfxDescription.h
	${SFX_DIR}/sfxDevice.h
	${SFX_DIR}/sfxEnvironment.h
	${SFX_DIR}/sfxFileStream.h
	${SFX_DIR}/sfxInternal.h
	${SFX_DIR}/sfxMemoryStream.h
	${SFX_DIR}/sfxModifier.h
	${SFX_DIR}/sfxParameter.h
	${SFX_DIR}/sfxPlayList.h
	${SFX_DIR}/sfxProfile.h
	${SFX_DIR}/sfxProvider.h
	${SFX_DIR}/sfxResource.h
	${SFX_DIR}/sfxSound.h
	${SFX_DIR}/sfxSoundscape.h
	${SFX_DIR}/sfxSource.h
	${SFX_DIR}/sfxState.h
	${SFX_DIR}/sfxStream.h
	${SFX_DIR}/sfxSystem.h
	${SFX_DIR}/sfxTrack.h
	${SFX_DIR}/sfxTypes.h
	${SFX_DIR}/sfxVoice.h
	${SFX_DIR}/sfxWorld.h
)

SET( SFX_SOURCES
    ${SFX_DIR}/sfxAmbience.cpp
	${SFX_DIR}/sfxBuffer.cpp
	${SFX_DIR}/sfxController.cpp
	${SFX_DIR}/sfxDescription.cpp
	${SFX_DIR}/sfxDevice.cpp
	${SFX_DIR}/sfxEnvironment.cpp
	${SFX_DIR}/sfxFileStream.cpp
	${SFX_DIR}/sfxInternal.cpp
	${SFX_DIR}/sfxMemoryStream.cpp
	${SFX_DIR}/sfxModifier.cpp
	${SFX_DIR}/sfxParameter.cpp
	${SFX_DIR}/sfxPlayList.cpp
	${SFX_DIR}/sfxProfile.cpp
	${SFX_DIR}/sfxProvider.cpp
	${SFX_DIR}/sfxResource.cpp
	${SFX_DIR}/sfxSound.cpp
	${SFX_DIR}/sfxSoundscape.cpp
	${SFX_DIR}/sfxSource.cpp
	${SFX_DIR}/sfxState.cpp
	${SFX_DIR}/sfxSystem.cpp
	${SFX_DIR}/sfxTrack.cpp
	${SFX_DIR}/sfxTypes.cpp
	${SFX_DIR}/sfxVoice.cpp
)

SET( SFXDSOUND_HEADERS
    ${SFXDSOUND_DIR}/dsFunctions.h
	${SFXDSOUND_DIR}/sfxDSBuffer.h
	${SFXDSOUND_DIR}/sfxDSDevice.h
	${SFXDSOUND_DIR}/sfxDSVoice.h
)

SET( SFXDSOUND_SOURCES
    ${SFXDSOUND_DIR}/sfxDSBuffer.cpp
	${SFXDSOUND_DIR}/sfxDSDevice.cpp
	${SFXDSOUND_DIR}/sfxDSProvider.cpp
	${SFXDSOUND_DIR}/sfxDSVoice.cpp
)

IF( USE_FMODSOUNDSYSTEM )
    SET( SFXFMOD_HEADERS
	    ${SFXFMOD_DIR}/fmodErrors.h
		${SFXFMOD_DIR}/fmodFunctions.h
		${SFXFMOD_DIR}/sfxFMODBuffer.h
		${SFXFMOD_DIR}/sfxFMODDevice.h
		${SFXFMOD_DIR}/sfxFMODEvent.h
		${SFXFMOD_DIR}/sfxFMODEventGroup.h
		${SFXFMOD_DIR}/sfxFMODEventSource.h
		${SFXFMOD_DIR}/sfxFMODPlugin.h
		${SFXFMOD_DIR}/sfxFMODProject.h
		${SFXFMOD_DIR}/sfxFMODVoice.h
	)
	
    SET( SFXFMOD_SOURCES
	    ${SFXFMOD_DIR}/sfxFMODBuffer.cpp
		${SFXFMOD_DIR}/sfxFMODDevice.cpp
		${SFXFMOD_DIR}/sfxFMODEvent.cpp
		${SFXFMOD_DIR}/sfxFMODEventGroup.cpp
		${SFXFMOD_DIR}/sfxFMODEventSource.cpp
		${SFXFMOD_DIR}/sfxFMODPlugin.cpp
		${SFXFMOD_DIR}/sfxFMODProject.cpp
		${SFXFMOD_DIR}/sfxFMODProvider.cpp
		${SFXFMOD_DIR}/sfxFMODVoice.cpp
	)
ENDIF( USE_FMODSOUNDSYSTEM )

SET( SFXMEDIA_HEADERS
    ${SFXMEDIA_DIR}/sfxVorbisStream.h
	${SFXMEDIA_DIR}/sfxWavStream.h
)

SET( SFXMEDIA_SOURCES
    ${SFXMEDIA_DIR}/sfxVorbisStream.cpp
	${SFXMEDIA_DIR}/sfxWavStream.cpp
)

SET( SFXNULL_HEADERS
    ${SFXNULL_DIR}/sfxNullBuffer.h
	${SFXNULL_DIR}/sfxNullDevice.h
	${SFXNULL_DIR}/sfxNullVoice.h
)

SET( SFXNULL_SOURCES
    ${SFXNULL_DIR}/sfxNullBuffer.cpp
	${SFXNULL_DIR}/sfxNullDevice.cpp
	${SFXNULL_DIR}/sfxNullProvider.cpp
	${SFXNULL_DIR}/sfxNullVoice.cpp
)

SET( SFXOPENAL_HEADERS
    ${SFXOPENAL_DIR}/aldlist.h
	${SFXOPENAL_DIR}/LoadOAL.h
	${SFXOPENAL_DIR}/sfxALBuffer.h
	${SFXOPENAL_DIR}/sfxALCaps.h
	${SFXOPENAL_DIR}/sfxALDevice.h
	${SFXOPENAL_DIR}/sfxALVoice.h
)

SET( SFXOPENAL_SOURCES
    ${SFXOPENAL_DIR}/aldlist.cpp
	${SFXOPENAL_DIR}/sfxALBuffer.cpp
	${SFXOPENAL_DIR}/sfxALDevice.cpp
	${SFXOPENAL_DIR}/sfxALProvider.cpp
	${SFXOPENAL_DIR}/sfxALVoice.cpp
)

SET( SFXOPENALWIN32_SOURCES
    ${SFXOPENALWIN32_DIR}/LoadOAL.cpp
)

SET( SFXXAUDIO_HEADERS
    ${SFXXAUDIO_DIR}/sfxXAudioBuffer.h
	${SFXXAUDIO_DIR}/sfxXAudioDevice.h
	${SFXXAUDIO_DIR}/sfxXAudioVoice.h
)

SET( SFXXAUDIO_SOURCES
    ${SFXXAUDIO_DIR}/sfxXAudioBuffer.cpp
	${SFXXAUDIO_DIR}/sfxXAudioDevice.cpp
	${SFXXAUDIO_DIR}/sfxXAudioProvider.cpp
	${SFXXAUDIO_DIR}/sfxXAudioVoice.cpp
)

SET( SHADERGEN_HEADERS
    ${SHADERGEN_DIR}/conditionerFeature.h
	${SHADERGEN_DIR}/featureMgr.h
	${SHADERGEN_DIR}/featureSet.h
	${SHADERGEN_DIR}/featureType.h
	${SHADERGEN_DIR}/langElement.h
	${SHADERGEN_DIR}/shaderComp.h
	${SHADERGEN_DIR}/shaderDependency.h
	${SHADERGEN_DIR}/shaderFeature.h
	${SHADERGEN_DIR}/shaderGen.h
	${SHADERGEN_DIR}/shaderGenVars.h
	${SHADERGEN_DIR}/shaderOp.h
)

SET( SHADERGEN_SOURCES
    ${SHADERGEN_DIR}/conditionerFeature.cpp
	${SHADERGEN_DIR}/featureMgr.cpp
	${SHADERGEN_DIR}/featureSet.cpp
	${SHADERGEN_DIR}/featureType.cpp
	${SHADERGEN_DIR}/langElement.cpp
	${SHADERGEN_DIR}/shaderComp.cpp
	${SHADERGEN_DIR}/shaderDependency.cpp
	${SHADERGEN_DIR}/shaderFeature.cpp
	${SHADERGEN_DIR}/shaderGen.cpp
	${SHADERGEN_DIR}/shaderGenVars.cpp
	${SHADERGEN_DIR}/shaderOp.cpp
)

IF( USE_OPENGL )
    SET( SHADERGENGLSL_HEADERS
	    ${SHADERGENGLSL_DIR}/bumpGLSL.h
		${SHADERGENGLSL_DIR}/depthGLSL.h
		${SHADERGENGLSL_DIR}/paraboloidGLSL.h
		${SHADERGENGLSL_DIR}/pixSpecularGLSL.h
		${SHADERGENGLSL_DIR}/shaderCompGLSL.h
		${SHADERGENGLSL_DIR}/shaderFeatureGLSL.h
		${SHADERGENGLSL_DIR}/shaderGenGLSL.h
	)
	
	SET( SHADERGENGLSL_SOURCES
	    ${SHADERGENGLSL_DIR}/bumpGLSL.cpp
		${SHADERGENGLSL_DIR}/depthGLSL.cpp
		${SHADERGENGLSL_DIR}/paraboloidGLSL.cpp
		${SHADERGENGLSL_DIR}/pixSpecularGLSL.cpp
		${SHADERGENGLSL_DIR}/shaderCompGLSL.cpp
		${SHADERGENGLSL_DIR}/shaderFeatureGLSL.cpp
		${SHADERGENGLSL_DIR}/shaderGenGLSL.cpp
		${SHADERGENGLSL_DIR}/shaderGenGLSLInit.cpp
	)
ENDIF( USE_OPENGL )

SET( SHADERGENHLSL_HEADERS
    ${SHADERGENHLSL_DIR}/bumpHLSL.h
	${SHADERGENHLSL_DIR}/depthHLSL.h
	${SHADERGENHLSL_DIR}/paraboloidHLSL.h
	${SHADERGENHLSL_DIR}/pixSpecularHLSL.h
	${SHADERGENHLSL_DIR}/shaderCompHLSL.h
	${SHADERGENHLSL_DIR}/shaderFeatureHLSL.h
	${SHADERGENHLSL_DIR}/shaderGenHLSL.h
)

SET( SHADERGENHLSL_SOURCES
    ${SHADERGENHLSL_DIR}/bumpHLSL.cpp
	${SHADERGENHLSL_DIR}/depthHLSL.cpp
	${SHADERGENHLSL_DIR}/paraboloidHLSL.cpp
	${SHADERGENHLSL_DIR}/pixSpecularHLSL.cpp
	${SHADERGENHLSL_DIR}/shaderCompHLSL.cpp
	${SHADERGENHLSL_DIR}/shaderFeatureHLSL.cpp
	${SHADERGENHLSL_DIR}/shaderGenHLSL.cpp
	${SHADERGENHLSL_DIR}/shaderGenHLSLInit.cpp
)

SET( SIM_HEADERS
    ${SIM_DIR}/actionMap.h
	${SIM_DIR}/connectionStringTable.h
	${SIM_DIR}/netConnection.h
	${SIM_DIR}/netInterface.h
	${SIM_DIR}/netObject.h
	${SIM_DIR}/netStringTable.h
)

SET( SIM_SOURCES
    ${SIM_DIR}/actionMap.cpp
	${SIM_DIR}/connectionStringTable.cpp
	${SIM_DIR}/netConnection.cpp
	${SIM_DIR}/netDownload.cpp
	${SIM_DIR}/netEvent.cpp
	${SIM_DIR}/netGhost.cpp
	${SIM_DIR}/netInterface.cpp
	${SIM_DIR}/netObject.cpp
	${SIM_DIR}/netStringTable.cpp
)

SET( T3D_HEADERS
    ${T3D_DIR}/aiClient.h
	${T3D_DIR}/aiConnection.h
	${T3D_DIR}/aiPlayer.h
	${T3D_DIR}/camera.h
	${T3D_DIR}/cameraSpline.h
	${T3D_DIR}/containerQuery.h
	${T3D_DIR}/convexShape.h
	${T3D_DIR}/debris.h
	${T3D_DIR}/gameFunctions.h
	${T3D_DIR}/gameTSCtrl.h
	${T3D_DIR}/groundPlane.h
	${T3D_DIR}/guiMaterialPreview.h
	${T3D_DIR}/guiObjectView.h
	${T3D_DIR}/item.h
	${T3D_DIR}/levelInfo.h
	${T3D_DIR}/lightAnimData.h
	${T3D_DIR}/lightBase.h
	${T3D_DIR}/lightDescription.h
	${T3D_DIR}/lightFlareData.h
	${T3D_DIR}/missionArea.h
	${T3D_DIR}/missionMarker.h
	${T3D_DIR}/objectTypes.h
	${T3D_DIR}/occlusionVolume.h
	${T3D_DIR}/pathCamera.h
	${T3D_DIR}/physicalZone.h
	${T3D_DIR}/player.h
	${T3D_DIR}/pointLight.h
	${T3D_DIR}/portal.h
	${T3D_DIR}/prefab.h
	${T3D_DIR}/projectile.h
	${T3D_DIR}/proximityMine.h
	${T3D_DIR}/resource.h
	${T3D_DIR}/rigid.h
	${T3D_DIR}/rigidShape.h
	${T3D_DIR}/shapeBase.h
	${T3D_DIR}/spotLight.h
	${T3D_DIR}/staticShape.h
	${T3D_DIR}/trigger.h
	${T3D_DIR}/tsStatic.h
	${T3D_DIR}/zone.h
)

SET( T3D_SOURCES
    ${T3D_DIR}/aiClient.cpp
	${T3D_DIR}/aiConnection.cpp
	${T3D_DIR}/aiPlayer.cpp
	${T3D_DIR}/camera.cpp
	${T3D_DIR}/cameraSpline.cpp
	${T3D_DIR}/containerQuery.cpp
	${T3D_DIR}/convexShape.cpp
	${T3D_DIR}/debris.cpp
	${T3D_DIR}/gameFunctions.cpp
	${T3D_DIR}/gameTSCtrl.cpp
	${T3D_DIR}/groundPlane.cpp
	${T3D_DIR}/guiMaterialPreview.cpp
	${T3D_DIR}/guiNoMouseCtrl.cpp
	${T3D_DIR}/guiObjectView.cpp
	${T3D_DIR}/item.cpp
	${T3D_DIR}/levelInfo.cpp
	${T3D_DIR}/lightAnimData.cpp
	${T3D_DIR}/lightBase.cpp
	${T3D_DIR}/lightDescription.cpp
	${T3D_DIR}/lightFlareData.cpp
	${T3D_DIR}/missionArea.cpp
	${T3D_DIR}/missionMarker.cpp
	${T3D_DIR}/occlusionVolume.cpp
	${T3D_DIR}/pathCamera.cpp
	${T3D_DIR}/physicalZone.cpp
	${T3D_DIR}/player.cpp
	${T3D_DIR}/pointLight.cpp
	${T3D_DIR}/portal.cpp
	${T3D_DIR}/prefab.cpp
	${T3D_DIR}/projectile.cpp
	${T3D_DIR}/proximityMine.cpp
	${T3D_DIR}/rigid.cpp
	${T3D_DIR}/rigidShape.cpp
	${T3D_DIR}/scopeAlwaysShape.cpp
	${T3D_DIR}/shapeBase.cpp
	${T3D_DIR}/shapeCollision.cpp
	${T3D_DIR}/shapeImage.cpp
	${T3D_DIR}/spotLight.cpp
	${T3D_DIR}/staticShape.cpp
	${T3D_DIR}/trigger.cpp
	${T3D_DIR}/tsStatic.cpp
	${T3D_DIR}/zone.cpp
)

SET( T3DDECAL_HEADERS
    ${T3DDECAL_DIR}/decalData.h
	${T3DDECAL_DIR}/decalDataFile.h
	${T3DDECAL_DIR}/decalInstance.h
	${T3DDECAL_DIR}/decalManager.h
	${T3DDECAL_DIR}/decalSphere.h
)

SET( T3DDECAL_SOURCES
    ${T3DDECAL_DIR}/decalData.cpp
	${T3DDECAL_DIR}/decalDataFile.cpp
	${T3DDECAL_DIR}/decalInstance.cpp
	${T3DDECAL_DIR}/decalManager.cpp
	${T3DDECAL_DIR}/decalSphere.cpp
)

SET( T3DEXAMPLES_HEADERS
    ${T3DEXAMPLES_DIR}/renderMeshExample.h
	${T3DEXAMPLES_DIR}/renderObjectExample.h
	${T3DEXAMPLES_DIR}/renderShapeExample.h
)

SET( T3DEXAMPLES_SOURCES
    ${T3DEXAMPLES_DIR}/renderMeshExample.cpp
	${T3DEXAMPLES_DIR}/renderObjectExample.cpp
	${T3DEXAMPLES_DIR}/renderShapeExample.cpp
)

SET( T3DFPS_SOURCES
    ${T3DFPS_DIR}/guiClockHud.cpp
	${T3DFPS_DIR}/guiCrossHairHud.cpp
	${T3DFPS_DIR}/guiHealthBarHud.cpp
	${T3DFPS_DIR}/guiHealthTextHud.cpp
	${T3DFPS_DIR}/guiShapeNameHud.cpp
)

SET( T3DFX_HEADERS
    ${T3DFX_DIR}/cameraFXMgr.h
	${T3DFX_DIR}/explosion.h
	${T3DFX_DIR}/fxFoliageReplicator.h
	${T3DFX_DIR}/fxShapeReplicator.h
	${T3DFX_DIR}/groundCover.h
	${T3DFX_DIR}/lightning.h
	${T3DFX_DIR}/particle.h
	${T3DFX_DIR}/particleEmitter.h
	${T3DFX_DIR}/particleEmitterNode.h
	${T3DFX_DIR}/precipitation.h
	${T3DFX_DIR}/splash.h
	${T3DFX_DIR}/windEmitter.h
)

SET( T3DFX_SOURCES
    ${T3DFX_DIR}/cameraFXMgr.cpp
	${T3DFX_DIR}/explosion.cpp
	${T3DFX_DIR}/fxFoliageReplicator.cpp
	${T3DFX_DIR}/fxShapeReplicator.cpp
	${T3DFX_DIR}/groundCover.cpp
	${T3DFX_DIR}/lightning.cpp
	${T3DFX_DIR}/particle.cpp
	${T3DFX_DIR}/particleEmitter.cpp
	${T3DFX_DIR}/particleEmitterNode.cpp
	${T3DFX_DIR}/precipitation.cpp
	${T3DFX_DIR}/splash.cpp
	${T3DFX_DIR}/windEmitter.cpp
)

SET( T3DGAMEBASE_HEADERS
    ${T3DGAMEBASE_DIR}/gameBase.h
	${T3DGAMEBASE_DIR}/gameConnection.h
	${T3DGAMEBASE_DIR}/gameConnectionEvents.h
	${T3DGAMEBASE_DIR}/gameProcess.h
	${T3DGAMEBASE_DIR}/moveList.h
	${T3DGAMEBASE_DIR}/moveManager.h
	${T3DGAMEBASE_DIR}/processList.h
	${T3DGAMEBASE_DIR}/tickCache.h
)

SET( T3DGAMEBASE_SOURCES
    ${T3DGAMEBASE_DIR}/gameBase.cpp
	${T3DGAMEBASE_DIR}/gameConnection.cpp
	${T3DGAMEBASE_DIR}/gameConnectionEvents.cpp
	${T3DGAMEBASE_DIR}/gameProcess.cpp
	${T3DGAMEBASE_DIR}/moveList.cpp
	${T3DGAMEBASE_DIR}/moveManager.cpp
	${T3DGAMEBASE_DIR}/processList.cpp
	${T3DGAMEBASE_DIR}/tickCache.cpp
)

IF( TORQUE_EXTENDED_MOVE )
    SET( T3DGAMEBASEEXTENDED_HEADERS
        ${T3DGAMEBASEEXTENDED_DIR}/extendedGameProcess.h
		${T3DGAMEBASEEXTENDED_DIR}/extendedMove.h
		${T3DGAMEBASEEXTENDED_DIR}/extendedMoveList.h
    )

    SET( T3DGAMEBASEEXTENDED_SOURCES
        ${T3DGAMEBASEEXTENDED_DIR}/extendedGameProcess.cpp
		${T3DGAMEBASEEXTENDED_DIR}/extendedMove.cpp
		${T3DGAMEBASEEXTENDED_DIR}/extendedMoveList.cpp
    )
ENDIF( TORQUE_EXTENDED_MOVE )

IF( TORQUE_HIFI_NET )
    SET( T3DGAMEBASEHIFI_HEADERS
	    ${T3DGAMEBASEHIFI_DIR}/hifiGameProcess.h
		${T3DGAMEBASEHIFI_DIR}/hifiMoveList.h
	)
	
	SET( T3DGAMEBASEHIFI_SOURCES
	    ${T3DGAMEBASEHIFI_DIR}/hifiGameProcess.cpp
		${T3DGAMEBASEHIFI_DIR}/hifiMoveList.cpp
	)
ENDIF( TORQUE_HIFI_NET )

SET( T3DGAMEBASESTD_HEADERS
    ${T3DGAMEBASESTD_DIR}/stdGameProcess.h
	${T3DGAMEBASESTD_DIR}/stdMoveList.h
)

SET( T3DGAMEBASESTD_SOURCES
    ${T3DGAMEBASESTD_DIR}/stdGameProcess.cpp
	${T3DGAMEBASESTD_DIR}/stdMoveList.cpp
)

SET( T3DGAMEBASEPHYSICS_HEADERS
    ${T3DPHYSICS_DIR}/physicsBody.h
	${T3DPHYSICS_DIR}/physicsCollision.h
	${T3DPHYSICS_DIR}/physicsCommon.h
	${T3DPHYSICS_DIR}/physicsDebris.h
	${T3DPHYSICS_DIR}/physicsEvents.h
	${T3DPHYSICS_DIR}/physicsForce.h
	${T3DPHYSICS_DIR}/physicsObject.h
	${T3DPHYSICS_DIR}/physicsPlayer.h
	${T3DPHYSICS_DIR}/physicsPlugin.h
	${T3DPHYSICS_DIR}/physicsShape.h
	${T3DPHYSICS_DIR}/physicsUserData.h
	${T3DPHYSICS_DIR}/physicsWorld.h
)

SET( T3DGAMEBASEPHYSICS_SOURCES
    ${T3DPHYSICS_DIR}/physicsBody.cpp
	${T3DPHYSICS_DIR}/physicsDebris.cpp
	${T3DPHYSICS_DIR}/physicsEvents.cpp
	${T3DPHYSICS_DIR}/physicsForce.cpp
	${T3DPHYSICS_DIR}/physicsObject.cpp
	${T3DPHYSICS_DIR}/physicsPlayer.cpp
	${T3DPHYSICS_DIR}/physicsPlugin.cpp
	${T3DPHYSICS_DIR}/physicsShape.cpp
	${T3DPHYSICS_DIR}/physicsUserData.cpp
	${T3DPHYSICS_DIR}/physicsWorld.cpp
)

IF( USE_BULLETPHYSICS )
    SET( T3DGAMEBASEPHYSICSBULLET_HEADERS
	    ${T3DPHYSICSBULLET_DIR}/bt.h
		${T3DPHYSICSBULLET_DIR}/btBody.h
		${T3DPHYSICSBULLET_DIR}/btCasts.h
		${T3DPHYSICSBULLET_DIR}/btCollision.h
		${T3DPHYSICSBULLET_DIR}/btDebugDraw.h
		${T3DPHYSICSBULLET_DIR}/btPlayer.h
		${T3DPHYSICSBULLET_DIR}/btPlugin.h
		${T3DPHYSICSBULLET_DIR}/btWorld.h
	)
	
    SET( T3DGAMEBASEPHYSICSBULLET_SOURCES
	    ${T3DPHYSICSBULLET_DIR}/btBody.cpp
		${T3DPHYSICSBULLET_DIR}/btCollision.cpp
		${T3DPHYSICSBULLET_DIR}/btDebugDraw.cpp
		${T3DPHYSICSBULLET_DIR}/btPlayer.cpp
		${T3DPHYSICSBULLET_DIR}/btPlugin.cpp
		${T3DPHYSICSBULLET_DIR}/btWorld.cpp
	)	
ENDIF( USE_BULLETPHYSICS )

IF( USE_NVIDIAPHYSX )
    SET( T3DGAMEBASEPHYSICSPHYSX_HEADERS
	    ${T3DPHYSICSPHYSX_DIR}/px.h
		${T3DPHYSICSPHYSX_DIR}/pxBody.h
		${T3DPHYSICSPHYSX_DIR}/pxCasts.h
		${T3DPHYSICSPHYSX_DIR}/pxCloth.h
		${T3DPHYSICSPHYSX_DIR}/pxCollision.h
		${T3DPHYSICSPHYSX_DIR}/pxContactReporter.h
		${T3DPHYSICSPHYSX_DIR}/pxFluid.h
		${T3DPHYSICSPHYSX_DIR}/pxMaterial.h
		${T3DPHYSICSPHYSX_DIR}/pxMultiActor.h
		${T3DPHYSICSPHYSX_DIR}/pxPlayer.h
		${T3DPHYSICSPHYSX_DIR}/pxPlugin.h
		${T3DPHYSICSPHYSX_DIR}/pxStream.h
		${T3DPHYSICSPHYSX_DIR}/pxUtils.h
		${T3DPHYSICSPHYSX_DIR}/pxWorld.h
	)
	
    SET( T3DGAMEBASEPHYSICSPHYSX_SOURCES
	    ${T3DPHYSICSPHYSX_DIR}/pxBody.cpp
		${T3DPHYSICSPHYSX_DIR}/pxCloth.cpp
		${T3DPHYSICSPHYSX_DIR}/pxCollision.cpp
		${T3DPHYSICSPHYSX_DIR}/pxContactReporter.cpp
		${T3DPHYSICSPHYSX_DIR}/pxFluid.cpp
		${T3DPHYSICSPHYSX_DIR}/pxMaterial.cpp
		${T3DPHYSICSPHYSX_DIR}/pxMultiActor.cpp
		${T3DPHYSICSPHYSX_DIR}/pxPlayer.cpp
		${T3DPHYSICSPHYSX_DIR}/pxPlugin.cpp
		${T3DPHYSICSPHYSX_DIR}/pxStream.cpp
		${T3DPHYSICSPHYSX_DIR}/pxUtils.cpp
		${T3DPHYSICSPHYSX_DIR}/pxWorld.cpp
	)
ENDIF( USE_NVIDIAPHYSX )

# Dushan - this isn't in original project
#SET( T3DSCENECOMPONENT_HEADERS
#    ${T3DSCENECOMPONENT_DIR}/T3DSceneClient.h
#	${T3DSCENECOMPONENT_DIR}/T3DSceneComponent.h
#	${T3DSCENECOMPONENT_DIR}/T3DTransform.h
#)
#
#SET( T3DSCENECOMPONENT_SOURCES
#    ${T3DSCENECOMPONENT_DIR}/T3DSceneClient.cpp
#	${T3DSCENECOMPONENT_DIR}/T3DSceneComponent.cpp
#	${T3DSCENECOMPONENT_DIR}/T3DTransform.cpp
#)

SET( T3DSFX_HEADERS
    ${T3DSFX_DIR}/sfx3DWorld.h
	${T3DSFX_DIR}/sfxEmitter.h
	${T3DSFX_DIR}/sfxSpace.h
)

SET( T3DSFX_SOURCES
    ${T3DSFX_DIR}/sfx3DWorld.cpp
	${T3DSFX_DIR}/sfxEmitter.cpp
	${T3DSFX_DIR}/sfxSpace.cpp
)

SET( T3DTURRET_HEADERS
    ${T3DTURRET_DIR}/aiTurretShape.h
	${T3DTURRET_DIR}/turretShape.h
)

SET( T3DTURRET_SOURCES
    ${T3DTURRET_DIR}/aiTurretShape.cpp
	${T3DTURRET_DIR}/turretShape.cpp
)

SET( T3DVEHICLES_HEADERS
    ${T3DVEHICLES_DIR}/flyingVehicle.h
	${T3DVEHICLES_DIR}/hoverVehicle.h
	${T3DVEHICLES_DIR}/vehicle.h
	${T3DVEHICLES_DIR}/vehicleBlocker.h
	${T3DVEHICLES_DIR}/wheeledVehicle.h
)

SET( T3DVEHICLES_SOURCES
    ${T3DVEHICLES_DIR}/flyingVehicle.cpp
	${T3DVEHICLES_DIR}/guiSpeedometer.cpp
	${T3DVEHICLES_DIR}/hoverVehicle.cpp
	${T3DVEHICLES_DIR}/vehicle.cpp
	${T3DVEHICLES_DIR}/vehicleBlocker.cpp
	${T3DVEHICLES_DIR}/wheeledVehicle.cpp
)

SET( TERRAIN_HEADERS
    ${TERRAIN_DIR}/terrCell.h
	${TERRAIN_DIR}/terrCellMaterial.h
	${TERRAIN_DIR}/terrCollision.h
	${TERRAIN_DIR}/terrData.h
	${TERRAIN_DIR}/terrFeatureTypes.h
	${TERRAIN_DIR}/terrFile.h
	${TERRAIN_DIR}/terrMaterial.h
	${TERRAIN_DIR}/terrRender.h
)

SET( TERRAIN_SOURCES
    ${TERRAIN_DIR}/terrCell.cpp
	${TERRAIN_DIR}/terrCellMaterial.cpp
	${TERRAIN_DIR}/terrCollision.cpp
	${TERRAIN_DIR}/terrData.cpp
	${TERRAIN_DIR}/terrExport.cpp
	${TERRAIN_DIR}/terrFeatureTypes.cpp
	${TERRAIN_DIR}/terrFile.cpp
	${TERRAIN_DIR}/terrImport.cpp
	${TERRAIN_DIR}/terrLighting.cpp
	${TERRAIN_DIR}/terrMaterial.cpp
	${TERRAIN_DIR}/terrRender.cpp
)

IF( USE_OPENGL )
    SET( TERRAINGLSL_HEADERS
        ${TERRAINGLSL_DIR}/terrFeatureGLSL.h
    )

    SET( TERRAINGLSL_SOURCES
        ${TERRAINGLSL_DIR}/terrFeatureGLSL.cpp
    )
ENDIF( USE_OPENGL )

SET( TERRAINHLSL_HEADERS
    ${TERRAINHLSL_DIR}/terrFeatureHLSL.h
)

SET( TERRAINHLSL_SOURCES
    ${TERRAINHLSL_DIR}/terrFeatureHLSL.cpp
)

SET( TS_HEADERS
    ${TS_DIR}/instancingMatHook.h
	${TS_DIR}/tsDecal.h
	${TS_DIR}/tsIntegerSet.h
	${TS_DIR}/tsLastDetail.h
	${TS_DIR}/tsMaterialList.h
	${TS_DIR}/tsMesh.h
	${TS_DIR}/tsMeshIntrinsics.h
	${TS_DIR}/tsPartInstance.h
	${TS_DIR}/tsRenderState.h
	${TS_DIR}/tsShape.h
	${TS_DIR}/tsShapeAlloc.h
	${TS_DIR}/tsShapeConstruct.h
	${TS_DIR}/tsShapeInstance.h
	${TS_DIR}/tsSortedMesh.h
	${TS_DIR}/tsTransform.h
)

SET( TS_SOURCES
    ${TS_DIR}/instancingMatHook.cpp
	${TS_DIR}/tsAnimate.cpp
	${TS_DIR}/tsCollision.cpp
	${TS_DIR}/tsDecal.cpp
	${TS_DIR}/tsDump.cpp
	${TS_DIR}/tsIntegerSet.cpp
	${TS_DIR}/tsLastDetail.cpp
	${TS_DIR}/tsMaterialList.cpp
	${TS_DIR}/tsMesh.cpp
	${TS_DIR}/tsMeshFit.cpp
	${TS_DIR}/tsMeshIntrinsics.cpp
	${TS_DIR}/tsPartInstance.cpp
	${TS_DIR}/tsRenderState.cpp
	${TS_DIR}/tsShape.cpp
	${TS_DIR}/tsShapeAlloc.cpp
	${TS_DIR}/tsShapeConstruct.cpp
	${TS_DIR}/tsShapeEdit.cpp
	${TS_DIR}/tsShapeInstance.cpp
	${TS_DIR}/tsShapeOldRead.cpp
	${TS_DIR}/tsSortedMesh.cpp
	${TS_DIR}/tsThread.cpp
	${TS_DIR}/tsTransform.cpp
)

SET( TSARCH_HEADERS
    ${TSARCH_DIR}/tsMeshIntrinsics.arch.h
)

SET( TSARCH_SOURCES
    ${TSARCH_DIR}/tsMeshIntrinsics.sse.cpp
	${TSARCH_DIR}/tsMeshIntrinsics.sse4.cpp
)

SET( TSCOLLADA_HEADERS
    ${TSCOLLADA_DIR}/colladaAppMaterial.h
	${TSCOLLADA_DIR}/colladaAppMesh.h
	${TSCOLLADA_DIR}/colladaAppNode.h
	${TSCOLLADA_DIR}/colladaAppSequence.h
	${TSCOLLADA_DIR}/colladaExtensions.h
	${TSCOLLADA_DIR}/colladaShapeLoader.h
	${TSCOLLADA_DIR}/colladaUtils.h
)

SET( TSCOLLADA_SOURCES
    ${TSCOLLADA_DIR}/colladaAppMaterial.cpp
	${TSCOLLADA_DIR}/colladaAppMesh.cpp
	${TSCOLLADA_DIR}/colladaAppNode.cpp
	${TSCOLLADA_DIR}/colladaAppSequence.cpp
	${TSCOLLADA_DIR}/colladaExtensions.cpp
	${TSCOLLADA_DIR}/colladaImport.cpp
	${TSCOLLADA_DIR}/colladaLights.cpp
	${TSCOLLADA_DIR}/colladaShapeLoader.cpp
	${TSCOLLADA_DIR}/colladaUtils.cpp
)

SET( TSLOADER_HEADERS
    ${TSLOADER_DIR}/appMaterial.h
	${TSLOADER_DIR}/appMesh.h
	${TSLOADER_DIR}/appNode.h
	${TSLOADER_DIR}/appSequence.h
	${TSLOADER_DIR}/tsShapeLoader.h
)

SET( TSLOADER_SOURCES
    ${TSLOADER_DIR}/appMesh.cpp
	${TSLOADER_DIR}/appNode.cpp
	${TSLOADER_DIR}/tsShapeLoader.cpp
)

SET( UNIT_HEADERS
    ${UNIT_DIR}/memoryTester.h
	${UNIT_DIR}/test.h
	${UNIT_DIR}/unitTestComponentInterface.h
)

SET( UNIT_SOURCES
    ${UNIT_DIR}/consoleTest.cpp
	${UNIT_DIR}/memoryTester.cpp
	${UNIT_DIR}/test.cpp
	${UNIT_DIR}/unitTestComponentInterface.cpp
)

SET( UNITTESTS_SOURCES
    ${UNITTESTS_DIR}/testComponents.cpp
	${UNITTESTS_DIR}/testDefaultConstruction.cpp
	${UNITTESTS_DIR}/testMatrixMul.cpp
	${UNITTESTS_DIR}/testRuntimeClassRep.cpp
	${UNITTESTS_DIR}/testSwizzle.cpp
	${UNITTESTS_DIR}/testThreadStatic.cpp
	${UNITTESTS_DIR}/testThreadStaticPerformance.cpp
	${UNITTESTS_DIR}/testVector.cpp
)

SET( UTIL_HEADERS
    ${UTIL_DIR}/catmullRom.h
	${UTIL_DIR}/fpsTracker.h
	${UTIL_DIR}/imposterCapture.h
	${UTIL_DIR}/interpolatedChangeProperty.h
	${UTIL_DIR}/noise2d.h
	${UTIL_DIR}/quadTreeTracer.h
	${UTIL_DIR}/rectClipper.h
	${UTIL_DIR}/returnType.h
	${UTIL_DIR}/sampler.h
	${UTIL_DIR}/scopeTracker.h
	${UTIL_DIR}/settings.h
	${UTIL_DIR}/tempAlloc.h
	${UTIL_DIR}/triBoxCheck.h
	${UTIL_DIR}/triRayCheck.h
	${UTIL_DIR}/undo.h
)

SET( UTIL_SOURCES
    ${UTIL_DIR}/catmullRom.cpp
	${UTIL_DIR}/fpsTracker.cpp
	${UTIL_DIR}/imposterCapture.cpp
	${UTIL_DIR}/noise2d.cpp
	${UTIL_DIR}/quadTreeTracer.cpp
	${UTIL_DIR}/rectClipper.cpp
	${UTIL_DIR}/sampler.cpp
	${UTIL_DIR}/settings.cpp
	${UTIL_DIR}/triBoxCheck.cpp
	${UTIL_DIR}/triRayCheck.cpp
	${UTIL_DIR}/undo.cpp
)

SET( UTILMESSAGING_HEADERS
    ${UTILMESSAGING_DIR}/dispatcher.h
	${UTILMESSAGING_DIR}/eventManager.h
	${UTILMESSAGING_DIR}/message.h
	${UTILMESSAGING_DIR}/messageForwarder.h
	${UTILMESSAGING_DIR}/scriptMsgListener.h
)

SET( UTILMESSAGING_SOURCES
    ${UTILMESSAGING_DIR}/dispatcher.cpp
	${UTILMESSAGING_DIR}/eventManager.cpp
	${UTILMESSAGING_DIR}/message.cpp
	${UTILMESSAGING_DIR}/messageForwarder.cpp
	${UTILMESSAGING_DIR}/scriptMsgListener.cpp
	
)

SET( WINDOWSMANAGER_HEADERS
    ${WINDOWMANAGER_DIR}/platformCursorController.h
	${WINDOWMANAGER_DIR}/platformWindow.h
	${WINDOWMANAGER_DIR}/platformWindowMgr.h
	${WINDOWMANAGER_DIR}/windowInputGenerator.h
)

SET( WINDOWSMANAGER_SOURCES
    ${WINDOWMANAGER_DIR}/platformCursorController.cpp
	${WINDOWMANAGER_DIR}/platformInterface.cpp
	${WINDOWMANAGER_DIR}/platformWindow.cpp
	${WINDOWMANAGER_DIR}/windowInputGenerator.cpp
)

#SET( WINDOWSMANAGERDEDICATED_HEADERS
#    ${WINDOWMANAGERDEDICATED_DIR}/dedicatedWindowStub.h
#)

#SET( WINDOWSMANAGERDEDICATED_SOURCES
#    ${WINDOWMANAGERDEDICATED_DIR}/dedicatedWindowStub.cpp
#)

SET( WINDOWSMANAGERTEST_SOURCES
    ${WINDOWMANAGERTEST_DIR}/testWinMgr.cpp
)

SET( WINDOWSMANAGERWIN32_HEADERS
    ${WINDOWMANAGERWIN32_DIR}/win32CursorController.h
	${WINDOWMANAGERWIN32_DIR}/win32Window.h
	${WINDOWMANAGERWIN32_DIR}/win32WindowMgr.h
	${WINDOWMANAGERWIN32_DIR}/winDispatch.h
)

SET( WINDOWSMANAGERWIN32_SOURCES
    ${WINDOWMANAGERWIN32_DIR}/win32CursorController.cpp
	${WINDOWMANAGERWIN32_DIR}/win32SplashScreen.cpp
	${WINDOWMANAGERWIN32_DIR}/win32Window.cpp
	${WINDOWMANAGERWIN32_DIR}/win32WindowMgr.cpp
	${WINDOWMANAGERWIN32_DIR}/winDispatch.cpp
)

# Dushan - This is how visual studio directories (or filters) are generated
SOURCE_GROUP( Header\ Files\\app                                      FILES ${APP_HEADERS} )
SOURCE_GROUP( Header\ Files\\app\\net                                 FILES ${NET_HEADERS} )
SOURCE_GROUP( Header\ Files\\cinterface                               FILES ${CINT_HEADERS} )
SOURCE_GROUP( Header\ Files\\collision                                FILES ${COLL_HEADERS} )
SOURCE_GROUP( Header\ Files\\component\\interfaces                    FILES ${COMPINT_HEADERS} )
SOURCE_GROUP( Header\ Files\\component                                FILES ${COMP_HEADERS} )
SOURCE_GROUP( Header\ Files\\console                                  FILES ${CONSOLE_HEADERS} )
SOURCE_GROUP( Header\ Files\\core                                     FILES ${CORE_HEADERS} )
SOURCE_GROUP( Header\ Files\\core\\ogg                                FILES ${COREOGG_HEADERS} )
SOURCE_GROUP( Header\ Files\\core\\stream                             FILES ${CORESTREAM_HEADERS} )
SOURCE_GROUP( Header\ Files\\core\\strings                            FILES ${CORESTRINGS_HEADERS} )
SOURCE_GROUP( Header\ Files\\core\\util                               FILES ${COREUTIL_HEADERS} )
SOURCE_GROUP( Header\ Files\\core\\util\\journal                      FILES ${COREUTILJOURNAL_HEADERS} )
SOURCE_GROUP( Header\ Files\\core\\zip                                FILES ${COREUTILTESTZIP_HEADERS} )
SOURCE_GROUP( Header\ Files\\core\\zip\\unitTest                      FILES ${COREUTILTESTZIPUNITTESTS_HEADERS} )
SOURCE_GROUP( Header\ Files\\environment                              FILES ${ENVIRONMENT_HEADERS} )
SOURCE_GROUP( Header\ Files\\environment\\editors                     FILES ${ENVIRONMENTEDITOR_HEADERS} )
SOURCE_GROUP( Header\ Files\\forest                                   FILES ${FOREST_HEADERS} )
SOURCE_GROUP( Header\ Files\\forest\\editor                           FILES ${FORESTEDITOR_HEADERS} )

IF( USE_OPENGL )
    SOURCE_GROUP( Header\ Files\\forest\\glsl                             FILES ${FORESTGLSL_HEADERS} )
ENDIF( USE_OPENGL )

SOURCE_GROUP( Header\ Files\\forest\\hlsl                             FILES ${FORESTHLSL_HEADERS} )
SOURCE_GROUP( Header\ Files\\forest\\ts                               FILES ${FORESTTS_HEADERS} )
SOURCE_GROUP( Header\ Files\\gfx                                      FILES ${GFX_HEADERS} )
SOURCE_GROUP( Header\ Files\\gfx\\bitmap                              FILES ${GFXBITMAP_HEADERS} )
SOURCE_GROUP( Header\ Files\\gfx\\d3d9                                FILES ${D3D9_HEADERS} )
SOURCE_GROUP( Header\ Files\\gfx\\d3d9\\pc                            FILES ${D3D9PC_HEADERS} )

IF( USE_OPENGL )
    SOURCE_GROUP( Header\ Files\\gfx\\gl                                  FILES ${GL_HEADERS} )
	SOURCE_GROUP( Header\ Files\\gfx\\gl\\ggl                             FILES ${GLGGL_HEADERS} )
	SOURCE_GROUP( Header\ Files\\gfx\\gl\\ggl\\win32                      FILES ${GLGGLWIN32_HEADERS} )
ENDIF( USE_OPENGL )

SOURCE_GROUP( Header\ Files\\gfx\\Null                                FILES ${GFXNULL_HEADERS} )
SOURCE_GROUP( Header\ Files\\gfx\\sim                                 FILES ${GFXSIM_HEADERS} )
SOURCE_GROUP( Header\ Files\\gfx\\util                                FILES ${GFXUTIL_HEADERS} )
SOURCE_GROUP( Header\ Files\\gfx\\video                               FILES ${GFXVIDEO_HEADERS} )
SOURCE_GROUP( Header\ Files\\gui\\3d                                  FILES ${GUI3D_HEADERS} )
SOURCE_GROUP( Header\ Files\\gui\\buttons                             FILES ${GUIBUTTONS_HEADERS} )
SOURCE_GROUP( Header\ Files\\gui\\containers                          FILES ${GUICONTAINERS_HEADERS} )
SOURCE_GROUP( Header\ Files\\gui\\controls                            FILES ${GUICONTROLS_HEADERS} )
SOURCE_GROUP( Header\ Files\\gui\\core                                FILES ${GUICORE_HEADERS} )
SOURCE_GROUP( Header\ Files\\gui\\editor                              FILES ${GUIEDITOR_HEADERS} )
SOURCE_GROUP( Header\ Files\\gui\\editor\\inspector                   FILES ${GUIEDITORINSPECTOR_HEADERS} )
SOURCE_GROUP( Header\ Files\\gui\\game                                FILES ${GUIGAME_HEADERS} )
SOURCE_GROUP( Header\ Files\\gui\\shiny                               FILES ${GUISHINY_HEADERS} )
SOURCE_GROUP( Header\ Files\\gui\\theora                              FILES ${GUITHEORA_HEADERS} )
SOURCE_GROUP( Header\ Files\\gui\\utility                             FILES ${GUIUTILITY_HEADERS} )
SOURCE_GROUP( Header\ Files\\gui\\worldEditor                         FILES ${GUIWORLDEDITOR_HEADERS} )
SOURCE_GROUP( Header\ Files\\i18n                                     FILES ${I18N_HEADERS} )
SOURCE_GROUP( Header\ Files\\lighting                                 FILES ${LIGHTING_HEADERS} )
SOURCE_GROUP( Header\ Files\\lighting\\advanced                       FILES ${LIGHTINGADVANCED_HEADERS} )

IF( USE_OPENGL )
    SOURCE_GROUP( Header\ Files\\lighting\\advanced\\glsl                 FILES ${LIGHTINGADVANCEDGLSL_HEADERS} )
ENDIF( USE_OPENGL )

SOURCE_GROUP( Header\ Files\\lighting\\advanced\\hlsl                 FILES ${LIGHTINGADVANCEDHLSL_HEADERS} )
SOURCE_GROUP( Header\ Files\\lighting\\basic                          FILES ${LIGHTINGBASIC_HEADERS} )
SOURCE_GROUP( Header\ Files\\lighting\\common                         FILES ${LIGHTINGCOMMON_HEADERS} )
SOURCE_GROUP( Header\ Files\\lighting\\shadowMap                      FILES ${LIGHTINGSHADOWMAP_HEADERS} )
SOURCE_GROUP( Header\ Files\\materials                                FILES ${MATERIALS_HEADERS} )
SOURCE_GROUP( Header\ Files\\math                                     FILES ${MATH_HEADERS} )
SOURCE_GROUP( Header\ Files\\math\\util                               FILES ${MATHUTIL_HEADERS} )

IF( USE_RECASTNAVITATION )
    SOURCE_GROUP( Header\ Files\\navigation                               FILES ${RECASTNAVITATION_HEADERS} )
ENDIF( USE_RECASTNAVITATION )

SOURCE_GROUP( Header\ Files\\platform                                 FILES ${PLATFORM_HEADERS} )
SOURCE_GROUP( Header\ Files\\platform\\async                          FILES ${PLATFORMASYNC_HEADERS} )
SOURCE_GROUP( Header\ Files\\platform\\input                          FILES ${PLATFORMINPUT_HEADERS} )

IF( USE_LEAPMOTIONINPUT )
    SOURCE_GROUP( Header\ Files\\platform\\input\\leapMotion              FILES ${PLATFORMLEAPMOTION_HEADERS} )
ENDIF( USE_LEAPMOTIONINPUT )

IF( USE_OCULUSRIFT )
    SOURCE_GROUP( Header\ Files\\platform\\input\\oculusVR                FILES ${PLATFORMOCULUSVR_HEADERS} )
ENDIF( USE_OCULUSRIFT )	

IF( USE_RAZERHYDRAINPUT )
    SOURCE_GROUP( Header\ Files\\platform\\input\\razerHydra              FILES ${PLATFORMRAZERHYDRA_HEADERS} )
ENDIF( USE_RAZERHYDRAINPUT )

SOURCE_GROUP( Header\ Files\\platform\\menus                          FILES ${PLATFORMMENUS_HEADERS} )
SOURCE_GROUP( Header\ Files\\platform\\nativeDialogs                  FILES ${PLATFORMNATIVEDIALOGS_HEADERS} )
SOURCE_GROUP( Header\ Files\\platform\\output                         FILES ${PLATFORMOUTPUT_HEADERS} )
SOURCE_GROUP( Header\ Files\\platform\\threads                        FILES ${PLATFORMTHREADS_HEADERS} )
SOURCE_GROUP( Header\ Files\\platformWin32                            FILES ${PLATFORMWIN32_HEADERS} )
SOURCE_GROUP( Header\ Files\\platformWin32\\minidump                  FILES ${PLATFORMWIN32MINIDUMP_HEADERS} )
SOURCE_GROUP( Header\ Files\\platformWin32\\videoInfo                 FILES ${PLATFORMWIN32VIDEOINFO_HEADERS} )
SOURCE_GROUP( Header\ Files\\postFX                                   FILES ${POSTFX_HEADERS} )
SOURCE_GROUP( Header\ Files\\renderInstance                           FILES ${RENDERINSTANCE_HEADERS} )
SOURCE_GROUP( Header\ Files\\scene                                    FILES ${SCENE_HEADERS} )
SOURCE_GROUP( Header\ Files\\scene\\culling                           FILES ${SCENECULLING_HEADERS} )
SOURCE_GROUP( Header\ Files\\scene\\mixin                             FILES ${SCENEMIXIN_HEADERS} )
SOURCE_GROUP( Header\ Files\\scene\\zones                             FILES ${SCENEZONES_HEADERS} )
SOURCE_GROUP( Header\ Files\\sfx                                      FILES ${SFX_HEADERS} )
SOURCE_GROUP( Header\ Files\\sfx\\dsound                              FILES ${SFXDSOUND_HEADERS} )
IF( USE_FMODSOUNDSYSTEM )
    SOURCE_GROUP( Header\ Files\\sfx\\fmod                                FILES ${SFXFMOD_HEADERS} )
ENDIF( USE_FMODSOUNDSYSTEM )
SOURCE_GROUP( Header\ Files\\sfx\\media                               FILES ${SFXMEDIA_HEADERS} )
SOURCE_GROUP( Header\ Files\\sfx\\null                                FILES ${SFXNULL_HEADERS} )
SOURCE_GROUP( Header\ Files\\sfx\\openal                              FILES ${SFXOPENAL_HEADERS} )
SOURCE_GROUP( Header\ Files\\sfx\\xaudio                              FILES ${SFXXAUDIO_HEADERS} )
SOURCE_GROUP( Header\ Files\\shaderGen                                FILES ${SHADERGEN_HEADERS} )

IF( USE_OPENGL )
    SOURCE_GROUP( Header\ Files\\shaderGen\\GLSL                          FILES ${SHADERGENGLSL_HEADERS} )
ENDIF( USE_OPENGL )

SOURCE_GROUP( Header\ Files\\shaderGen\\HLSL                          FILES ${SHADERGENHLSL_HEADERS} )
SOURCE_GROUP( Header\ Files\\sim                                      FILES ${SIM_HEADERS} )
SOURCE_GROUP( Header\ Files\\T3D                                      FILES ${T3D_HEADERS} )
SOURCE_GROUP( Header\ Files\\T3D\\decal                               FILES ${T3DDECAL_HEADERS} )
SOURCE_GROUP( Header\ Files\\T3D\\examples                            FILES ${T3DEXAMPLES_HEADERS} )
SOURCE_GROUP( Header\ Files\\T3D\\fx                                  FILES ${T3DFX_HEADERS} )
SOURCE_GROUP( Header\ Files\\T3D\\gameBase                            FILES ${T3DGAMEBASE_HEADERS} )
IF( TORQUE_EXTENDED_MOVE )
    SOURCE_GROUP( Header\ Files\\T3D\\gameBase\\extended                  FILES ${T3DGAMEBASEEXTENDED_HEADERS} )
ENDIF( TORQUE_EXTENDED_MOVE )
IF( TORQUE_HIFI_NET )
    SOURCE_GROUP( Header\ Files\\T3D\\gameBase\\hifi                      FILES ${T3DGAMEBASEHIFI_HEADERS} )
ENDIF( TORQUE_HIFI_NET )
SOURCE_GROUP( Header\ Files\\T3D\\gameBase\\std                       FILES ${T3DGAMEBASESTD_HEADERS} )
SOURCE_GROUP( Header\ Files\\T3D\\physics                             FILES ${T3DGAMEBASEPHYSICS_HEADERS} )
IF( USE_BULLETPHYSICS )
    SOURCE_GROUP( Header\ Files\\T3D\\physics\\bullet                     FILES ${T3DGAMEBASEPHYSICSBULLET_HEADERS} )
ENDIF( USE_BULLETPHYSICS )
IF( USE_NVIDIAPHYSX )
    SOURCE_GROUP( Header\ Files\\T3D\\physics\\physx                      FILES ${T3DGAMEBASEPHYSICSPHYSX_HEADERS} )
ENDIF( USE_NVIDIAPHYSX )
# Dushan - this isn't in original project
#SOURCE_GROUP( Header\ Files\\T3D\\sceneComponent                      FILES ${T3DSCENECOMPONENT_HEADERS} )
SOURCE_GROUP( Header\ Files\\T3D\\sfx                                 FILES ${T3DSFX_HEADERS} )
SOURCE_GROUP( Header\ Files\\T3D\\turret                              FILES ${T3DTURRET_HEADERS} )
SOURCE_GROUP( Header\ Files\\T3D\\vehicles                            FILES ${T3DVEHICLES_HEADERS} )
SOURCE_GROUP( Header\ Files\\terrain                                  FILES ${TERRAIN_HEADERS} )

IF( USE_OPENGL )
    SOURCE_GROUP( Header\ Files\\terrain\\glsl                            FILES ${TERRAINGLSL_HEADERS} )
ENDIF( USE_OPENGL )	

SOURCE_GROUP( Header\ Files\\terrain\\hlsl                            FILES ${TERRAINHLSL_HEADERS} )
SOURCE_GROUP( Header\ Files\\ts                                       FILES ${TS_HEADERS} )
SOURCE_GROUP( Header\ Files\\ts\\arch                                 FILES ${TSARCH_HEADERS} )
SOURCE_GROUP( Header\ Files\\ts\\collada                              FILES ${TSCOLLADA_HEADERS} )
SOURCE_GROUP( Header\ Files\\ts\\loader                               FILES ${TSLOADER_HEADERS} )
SOURCE_GROUP( Header\ Files\\unit                                     FILES ${UNIT_HEADERS} )
SOURCE_GROUP( Header\ Files\\util                                     FILES ${UTIL_HEADERS} )
SOURCE_GROUP( Header\ Files\\util\\messaging                          FILES ${UTILMESSAGING_HEADERS} )
SOURCE_GROUP( Header\ Files\\windowManager                            FILES ${WINDOWSMANAGER_HEADERS} )
#SOURCE_GROUP( Header\ Files\\windowManager\\dedicated                 FILES ${WINDOWSMANAGERDEDICATED_HEADERS} )
SOURCE_GROUP( Header\ Files\\windowManager\\win32                     FILES ${WINDOWSMANAGERWIN32_HEADERS} )



#Dushan - Same thing for sources files
SOURCE_GROUP( Source\ Files\\app                                      FILES ${APP_SOURCES} )
SOURCE_GROUP( Source\ Files\\app\\net                                 FILES ${NET_SOURCES} )
SOURCE_GROUP( Source\ Files\\cinterface                               FILES ${CINT_SOURCES} )
SOURCE_GROUP( Source\ Files\\collision                                FILES ${COLL_SOURCES} )
SOURCE_GROUP( Source\ Files\\component                                FILES ${COMP_SOURCES} )
SOURCE_GROUP( Source\ Files\\console                                  FILES ${CONSOLE_SOURCES} )
SOURCE_GROUP( Source\ Files\\core                                     FILES ${CORE_SOURCES} )
SOURCE_GROUP( Source\ Files\\core\\ogg                                FILES ${COREOGG_SOURCES} )
SOURCE_GROUP( Source\ Files\\core\\stream                             FILES ${CORESTREAM_SOURCES} )
SOURCE_GROUP( Source\ Files\\core\\strings                            FILES ${CORESTRINGS_SOURCES} )
SOURCE_GROUP( Source\ Files\\core\\util                               FILES ${COREUTIL_SOURCES} )
SOURCE_GROUP( Source\ Files\\core\\util\\journal                      FILES ${COREUTILJOURNAL_SOURCES} )
SOURCE_GROUP( Source\ Files\\core\\util\\journal\\test                FILES ${COREUTILJOURNALTEST_SOURCES} )
SOURCE_GROUP( Source\ Files\\core\\util\\test                         FILES ${COREUTILTEST_SOURCES} )
SOURCE_GROUP( Source\ Files\\core\\zip                                FILES ${COREUTILTESTZIP_SOURCES} )
SOURCE_GROUP( Source\ Files\\core\\zip\\compressors                   FILES ${COREUTILZIPCOMPRESSORS_SOURCES} )
SOURCE_GROUP( Source\ Files\\core\\zip\\unitTest                      FILES ${COREUTILTESTZIPUNITTESTS_SOURCES} )
SOURCE_GROUP( Source\ Files\\environment                              FILES ${ENVIRONMENT_SOURCES} )
SOURCE_GROUP( Source\ Files\\environment\\editors                     FILES ${ENVIRONMENTEDITOR_SOURCES} )
SOURCE_GROUP( Source\ Files\\forest                                   FILES ${FOREST_SOURCES} )
SOURCE_GROUP( Source\ Files\\forest\\editor                           FILES ${FORESTEDITOR_SOURCES} )

IF( USE_OPENGL )
    SOURCE_GROUP( Source\ Files\\forest\\glsl                             FILES ${FORESTGLSL_SOURCES} )
ENDIF( USE_OPENGL )

SOURCE_GROUP( Source\ Files\\forest\\hlsl                             FILES ${FORESTHLSL_SOURCES} )
SOURCE_GROUP( Source\ Files\\forest\\ts                               FILES ${FORESTTS_SOURCES} )
SOURCE_GROUP( Source\ Files\\gfx                                      FILES ${GFX_SOURCES} )
SOURCE_GROUP( Source\ Files\\gfx\\bitmap                              FILES ${GFXBITMAP_SOURCES} )
SOURCE_GROUP( Source\ Files\\gfx\\bitmap\\loader                      FILES ${GFXBITMAPLOADER_SOURCES} )
SOURCE_GROUP( Source\ Files\\gfx\\d3d9                                FILES ${D3D9_SOURCES} )
SOURCE_GROUP( Source\ Files\\gfx\\d3d9\\pc                            FILES ${D3D9PC_SOURCES} )

IF( USE_OPENGL )
    SOURCE_GROUP( Source\ Files\\gfx\\gl                                  FILES ${GL_SOURCES} )
	SOURCE_GROUP( Source\ Files\\gfx\\gl\\ggl                             FILES ${GLGGL_SOURCES} )
	SOURCE_GROUP( Source\ Files\\gfx\\gl\\ggl\\win32                      FILES ${GLGGLWIN32_SOURCES} )
ENDIF( USE_OPENGL )

SOURCE_GROUP( Source\ Files\\gfx\\Null                                FILES ${GFXNULL_SOURCES} )
SOURCE_GROUP( Source\ Files\\gfx\\sim                                 FILES ${GFXSIM_SOURCES} )
SOURCE_GROUP( Source\ Files\\gfx\\test                                FILES ${GFXTEST_SOURCES} )
SOURCE_GROUP( Source\ Files\\gfx\\util                                FILES ${GFXUTIL_SOURCES} )
SOURCE_GROUP( Source\ Files\\gfx\\video                               FILES ${GFXVIDEO_SOURCES} )
SOURCE_GROUP( Source\ Files\\gui\\3d                                  FILES ${GUI3D_SOURCES} )
SOURCE_GROUP( Source\ Files\\gui\\buttons                             FILES ${GUIBUTTONS_SOURCES} )
SOURCE_GROUP( Source\ Files\\gui\\containers                          FILES ${GUICONTAINERS_SOURCES} )
SOURCE_GROUP( Source\ Files\\gui\\controls                            FILES ${GUICONTROLS_SOURCES} )
SOURCE_GROUP( Source\ Files\\gui\\core                                FILES ${GUICORE_SOURCES} )
SOURCE_GROUP( Source\ Files\\gui\\editor                              FILES ${GUIEDITOR_SOURCES} )
SOURCE_GROUP( Source\ Files\\gui\\editor\\inspector                   FILES ${GUIEDITORINSPECTOR_SOURCES} )
SOURCE_GROUP( Source\ Files\\gui\\game                                FILES ${GUIGAME_SOURCES} )
SOURCE_GROUP( Source\ Files\\gui\\shiny                               FILES ${GUISHINY_SOURCES} )
SOURCE_GROUP( Source\ Files\\gui\\theora                              FILES ${GUITHEORA_SOURCES} )
SOURCE_GROUP( Source\ Files\\gui\\utility                             FILES ${GUIUTILITY_SOURCES} )
SOURCE_GROUP( Source\ Files\\gui\\worldEditor                         FILES ${GUIWORLDEDITOR_SOURCES} )
SOURCE_GROUP( Source\ Files\\i18n                                     FILES ${I18N_SOURCES} )
SOURCE_GROUP( Source\ Files\\lighting                                 FILES ${LIGHTING_SOURCES} )
SOURCE_GROUP( Source\ Files\\lighting\\advanced                       FILES ${LIGHTINGADVANCED_SOURCES} )

IF( USE_OPENGL )
    SOURCE_GROUP( Source\ Files\\lighting\\advanced\\glsl                 FILES ${LIGHTINGADVANCEDGLSL_SOURCES} )
ENDIF( USE_OPENGL )

SOURCE_GROUP( Source\ Files\\lighting\\advanced\\hlsl                 FILES ${LIGHTINGADVANCEDHLSL_SOURCES} )
SOURCE_GROUP( Source\ Files\\lighting\\basic                          FILES ${LIGHTINGBASIC_SOURCES} )
SOURCE_GROUP( Source\ Files\\lighting\\common                         FILES ${LIGHTINGCOMMON_SOURCES} )
SOURCE_GROUP( Source\ Files\\lighting\\shadowMap                      FILES ${LIGHTINGSHADOWMAP_SOURCES} )
SOURCE_GROUP( Source\ Files\\materials                                FILES ${MATERIALS_SOURCES} )
SOURCE_GROUP( Source\ Files\\math                                     FILES ${MATH_SOURCES} )
SOURCE_GROUP( Source\ Files\\math\\test                               FILES ${MATHTEST_SOURCES} )
SOURCE_GROUP( Source\ Files\\math\\util                               FILES ${MATHUTIL_SOURCES} )

IF( USE_RECASTNAVITATION )
    SOURCE_GROUP( Header\ Files\\navigation                               FILES ${RECASTNAVITATION_SOURCES} )
ENDIF( USE_RECASTNAVITATION )

SOURCE_GROUP( Source\ Files\\platform                                 FILES ${PLATFORM_SOURCES} )
SOURCE_GROUP( Source\ Files\\platform\\async                          FILES ${PLATFORMASYNC_SOURCES} )
SOURCE_GROUP( Source\ Files\\platform\\input                          FILES ${PLATFORMINPUT_SOURCES} )

IF( USE_LEAPMOTIONINPUT )
    SOURCE_GROUP( Source\ Files\\platform\\input\\leapMotion              FILES ${PLATFORMLEAPMOTION_SOURCES} )
ENDIF( USE_LEAPMOTIONINPUT )

IF( USE_OCULUSRIFT )
    SOURCE_GROUP( Source\ Files\\platform\\input\\oculusVR                FILES ${PLATFORMOCULUSVR_SOURCES} )
ENDIF( USE_OCULUSRIFT )

IF( USE_RAZERHYDRAINPUT )
    SOURCE_GROUP( Source\ Files\\platform\\input\\razerHydra              FILES ${PLATFORMRAZERHYDRA_SOURCES} )
ENDIF( USE_RAZERHYDRAINPUT )

SOURCE_GROUP( Source\ Files\\platform\\menus                          FILES ${PLATFORMMENUS_SOURCES} )
SOURCE_GROUP( Source\ Files\\platform\\nativeDialogs                  FILES ${PLATFORMNATIVEDIALOGS_SOURCES} )
SOURCE_GROUP( Source\ Files\\platform\\test                           FILES ${PLATFORMTEST_SOURCES} )
SOURCE_GROUP( Source\ Files\\platform\\threads                        FILES ${PLATFORMTHREADS_SOURCES} )
SOURCE_GROUP( Source\ Files\\platformWin32                            FILES ${PLATFORMWIN32_SOURCES} )
SOURCE_GROUP( Source\ Files\\platformWin32\\menus                     FILES ${PLATFORMWIN32MENUS_SOURCES} )
SOURCE_GROUP( Source\ Files\\platformWin32\\minidump                  FILES ${PLATFORMWIN32MINIDUMP_SOURCES} )
SOURCE_GROUP( Source\ Files\\platformWin32\\nativeDialogs             FILES ${PLATFORMWIN32NATIVEDIALOGS_SOURCES} )
SOURCE_GROUP( Source\ Files\\platformWin32\\threads                   FILES ${PLATFORMWIN32THREADS_SOURCES} )
SOURCE_GROUP( Source\ Files\\platformWin32\\videoInfo                 FILES ${PLATFORMWIN32VIDEOINFO_SOURCES} )
SOURCE_GROUP( Source\ Files\\postFX                                   FILES ${POSTFX_SOURCES} )
SOURCE_GROUP( Source\ Files\\renderInstance                           FILES ${RENDERINSTANCE_SOURCES} )
SOURCE_GROUP( Source\ Files\\scene                                    FILES ${SCENE_SOURCES} )
SOURCE_GROUP( Source\ Files\\scene\\culling                           FILES ${SCENECULLING_SOURCES} )
SOURCE_GROUP( Source\ Files\\scene\\zones                             FILES ${SCENEZONES_SOURCES} )
SOURCE_GROUP( Source\ Files\\sfx                                      FILES ${SFX_SOURCES} )
SOURCE_GROUP( Source\ Files\\sfx\\dsound                              FILES ${SFXDSOUND_SOURCES} )

IF( USE_FMODSOUNDSYSTEM )
    SOURCE_GROUP( Source\ Files\\sfx\\fmod                                FILES ${SFXFMOD_SOURCES} )
ENDIF( USE_FMODSOUNDSYSTEM )

SOURCE_GROUP( Source\ Files\\sfx\\media                               FILES ${SFXMEDIA_SOURCES} )
SOURCE_GROUP( Source\ Files\\sfx\\null                                FILES ${SFXNULL_SOURCES} )
SOURCE_GROUP( Source\ Files\\sfx\\openal                              FILES ${SFXOPENAL_SOURCES} )
SOURCE_GROUP( Source\ Files\\sfx\\openal\\win32                       FILES ${SFXOPENALWIN32_SOURCES} )
SOURCE_GROUP( Source\ Files\\sfx\\xaudio                              FILES ${SFXXAUDIO_SOURCES} )
SOURCE_GROUP( Source\ Files\\shaderGen                                FILES ${SHADERGEN_SOURCES} )

IF( USE_OPENGL )
    SOURCE_GROUP( Source\ Files\\shaderGen\\GLSL                          FILES ${SHADERGENGLSL_SOURCES} )
ENDIF( USE_OPENGL )

SOURCE_GROUP( Source\ Files\\shaderGen\\HLSL                          FILES ${SHADERGENHLSL_SOURCES} )
SOURCE_GROUP( Source\ Files\\sim                                      FILES ${SIM_SOURCES} )
SOURCE_GROUP( Source\ Files\\T3D                                      FILES ${T3D_SOURCES} )
SOURCE_GROUP( Source\ Files\\T3D\\decal                               FILES ${T3DDECAL_SOURCES} )
SOURCE_GROUP( Source\ Files\\T3D\\examples                            FILES ${T3DEXAMPLES_SOURCES} )
SOURCE_GROUP( Source\ Files\\T3D\\fps                                 FILES ${T3DFPS_SOURCES} )
SOURCE_GROUP( Source\ Files\\T3D\\fx                                  FILES ${T3DFX_SOURCES} )
SOURCE_GROUP( Source\ Files\\T3D\\gameBase                            FILES ${T3DGAMEBASE_SOURCES} )

IF( TORQUE_EXTENDED_MOVE )
    SOURCE_GROUP( Source\ Files\\T3D\\gameBase\\extended                  FILES ${T3DGAMEBASEEXTENDED_SOURCES} )
ENDIF( TORQUE_EXTENDED_MOVE )

IF( TORQUE_HIFI_NET )
    SOURCE_GROUP( Source\ Files\\T3D\\gameBase\\hifi                      FILES ${T3DGAMEBASEHIFI_SOURCES} )
ENDIF( TORQUE_HIFI_NET )

SOURCE_GROUP( Source\ Files\\T3D\\gameBase\\std                       FILES ${T3DGAMEBASESTD_SOURCES} )
SOURCE_GROUP( Source\ Files\\T3D\\physics                                 FILES ${T3DGAMEBASEPHYSICS_SOURCES} )

IF( USE_BULLETPHYSICS )
    SOURCE_GROUP( Source\ Files\\T3D\\physics\\bullet                     FILES ${T3DGAMEBASEPHYSICSBULLET_SOURCES} )
ENDIF( USE_BULLETPHYSICS )

IF( USE_NVIDIAPHYSX )
    SOURCE_GROUP( Source\ Files\\T3D\\physics\\physx                      FILES ${T3DGAMEBASEPHYSICSPHYSX_SOURCES} )
ENDIF( USE_NVIDIAPHYSX )

# Dushan - this isn't in original project
#SOURCE_GROUP( Source\ Files\\T3D\\sceneComponent                      FILES ${T3DSCENECOMPONENT_SOURCES} )
SOURCE_GROUP( Source\ Files\\T3D\\sfx                                 FILES ${T3DSFX_SOURCES} )
SOURCE_GROUP( Source\ Files\\T3D\\turret                              FILES ${T3DTURRET_SOURCES} )
SOURCE_GROUP( Source\ Files\\T3D\\vehicles                            FILES ${T3DVEHICLES_SOURCES} )
SOURCE_GROUP( Source\ Files\\terrain                                  FILES ${TERRAIN_SOURCES} )

IF( USE_OPENGL )
    SOURCE_GROUP( Source\ Files\\terrain\\glsl                            FILES ${TERRAINGLSL_SOURCES} )
ENDIF( USE_OPENGL )	

SOURCE_GROUP( Source\ Files\\terrain\\hlsl                            FILES ${TERRAINHLSL_SOURCES} )
SOURCE_GROUP( Source\ Files\\ts                                       FILES ${TS_SOURCES} )
SOURCE_GROUP( Source\ Files\\ts\\arch                                 FILES ${TSARCH_SOURCES} )
SOURCE_GROUP( Source\ Files\\ts\\collada                              FILES ${TSCOLLADA_SOURCES} )
SOURCE_GROUP( Source\ Files\\ts\\loader                               FILES ${TSLOADER_SOURCES} )
SOURCE_GROUP( Source\ Files\\unit                                     FILES ${UNIT_SOURCES} )
SOURCE_GROUP( Source\ Files\\unit\\tests                              FILES ${UNITTESTS_SOURCES} )
SOURCE_GROUP( Source\ Files\\util                                     FILES ${UTIL_SOURCES} )
SOURCE_GROUP( Source\ Files\\util\\messaging                          FILES ${UTILMESSAGING_SOURCES} )
SOURCE_GROUP( Source\ Files\\windowManager                            FILES ${WINDOWSMANAGER_SOURCES} )
#SOURCE_GROUP( Source\ Files\\windowManager\\dedicated                 FILES ${WINDOWSMANAGERDEDICATED_SOURCES} )
SOURCE_GROUP( Source\ Files\\windowManager\\test                      FILES ${WINDOWSMANAGERTEST_SOURCES} )
SOURCE_GROUP( Source\ Files\\windowManager\\win32                     FILES ${WINDOWSMANAGERWIN32_SOURCES} )

ADD_CUSTOM_COMMAND(OUTPUT mMath_ASM.obj
    COMMAND ${CMAKE_SOURCE_DIR}/Engine/bin/nasm/nasm.exe -f win32 ${CMAKE_SOURCE_DIR}/Engine/source/math/mMath_ASM.asm -o mMath_ASM.obj
    DEPENDS engine/source/math/mMath_ASM.asm)
ADD_CUSTOM_COMMAND(OUTPUT mMathAMD_ASM.obj
    COMMAND ${CMAKE_SOURCE_DIR}/Engine/bin/nasm/nasm.exe -f win32 ${CMAKE_SOURCE_DIR}/Engine/source/math/mMathAMD_ASM.asm -o mMathAMD_ASM.obj
    DEPENDS engine/source/math/mMathAMD_ASM.asm)
ADD_CUSTOM_COMMAND(OUTPUT mMathSSE_ASM.obj
    COMMAND ${CMAKE_SOURCE_DIR}/Engine/bin/nasm/nasm.exe -f win32 ${CMAKE_SOURCE_DIR}/Engine/source/math/mMathSSE_ASM.asm -o mMathSSE_ASM.obj
    DEPENDS engine/source/math/mMathSSE_ASM.asm)
ADD_CUSTOM_COMMAND(OUTPUT platformCPUInfo.obj
    COMMAND ${CMAKE_SOURCE_DIR}/Engine/bin/nasm/nasm.exe -f win32 ${CMAKE_SOURCE_DIR}/Engine/source/platform/platformCPUInfo.asm -o platformCPUInfo.obj
    DEPENDS engine/source/platform/platformCPUInfo.asm)


ADD_LIBRARY( dll SHARED 
    ${APP_HEADERS} 
    ${APP_SOURCES} 
    ${NET_HEADERS} 
    ${NET_SOURCES} 
    ${CINT_HEADERS} 
    ${CINT_SOURCES} 
    ${COLL_HEADERS} 
    ${COLL_SOURCES} 
    ${COMPINT_HEADERS} 
    ${COMP_HEADERS} 
    ${COMP_SOURCES} 
    ${CONSOLE_HEADERS} 
    ${CONSOLE_SOURCES} 
	${CORE_HEADERS}
	${CORE_SOURCES}
	${COREOGG_HEADERS}
	${COREOGG_SOURCES}
	${CORESTREAM_HEADERS}
	${CORESTREAM_SOURCES}
	${CORESTRINGS_HEADERS}
	${CORESTRINGS_SOURCES}
	${COREUTIL_HEADERS}
	${COREUTIL_SOURCES}	
	${COREUTILJOURNAL_HEADERS}
	${COREUTILJOURNAL_SOURCES}
	${COREUTILJOURNALTEST_SOURCES}
	${COREUTILTEST_SOURCES}
	${COREUTILTESTZIP_SOURCES}
	${COREUTILZIPCOMPRESSORS_SOURCES}
	${COREUTILTESTZIPUNITTESTS_HEADERS}
	${COREUTILTESTZIPUNITTESTS_SOURCES}
	${ENVIRONMENT_HEADERS}
	${ENVIRONMENT_SOURCES}
	${ENVIRONMENTEDITOR_HEADERS}
	${ENVIRONMENTEDITOR_SOURCES}
	${FOREST_HEADERS}
	${FOREST_SOURCES}
	${FORESTEDITOR_HEADERS}
	${FORESTEDITOR_SOURCES}
	${FORESTGLSL_HEADERS}
	${FORESTGLSL_SOURCES}	
	${FORESTHLSL_HEADERS}
	${FORESTHLSL_SOURCES}
	${FORESTTS_HEADERS}
	${FORESTTS_SOURCES}
	${GFX_HEADERS}
	${GFX_SOURCES}
	${GFXBITMAP_HEADERS}
	${GFXBITMAP_SOURCES}	
	${GFXBITMAPLOADER_SOURCES}
	${D3D9_HEADERS}
	${D3D9_SOURCES}
	${D3D9PC_HEADERS}
	${D3D9PC_SOURCES}
	${GL_HEADERS}
	${GL_SOURCES}
	${GLGGL_HEADERS}
	${GLGGL_SOURCES}
	${GLGGLWIN32_HEADERS}
	${GLGGLWIN32_SOURCES}
	${GFXNULL_HEADERS}
	${GFXNULL_SOURCES}
	${GFXSIM_HEADERS}
	${GFXSIM_SOURCES}
	${GFXTEST_SOURCES}
	${GFXUTIL_HEADERS}
	${GFXUTIL_SOURCES}
	${GFXVIDEO_HEADERS}
	${GFXVIDEO_SOURCES}	
	${GUI3D_HEADERS}
	${GUI3D_SOURCES}
	${GUIBUTTONS_HEADERS}
	${GUIBUTTONS_SOURCES}
	${GUICONTAINERS_HEADERS}
	${GUICONTAINERS_SOURCES}
	${GUICONTROLS_HEADERS}
	${GUICONTROLS_SOURCES}
	${GUICORE_HEADERS}
	${GUICORE_SOURCES}	
	${GUIEDITOR_HEADERS}
	${GUIEDITOR_SOURCES}
	${GUIEDITORINSPECTOR_HEADERS}
	${GUIEDITORINSPECTOR_SOURCES}
	${GUIGAME_HEADERS}
	${GUIGAME_SOURCES}
	${GUISHINY_HEADERS}
	${GUISHINY_SOURCES}
	${GUITHEORA_HEADERS}
	${GUITHEORA_SOURCES}
	${GUIUTILITY_HEADERS}
	${GUIUTILITY_SOURCES}
	${GUIWORLDEDITOR_HEADERS}
	${GUIWORLDEDITOR_SOURCES}
	${I18N_HEADERS}
	${I18N_SOURCES}	
	${LIGHTING_HEADERS}
	${LIGHTING_SOURCES}
	${LIGHTINGADVANCED_HEADERS}
	${LIGHTINGADVANCED_SOURCES}
	${LIGHTINGADVANCEDGLSL_HEADERS}
	${LIGHTINGADVANCEDGLSL_SOURCES}
	${LIGHTINGADVANCEDHLSL_HEADERS}
	${LIGHTINGADVANCEDHLSL_SOURCES}
	${LIGHTINGBASIC_HEADERS}
	${LIGHTINGBASIC_SOURCES}	
	${LIGHTINGCOMMON_HEADERS}
	${LIGHTINGCOMMON_SOURCES}
	${LIGHTINGSHADOWMAP_HEADERS}
	${LIGHTINGSHADOWMAP_SOURCES}
	${MATERIALS_HEADERS}
	${MATERIALS_SOURCES}
	${MATH_HEADERS}
	${MATH_SOURCES}
	${MATHTEST_HEADERS}
	${MATHTEST_SOURCES}	
	${MATHUTIL_HEADERS}
	${MATHUTIL_SOURCES}
	${RECASTNAVITATION_HEADERS}
	${RECASTNAVITATION_SOURCES}
	${PLATFORM_HEADERS}
	${PLATFORM_SOURCES}
	${PLATFORMASYNC_HEADERS}
	${PLATFORMASYNC_SOURCES}
	${PLATFORMINPUT_HEADERS}
	${PLATFORMINPUT_SOURCES}
	${PLATFORMLEAPMOTION_HEADERS}
	${PLATFORMLEAPMOTION_SOURCES}
	${PLATFORMOCULUSVR_HEADERS}
	${PLATFORMOCULUSVR_SOURCES}
	${PLATFORMRAZERHYDRA_HEADERS}
	${PLATFORMRAZERHYDRA_SOURCES}
	${PLATFORMMENUS_HEADERS}
	${PLATFORMMENUS_SOURCES}
	${PLATFORMNATIVEDIALOGS_HEADERS}
	${PLATFORMNATIVEDIALOGS_SOURCES}
	${PLATFORMOUTPUT_HEADERS}
	${PLATFORMTEST_SOURCES}
	${PLATFORMTHREADS_HEADERS}
	${PLATFORMTHREADS_SOURCES}
	${PLATFORMWIN32_HEADERS}
	${PLATFORMWIN32_SOURCES}
	${PLATFORMWIN32MENUS_SOURCES}
	${PLATFORMWIN32MINIDUMP_HEADERS}
	${PLATFORMWIN32MINIDUMP_SOURCES}
	${PLATFORMWIN32NATIVEDIALOGS_SOURCES}
	${PLATFORMWIN32THREADS_SOURCES}
	${PLATFORMWIN32VIDEOINFO_HEADERS}
	${PLATFORMWIN32VIDEOINFO_SOURCES}
	${POSTFX_HEADERS}
	${POSTFX_SOURCES}
	${RENDERINSTANCE_HEADERS}
	${RENDERINSTANCE_SOURCES}
	${SCENE_HEADERS}
	${SCENE_SOURCES}
	${SCENECULLING_HEADERS}
	${SCENECULLING_SOURCES}
	${SCENEMIXIN_HEADERS}
	${SCENEZONES_HEADERS}
	${SCENEZONES_SOURCES}
	${SFX_HEADERS}
	${SFX_SOURCES}
	${SFXDSOUND_HEADERS}
	${SFXDSOUND_SOURCES}
	${SFXFMOD_HEADERS}
	${SFXFMOD_SOURCES}
	${SFXMEDIA_HEADERS}
	${SFXMEDIA_SOURCES}
	${SFXNULL_HEADERS}
	${SFXNULL_SOURCES}
	${SFXOPENAL_HEADERS}
	${SFXOPENAL_SOURCES}
	${SFXOPENALWIN32_SOURCES}
	${SFXXAUDIO_HEADERS}
	${SFXXAUDIO_SOURCES}
	${SHADERGEN_HEADERS}
	${SHADERGEN_SOURCES}
	${SHADERGENGLSL_HEADERS}
	${SHADERGENGLSL_SOURCES}
	${SHADERGENHLSL_HEADERS}
	${SHADERGENHLSL_SOURCES}
	${SIM_HEADERS}
	${SIM_SOURCES}
	${T3D_HEADERS}
	${T3D_SOURCES}
	${T3DDECAL_HEADERS}
	${T3DDECAL_SOURCES}
	${T3DEXAMPLES_HEADERS}
	${T3DEXAMPLES_SOURCES}
	${T3DFPS_SOURCES}
	${T3DFX_HEADERS}
	${T3DFX_SOURCES}
	${T3DGAMEBASE_HEADERS}
	${T3DGAMEBASE_SOURCES}
	${T3DGAMEBASEEXTENDED_HEADERS}
	${T3DGAMEBASEEXTENDED_SOURCES}
	${T3DGAMEBASEHIFI_HEADERS}
	${T3DGAMEBASEHIFI_SOURCES}
	${T3DGAMEBASESTD_HEADERS}
	${T3DGAMEBASESTD_SOURCES}
	${T3DGAMEBASEPHYSICS_HEADERS}
	${T3DGAMEBASEPHYSICS_SOURCES}
	${T3DGAMEBASEPHYSICSBULLET_HEADERS}
	${T3DGAMEBASEPHYSICSBULLET_SOURCES}
	${T3DGAMEBASEPHYSICSPHYSX_HEADERS}
	${T3DGAMEBASEPHYSICSPHYSX_SOURCES}
	#${T3DSCENECOMPONENT_HEADERS}
	#${T3DSCENECOMPONENT_SOURCES}
	${T3DSFX_HEADERS}
	${T3DSFX_SOURCES}
	${T3DTURRET_HEADERS}
	${T3DTURRET_SOURCES}
	${T3DVEHICLES_HEADERS}
	${T3DVEHICLES_SOURCES}
	${TERRAIN_HEADERS}
	${TERRAIN_SOURCES}
	${TERRAINGLSL_HEADERS}
	${TERRAINGLSL_SOURCES}
	${TERRAINHLSL_HEADERS}
	${TERRAINHLSL_SOURCES}
	${TS_HEADERS}
	${TS_SOURCES}
	${TSARCH_HEADERS}
	${TSARCH_SOURCES}
	${TSCOLLADA_HEADERS}
	${TSCOLLADA_SOURCES}
	${TSLOADER_HEADERS}
	${TSLOADER_SOURCES}
	${UNIT_HEADERS}
	${UNIT_SOURCES}
	${UNITTESTS_SOURCES}
	${UTIL_HEADERS}
	${UTIL_SOURCES}
	${UTILMESSAGING_HEADERS}
	${UTILMESSAGING_SOURCES}
	${WINDOWSMANAGER_HEADERS}
	${WINDOWSMANAGER_SOURCES}
#	${WINDOWSMANAGERDEDICATED_HEADERS}
#	${WINDOWSMANAGERDEDICATED_SOURCES}
	${WINDOWSMANAGERTEST_SOURCES}
	${WINDOWSMANAGERWIN32_HEADERS}
	${WINDOWSMANAGERWIN32_SOURCES}
)

#Dushan - Add library modules provided by GarageGames
ADD_SUBDIRECTORY( ${ENGINE_DIR}/lib )

# Dushan
IF( WIN32 )
    FIND_PACKAGE( DirectX REQUIRED )
	
    INCLUDE_DIRECTORIES( dll ${DXSDK_INCLUDE_DIR} )
    TARGET_LINK_LIBRARIES( dll ${DXSDK_d3d9_LIBRARY} ${DXSDK_d3dx9_LIBRARY} ${DXSDK_DxErr_LIBRARY} ${DXSDK_dxgi_LIBRARY} ${DXSDK_x3daudio_LIBRARY} )
ENDIF( WIN32 )

#Dushan - not finish
IF( USE_BULLETPHYSICS )
    #FIND_PACKAGE( Bullet REQUIRED )
	
	INCLUDE_DIRECTORIES( dll ${CMAKE_SOURCE_DIR}/Engine/lib/bullet/src )
	TARGET_LINK_LIBRARIES( dll MiniCL BulletMultiThreaded BulletDynamics BulletCollision BulletSoftBody LinearMath)
ENDIF( USE_BULLETPHYSICS )

IF( USE_NVIDIAPHYSX )
    FIND_PACKAGE( PhysX REQUIRED )
	
	INCLUDE_DIRECTORIES( dll ${PHYSX_INCLUDE_DIRS} )
	TARGET_LINK_LIBRARIES( dll ${PHYSX_LIBRARIES} )
	#Dushan - generate and include nxuStream2 as dependence library if there is nVidia Physx SDK and if USE_NVIDIAPHYSX is defined
	TARGET_LINK_LIBRARIES( dll nxuStream2 nxuCharacter )
	ADD_DEFINITIONS( -DTORQUE_PHYSICS_PHYSX -DTORQUE_PHYSICS_ENABLED )
ENDIF( USE_NVIDIAPHYSX )

#Dushan - should we make this as user choice?
FIND_PACKAGE( OpenAL REQUIRED )
INCLUDE_DIRECTORIES(dll  ${OPENAL_INCLUDE_DIR} )
TARGET_LINK_LIBRARIES( dll ${OPENAL_LIBRARY} )

IF( USE_FMODSOUNDSYSTEM )
	FIND_PACKAGE( Fmod_Event REQUIRED )
	
	INCLUDE_DIRECTORIES( dll ${FMOD_INCLUDE_DIRS} ${FMOD_EVENT_INCLUDE_DIR} )
	TARGET_LINK_LIBRARIES( dll ${FMOD_LIBRARY} ${FMOD_EVENT_LIBRARY} )
ENDIF( USE_FMODSOUNDSYSTEM )

IF( USE_LEAPMOTIONINPUT )
    FIND_PACKAGE( LeapMotion REQUIRED )
	
	INCLUDE_DIRECTORIES( dll ${LEAP_INCLUDE_DIR} )
	TARGET_LINK_LIBRARIES( dll ${LEAP_LIBRARY} )
ENDIF( USE_LEAPMOTIONINPUT )

IF( USE_OCULUSRIFT )
    FIND_PACKAGE( OculusSDK REQUIRED )
	
	INCLUDE_DIRECTORIES( dll ${OCULUS_SDK_INCLUDE_DIRS} )
	TARGET_LINK_LIBRARIES( dll ${OCULUS_SDK_LIBRARIES} )
ENDIF( USE_OCULUSRIFT )

IF( USE_RAZERHYDRAINPUT )
    FIND_PACKAGE( Sixence REQUIRED )
	
	INCLUDE_DIRECTORIES( dll ${SIXSENCE_INCLUDE_DIR} )
	TARGET_LINK_LIBRARIES( dll ${SIXSENCE_LIBRARY} )
ENDIF( USE_RAZERHYDRAINPUT )

IF( USE_RECASTNAVITATION )
    
	TARGET_LINK_LIBRARIES( dll DebugUtils Detour DetourCrowd DetourTileCache Recast )
	INCLUDE_DIRECTORIES( dll ${ENGINE_DIR}/lib/recast/DebugUtils/Include ${ENGINE_DIR}/lib/recast/Recast/Include ${ENGINE_DIR}/lib/recast/Detour/Include ${ENGINE_DIR}/lib/recast/DetourTileCache/Include ${ENGINE_DIR}/lib/recast/DetourCrowd/Include )
	
ENDIF( USE_RECASTNAVITATION )

#Dushan - TODO Unix
#FIND_PACKAGE( OGG REQUIRED )
#FIND_PACKAGE( Theora REQUIRED )
#FIND_PACKAGE( TinyXML REQUIRED )
#FIND_PACKAGE( Vorbis REQUIRED )
#FIND_PACKAGE( VorbisFile REQUIRED )

IF( WIN32 AND MSVC )
    #Torque3D.dll
    TARGET_LINK_LIBRARIES( dll winmm ws2_32 )
	# Select flags.
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DTORQUE_DEBUG -DTORQUE_DEBUG_GUARD -DD3D_DEBUG_INFO -DNDEBUG /MD")
    SET(CMAKE_CXX_FLAGS_RELEASE "-O2 /TP /MD")
    SET(CMAKE_CXX_FLAGS_DEBUG  "-DTORQUE_DEBUG -DTORQUE_DEBUG_GUARD -DD3D_DEBUG_INFO -DDEBUG /MDd")	
    ADD_DEFINITIONS( -DTORQUE_SHADERGEN -DTORQUE_UNICODE -DTORQUE_SHARED -DBAN_OPCODE_AUTOLINK -DICE_NO_DLL -DTORQUE_OPCODE -DLTC_NO_PROTOTYPES -DTORQUE_ADVANCED_LIGHTING -DTORQUE_BASIC_LIGHTING -DTORQUE_COLLADA -DDOM_INCLUDE_TINYXML -DPCRE_STATIC -DUNICODE -DINITGUID -D_CRT_SECURE_NO_DEPRECATE -DTORQUE_NET_STATS -D_CRT_SECURE_NO_WARNINGS -DNDEBUG)
    INCLUDE_DIRECTORIES( dll ${ENGINE_DIR}/source ${ENGINE_DIR}/source/Torque3D ${ENGINE_DIR}/lib ${ENGINE_DIR}/lib/libogg/include ${ENGINE_DIR}/lib/libtheora/include ${ENGINE_DIR}/lib/libvorbis/include ${ENGINE_DIR}/lib/TinyXML ${ENGINE_DIR}/lib/collada/include ${ENGINE_DIR}/lib/collada/include/1.4 ${ENGINE_DIR}/lib/zlib ${ENGINE_DIR}/lib/ljpeg ${ENGINE_DIR}/lib/squish )
    TARGET_LINK_LIBRARIES( dll collada_dom convexDecomp libogg libtheora libvorbis ljpeg lmng lpng lungif opcode pcre squish tinyxml zlib )
    SET_TARGET_PROPERTIES( dll PROPERTIES OUTPUT_NAME "Torque3D" )
    SET_TARGET_PROPERTIES( dll PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} /EHsc /Oi /GF /GS- /Gy /openmp /W4 /wd4018 /wd4100 /wd4121 /wd4127 /wd4130 /wd4244 /wd4245 /wd4389 /wd4511 /wd4512 /wd4800 /Zc:wchar_t-")
    SET_TARGET_PROPERTIES( dll PROPERTIES LINK_FLAGS "${LINK_FLAGS} /DYNAMICBASE:NO /NXCOMPAT:NO /SUBSYSTEM:WINDOWS /INCREMENTAL /NODEFAULTLIB:LIBC /NODEFAULTLIB:LIBCD /SAFESEH:NO /DEF:${ENGINE_DIR}/lib/Torque3D/msvc/Torque3d.def")
	
	#Torque.exe
    ADD_EXECUTABLE( exe WIN32 engine/source/main/main.cpp "${CMAKE_SOURCE_DIR}/Engine/source/Torque3D/Torque.rc" )
    TARGET_LINK_LIBRARIES( exe dll )
    SET_TARGET_PROPERTIES( exe PROPERTIES OUTPUT_NAME "Torque3D" )

ENDIF( WIN32 AND MSVC )

IF(UNIX)
#Dushan TODO
ENDIF(UNIX)