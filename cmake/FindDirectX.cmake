#
#     Copyright (C) Pixar. All rights reserved.
#
#     This license governs use of the accompanying software. If you
#     use the software, you accept this license. If you do not accept
#     the license, do not use the software.
#
#     1. Definitions
#     The terms "reproduce," "reproduction," "derivative works," and
#     "distribution" have the same meaning here as under U.S.
#     copyright law.  A "contribution" is the original software, or
#     any additions or changes to the software.
#     A "contributor" is any person or entity that distributes its
#     contribution under this license.
#     "Licensed patents" are a contributor's patent claims that read
#     directly on its contribution.
#
#     2. Grant of Rights
#     (A) Copyright Grant- Subject to the terms of this license,
#     including the license conditions and limitations in section 3,
#     each contributor grants you a non-exclusive, worldwide,
#     royalty-free copyright license to reproduce its contribution,
#     prepare derivative works of its contribution, and distribute
#     its contribution or any derivative works that you create.
#     (B) Patent Grant- Subject to the terms of this license,
#     including the license conditions and limitations in section 3,
#     each contributor grants you a non-exclusive, worldwide,
#     royalty-free license under its licensed patents to make, have
#     made, use, sell, offer for sale, import, and/or otherwise
#     dispose of its contribution in the software or derivative works
#     of the contribution in the software.
#
#     3. Conditions and Limitations
#     (A) No Trademark License- This license does not grant you
#     rights to use any contributor's name, logo, or trademarks.
#     (B) If you bring a patent claim against any contributor over
#     patents that you claim are infringed by the software, your
#     patent license from such contributor to the software ends
#     automatically.
#     (C) If you distribute any portion of the software, you must
#     retain all copyright, patent, trademark, and attribution
#     notices that are present in the software.
#     (D) If you distribute any portion of the software in source
#     code form, you may do so only under this license by including a
#     complete copy of this license with your distribution. If you
#     distribute any portion of the software in compiled or object
#     code form, you may only do so under a license that complies
#     with this license.
#     (E) The software is licensed "as-is." You bear the risk of
#     using it. The contributors give no express warranties,
#     guarantees or conditions. You may have additional consumer
#     rights under your local laws which this license cannot change.
#     To the extent permitted under your local laws, the contributors
#     exclude the implied warranties of merchantability, fitness for
#     a particular purpose and non-infringement.
#
#

# Try to find DirectX SDK.
# Once done this will define
#
# DXSDK_FOUND
# DXSDK_INCLUDE_DIR
# DXSDK_LIBRARY_DIR
# DXSDK_LIBRARIES
# DXSDK_LOCATION
#
# Also will define

if (WIN32)
    find_path(DXSDK_INCLUDE_DIR
        NAMES
            d3dx9.h d3d9.h dxgi.h x3daudio2.h
        PATHS
            ${DXSDK_LOCATION}/Include
            $ENV{DXSDK_LOCATION}/Include
            ${DXSDK_ROOT}/Include
            $ENV{DXSDK_ROOT}/Include
            "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)*/Include"
            "C:/Program Files/Microsoft DirectX SDK (June 2010)*/Include"
    )

    if ("${CMAKE_GENERATOR}" MATCHES "[Ww]in64")
        set(ARCH x64)
    else()
        set(ARCH x86)
    endif()

    find_path(LIBRARY_DIR
            d3d9.lib d3dx9.lib DxErr.lib dxgi.lib
        PATHS
            ${DXSDK_LOCATION}/Lib/${ARCH}
            $ENV{DXSDK_LOCATION}/Lib/${ARCH}
            ${DXSDK_ROOT}/Lib/${ARCH}
            $ENV{DXSDK_ROOT}/Lib/${ARCH}
            "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)*/Lib/${ARCH}"
            "C:/Program Files/Microsoft DirectX SDK (June 2010)*/Lib/${ARCH}"
    )

    set(DXSDK_LIBRARY_DIR ${LIBRARY_DIR})

    foreach(DX_LIB d3d9 d3dx9 DxErr dxgi x3daudio )

        find_library(DXSDK_${DX_LIB}_LIBRARY
            NAMES 
                ${DX_LIB}.lib
            PATHS
                ${DXSDK_LIBRARY_DIR}
        )

        list(APPEND DXSDK_LIBRARIES ${DXSDK_${DX_LIB}_LIBRARY})


    endforeach(DX_LIB)

endif ()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(DXSDK DEFAULT_MSG
    DXSDK_INCLUDE_DIR
    DXSDK_LIBRARY_DIR
    DXSDK_LIBRARIES
)

mark_as_advanced(
    DXSDK_INCLUDE_DIR
    DXSDK_LIBRARY_DIR
    DXSDK_LIBRARIES
)

