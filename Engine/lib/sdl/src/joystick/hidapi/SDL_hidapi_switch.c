/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2019 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
/* This driver supports the Nintendo Switch Pro controller.
   Code and logic contributed by Valve Corporation under the SDL zlib license.
*/
#include "../../SDL_internal.h"

#ifdef SDL_JOYSTICK_HIDAPI

#include "SDL_hints.h"
#include "SDL_log.h"
#include "SDL_events.h"
#include "SDL_timer.h"
#include "SDL_joystick.h"
#include "SDL_gamecontroller.h"
#include "../SDL_sysjoystick.h"
#include "SDL_hidapijoystick_c.h"


#ifdef SDL_JOYSTICK_HIDAPI_SWITCH

typedef enum {
    k_eSwitchInputReportIDs_SubcommandReply       = 0x21,
    k_eSwitchInputReportIDs_FullControllerState   = 0x30,
    k_eSwitchInputReportIDs_SimpleControllerState = 0x3F,
    k_eSwitchInputReportIDs_CommandAck            = 0x81,
} ESwitchInputReportIDs;

typedef enum {
    k_eSwitchOutputReportIDs_RumbleAndSubcommand = 0x01,
    k_eSwitchOutputReportIDs_Rumble              = 0x10,
    k_eSwitchOutputReportIDs_Proprietary         = 0x80,
} ESwitchOutputReportIDs;

typedef enum {
    k_eSwitchSubcommandIDs_BluetoothManualPair = 0x01,
    k_eSwitchSubcommandIDs_RequestDeviceInfo   = 0x02,
    k_eSwitchSubcommandIDs_SetInputReportMode  = 0x03,
    k_eSwitchSubcommandIDs_SetHCIState         = 0x06,
    k_eSwitchSubcommandIDs_SPIFlashRead        = 0x10,
    k_eSwitchSubcommandIDs_SetPlayerLights     = 0x30,
    k_eSwitchSubcommandIDs_SetHomeLight        = 0x38,
    k_eSwitchSubcommandIDs_EnableIMU           = 0x40,
    k_eSwitchSubcommandIDs_SetIMUSensitivity   = 0x41,
    k_eSwitchSubcommandIDs_EnableVibration     = 0x48,
} ESwitchSubcommandIDs;

typedef enum {
    k_eSwitchProprietaryCommandIDs_Handshake = 0x02,
    k_eSwitchProprietaryCommandIDs_HighSpeed = 0x03,
    k_eSwitchProprietaryCommandIDs_ForceUSB  = 0x04,
    k_eSwitchProprietaryCommandIDs_ClearUSB  = 0x05,
    k_eSwitchProprietaryCommandIDs_ResetMCU  = 0x06,
} ESwitchProprietaryCommandIDs;

typedef enum {
    k_eSwitchDeviceInfoControllerType_JoyConLeft     = 0x1,
    k_eSwitchDeviceInfoControllerType_JoyConRight    = 0x2,
    k_eSwitchDeviceInfoControllerType_ProController  = 0x3,
} ESwitchDeviceInfoControllerType;

#define k_unSwitchOutputPacketDataLength 49
#define k_unSwitchMaxOutputPacketLength  64
#define k_unSwitchBluetoothPacketLength  k_unSwitchOutputPacketDataLength
#define k_unSwitchUSBPacketLength        k_unSwitchMaxOutputPacketLength

#define k_unSPIStickCalibrationStartOffset  0x603D
#define k_unSPIStickCalibrationEndOffset    0x604E
#define k_unSPIStickCalibrationLength       (k_unSPIStickCalibrationEndOffset - k_unSPIStickCalibrationStartOffset + 1)

#pragma pack(1)
typedef struct
{
    Uint8 rgucButtons[2];
    Uint8 ucStickHat;
    Sint16 sJoystickLeft[2];
    Sint16 sJoystickRight[2];
} SwitchSimpleStatePacket_t;

typedef struct
{
    Uint8 ucCounter;
    Uint8 ucBatteryAndConnection;
    Uint8 rgucButtons[3];
    Uint8 rgucJoystickLeft[3];
    Uint8 rgucJoystickRight[3];
    Uint8 ucVibrationCode;
} SwitchControllerStatePacket_t;

typedef struct
{
    SwitchControllerStatePacket_t controllerState;

    struct {
        Sint16 sAccelX;
        Sint16 sAccelY;
        Sint16 sAccelZ;

        Sint16 sGyroX;
        Sint16 sGyroY;
        Sint16 sGyroZ;
    } imuState[3];
} SwitchStatePacket_t;

typedef struct
{
    Uint32 unAddress;
    Uint8 ucLength;
} SwitchSPIOpData_t;

typedef struct
{
    SwitchControllerStatePacket_t m_controllerState;

    Uint8 ucSubcommandAck;
    Uint8 ucSubcommandID;

    #define k_unSubcommandDataBytes 35
    union {
        Uint8 rgucSubcommandData[ k_unSubcommandDataBytes ];

        struct {
            SwitchSPIOpData_t opData;
            Uint8 rgucReadData[ k_unSubcommandDataBytes - sizeof(SwitchSPIOpData_t) ];
        } spiReadData;

        struct {
            Uint8 rgucFirmwareVersion[2];
            Uint8 ucDeviceType;
            Uint8 ucFiller1;
            Uint8 rgucMACAddress[6];
            Uint8 ucFiller2;
            Uint8 ucColorLocation;
        } deviceInfo;
    };
} SwitchSubcommandInputPacket_t;

typedef struct
{
    Uint8 rgucData[4];
} SwitchRumbleData_t;

typedef struct
{
    Uint8 ucPacketType;
    Uint8 ucPacketNumber;
    SwitchRumbleData_t rumbleData[2];
} SwitchCommonOutputPacket_t;

typedef struct
{
    SwitchCommonOutputPacket_t commonData;

    Uint8 ucSubcommandID;
    Uint8 rgucSubcommandData[ k_unSwitchOutputPacketDataLength - sizeof(SwitchCommonOutputPacket_t) - 1 ];
} SwitchSubcommandOutputPacket_t;

typedef struct
{
    Uint8 ucPacketType;
    Uint8 ucProprietaryID;

    Uint8 rgucProprietaryData[ k_unSwitchOutputPacketDataLength - 1 - 1 ];
} SwitchProprietaryOutputPacket_t;
#pragma pack()

typedef struct {
    hid_device *dev;
    SDL_bool m_bIsUsingBluetooth;
    Uint8 m_nCommandNumber;
    SwitchCommonOutputPacket_t m_RumblePacket;
    Uint32 m_nRumbleExpiration;
    Uint8 m_rgucReadBuffer[k_unSwitchMaxOutputPacketLength];
    SwitchSimpleStatePacket_t m_lastSimpleState;
    SwitchStatePacket_t m_lastFullState;

    struct StickCalibrationData {
        struct {
            Sint16 sCenter;
            Sint16 sMin;
            Sint16 sMax;
        } axis[2];
    } m_StickCalData[2];

    struct StickExtents {
        struct {
            Sint16 sMin;
            Sint16 sMax;
        } axis[2];
    } m_StickExtents[2];
} SDL_DriverSwitch_Context;


static SDL_bool
HIDAPI_DriverSwitch_IsSupportedDevice(Uint16 vendor_id, Uint16 product_id, Uint16 version, int interface_number)
{
    return SDL_IsJoystickNintendoSwitchPro(vendor_id, product_id);
}

static const char *
HIDAPI_DriverSwitch_GetDeviceName(Uint16 vendor_id, Uint16 product_id)
{
    /* Give a user friendly name for this controller */
    if (SDL_IsJoystickNintendoSwitchPro(vendor_id, product_id)) {
        return "Nintendo Switch Pro Controller";
    }
    return NULL;
}

static int ReadInput(SDL_DriverSwitch_Context *ctx)
{
    return hid_read_timeout(ctx->dev, ctx->m_rgucReadBuffer, sizeof(ctx->m_rgucReadBuffer), 0);
}

static int WriteOutput(SDL_DriverSwitch_Context *ctx, Uint8 *data, int size)
{
    return hid_write(ctx->dev, data, size);
}

static SwitchSubcommandInputPacket_t *ReadSubcommandReply(SDL_DriverSwitch_Context *ctx, ESwitchSubcommandIDs expectedID)
{
    /* Average response time for messages is ~30ms */
    Uint32 TimeoutMs = 100;
    Uint32 startTicks = SDL_GetTicks();

    int nRead = 0;
    while ((nRead = ReadInput(ctx)) != -1) {
        if (nRead > 0) {
            if (ctx->m_rgucReadBuffer[0] == k_eSwitchInputReportIDs_SubcommandReply) {
                SwitchSubcommandInputPacket_t *reply = (SwitchSubcommandInputPacket_t *)&ctx->m_rgucReadBuffer[ 1 ];
                if (reply->ucSubcommandID == expectedID && (reply->ucSubcommandAck & 0x80)) {
                    return reply;
                }
            }
        } else {
            SDL_Delay(1);
        }

        if (SDL_TICKS_PASSED(SDL_GetTicks(), startTicks + TimeoutMs)) {
            break;
        }
    }
    return NULL;
}

static SDL_bool ReadProprietaryReply(SDL_DriverSwitch_Context *ctx, ESwitchProprietaryCommandIDs expectedID)
{
    /* Average response time for messages is ~30ms */
    Uint32 TimeoutMs = 100;
    Uint32 startTicks = SDL_GetTicks();

    int nRead = 0;
    while ((nRead = ReadInput(ctx)) != -1) {
        if (nRead > 0) {
            if (ctx->m_rgucReadBuffer[0] == k_eSwitchInputReportIDs_CommandAck && ctx->m_rgucReadBuffer[ 1 ] == expectedID) {
                return SDL_TRUE;
            }
        } else {
            SDL_Delay(1);
        }

        if (SDL_TICKS_PASSED(SDL_GetTicks(), startTicks + TimeoutMs)) {
            break;
        }
    }
    return SDL_FALSE;
}

static void ConstructSubcommand(SDL_DriverSwitch_Context *ctx, ESwitchSubcommandIDs ucCommandID, Uint8 *pBuf, Uint8 ucLen, SwitchSubcommandOutputPacket_t *outPacket)
{
    SDL_memset(outPacket, 0, sizeof(*outPacket));

    outPacket->commonData.ucPacketType = k_eSwitchOutputReportIDs_RumbleAndSubcommand;
    outPacket->commonData.ucPacketNumber = ctx->m_nCommandNumber;

    SDL_memcpy(&outPacket->commonData.rumbleData, &ctx->m_RumblePacket.rumbleData, sizeof(ctx->m_RumblePacket.rumbleData));

    outPacket->ucSubcommandID = ucCommandID;
    SDL_memcpy(outPacket->rgucSubcommandData, pBuf, ucLen);

    ctx->m_nCommandNumber = (ctx->m_nCommandNumber + 1) & 0xF;
}

static SDL_bool WritePacket(SDL_DriverSwitch_Context *ctx, void *pBuf, Uint8 ucLen)
{
    Uint8 rgucBuf[k_unSwitchMaxOutputPacketLength];
    const size_t unWriteSize = ctx->m_bIsUsingBluetooth ? k_unSwitchBluetoothPacketLength : k_unSwitchUSBPacketLength;

    if (ucLen > k_unSwitchOutputPacketDataLength) {
        return SDL_FALSE;
    }

    if (ucLen < unWriteSize) {
        SDL_memcpy(rgucBuf, pBuf, ucLen);
        SDL_memset(rgucBuf+ucLen, 0, unWriteSize-ucLen);
        pBuf = rgucBuf;
        ucLen = (Uint8)unWriteSize;
    }
    return (WriteOutput(ctx, (Uint8 *)pBuf, ucLen) >= 0);
}

static SDL_bool WriteSubcommand(SDL_DriverSwitch_Context *ctx, ESwitchSubcommandIDs ucCommandID, Uint8 *pBuf, Uint8 ucLen, SwitchSubcommandInputPacket_t **ppReply)
{
    int nRetries = 5;
    SwitchSubcommandInputPacket_t *reply = NULL;

    while (!reply && nRetries--) {
        SwitchSubcommandOutputPacket_t commandPacket;
        ConstructSubcommand(ctx, ucCommandID, pBuf, ucLen, &commandPacket);

        if (!WritePacket(ctx, &commandPacket, sizeof(commandPacket))) {
            continue;
        }

        reply = ReadSubcommandReply(ctx, ucCommandID);
    }

    if (ppReply) {
        *ppReply = reply;
    }
    return reply != NULL;
}

static SDL_bool WriteProprietary(SDL_DriverSwitch_Context *ctx, ESwitchProprietaryCommandIDs ucCommand, Uint8 *pBuf, Uint8 ucLen, SDL_bool waitForReply)
{
    int nRetries = 5;

    while (nRetries--) {
        SwitchProprietaryOutputPacket_t packet;

        if ((!pBuf && ucLen > 0) || ucLen > sizeof(packet.rgucProprietaryData)) {
            return SDL_FALSE;
        }

        packet.ucPacketType = k_eSwitchOutputReportIDs_Proprietary;
        packet.ucProprietaryID = ucCommand;
        SDL_memcpy(packet.rgucProprietaryData, pBuf, ucLen);

        if (!WritePacket(ctx, &packet, sizeof(packet))) {
            continue;
        }

        if (!waitForReply || ReadProprietaryReply(ctx, ucCommand)) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

static void SetNeutralRumble(SwitchRumbleData_t *pRumble)
{
    pRumble->rgucData[0] = 0x00;
    pRumble->rgucData[1] = 0x01;
    pRumble->rgucData[2] = 0x40;
    pRumble->rgucData[3] = 0x40;
}

static void EncodeRumble(SwitchRumbleData_t *pRumble, Uint16 usHighFreq, Uint8 ucHighFreqAmp, Uint8 ucLowFreq, Uint16 usLowFreqAmp)
{
    if (ucHighFreqAmp > 0 || usLowFreqAmp > 0) {
        // High-band frequency and low-band amplitude are actually nine-bits each so they
        // take a bit from the high-band amplitude and low-band frequency bytes respectively
        pRumble->rgucData[0] = usHighFreq & 0xFF;
        pRumble->rgucData[1] = ucHighFreqAmp | ((usHighFreq >> 8) & 0x01);

        pRumble->rgucData[2]  = ucLowFreq | ((usLowFreqAmp >> 8) & 0x80);
        pRumble->rgucData[3]  = usLowFreqAmp & 0xFF;

#ifdef DEBUG_RUMBLE
        SDL_Log("Freq: %.2X %.2X  %.2X, Amp: %.2X  %.2X %.2X\n",
            usHighFreq & 0xFF, ((usHighFreq >> 8) & 0x01), ucLowFreq,
            ucHighFreqAmp, ((usLowFreqAmp >> 8) & 0x80), usLowFreqAmp & 0xFF);
#endif
    } else {
        SetNeutralRumble(pRumble);
    }
}

static SDL_bool WriteRumble(SDL_DriverSwitch_Context *ctx)
{
    /* Write into m_RumblePacket rather than a temporary buffer to allow the current rumble state
     * to be retained for subsequent rumble or subcommand packets sent to the controller
     */
    ctx->m_RumblePacket.ucPacketType = k_eSwitchOutputReportIDs_Rumble;
    ctx->m_RumblePacket.ucPacketNumber = ctx->m_nCommandNumber;
    ctx->m_nCommandNumber = (ctx->m_nCommandNumber + 1) & 0xF;

    return WritePacket(ctx, (Uint8 *)&ctx->m_RumblePacket, sizeof(ctx->m_RumblePacket));
}

static SDL_bool BTrySetupUSB(SDL_DriverSwitch_Context *ctx)
{
    /* We have to send a connection handshake to the controller when communicating over USB
     * before we're able to send it other commands. Luckily this command is not supported
     * over Bluetooth, so we can use the controller's lack of response as a way to
     * determine if the connection is over USB or Bluetooth
     */
    if (!WriteProprietary(ctx, k_eSwitchProprietaryCommandIDs_Handshake, NULL, 0, SDL_TRUE)) {
        return SDL_FALSE;
    }
    if (!WriteProprietary(ctx, k_eSwitchProprietaryCommandIDs_HighSpeed, NULL, 0, SDL_TRUE)) {
        return SDL_FALSE;
    }
    if (!WriteProprietary(ctx, k_eSwitchProprietaryCommandIDs_Handshake, NULL, 0, SDL_TRUE)) {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

static SDL_bool SetVibrationEnabled(SDL_DriverSwitch_Context *ctx, Uint8 enabled)
{
    return WriteSubcommand(ctx, k_eSwitchSubcommandIDs_EnableVibration, &enabled, sizeof(enabled), NULL);

}
static SDL_bool SetInputMode(SDL_DriverSwitch_Context *ctx, Uint8 input_mode)
{
    return WriteSubcommand(ctx, k_eSwitchSubcommandIDs_SetInputReportMode, &input_mode, 1, NULL);
}

static SDL_bool SetHomeLED(SDL_DriverSwitch_Context *ctx, Uint8 brightness)
{
    Uint8 ucLedIntensity = 0;
    Uint8 rgucBuffer[4];

    if (brightness > 0) {
        if (brightness < 65) {
            ucLedIntensity = (brightness + 5) / 10;
        } else {
            ucLedIntensity = (Uint8)SDL_ceilf(0xF * SDL_powf((float)brightness / 100.f, 2.13f));
        }
    }

    rgucBuffer[0] = (0x0 << 4) | 0x1;  /* 0 mini cycles (besides first), cycle duration 8ms */
    rgucBuffer[1] = ((ucLedIntensity & 0xF) << 4) | 0x0;  /* LED start intensity (0x0-0xF), 0 cycles (LED stays on at start intensity after first cycle) */
    rgucBuffer[2] = ((ucLedIntensity & 0xF) << 4) | 0x0;  /* First cycle LED intensity, 0x0 intensity for second cycle */
    rgucBuffer[3] = (0x0 << 4) | 0x0;  /* 8ms fade transition to first cycle, 8ms first cycle LED duration */

    return WriteSubcommand(ctx, k_eSwitchSubcommandIDs_SetHomeLight, rgucBuffer, sizeof(rgucBuffer), NULL);
}

static SDL_bool SetSlotLED(SDL_DriverSwitch_Context *ctx, Uint8 slot)
{
    Uint8 led_data = (1 << slot);
    return WriteSubcommand(ctx, k_eSwitchSubcommandIDs_SetPlayerLights, &led_data, sizeof(led_data), NULL);
}

static SDL_bool LoadStickCalibration(SDL_DriverSwitch_Context *ctx)
{
    Uint8 *pStickCal;
    size_t stick, axis;
    SwitchSubcommandInputPacket_t *reply = NULL;

    /* Read Calibration Info */
    SwitchSPIOpData_t readParams;
    readParams.unAddress = k_unSPIStickCalibrationStartOffset;
    readParams.ucLength = k_unSPIStickCalibrationLength;

    if (!WriteSubcommand(ctx, k_eSwitchSubcommandIDs_SPIFlashRead, (uint8_t *)&readParams, sizeof(readParams), &reply)) {
        return SDL_FALSE;
    }

    /* Stick calibration values are 12-bits each and are packed by bit
     * For whatever reason the fields are in a different order for each stick
     * Left:  X-Max, Y-Max, X-Center, Y-Center, X-Min, Y-Min
     * Right: X-Center, Y-Center, X-Min, Y-Min, X-Max, Y-Max
     */
    pStickCal = reply->spiReadData.rgucReadData;

    /* Left stick */
    ctx->m_StickCalData[0].axis[0].sMax    = ((pStickCal[1] << 8) & 0xF00) | pStickCal[0];     /* X Axis max above center */
    ctx->m_StickCalData[0].axis[1].sMax    = (pStickCal[2] << 4) | (pStickCal[1] >> 4);         /* Y Axis max above center */
    ctx->m_StickCalData[0].axis[0].sCenter = ((pStickCal[4] << 8) & 0xF00) | pStickCal[3];     /* X Axis center */
    ctx->m_StickCalData[0].axis[1].sCenter = (pStickCal[5] << 4) | (pStickCal[4] >> 4);        /* Y Axis center */
    ctx->m_StickCalData[0].axis[0].sMin    = ((pStickCal[7] << 8) & 0xF00) | pStickCal[6];      /* X Axis min below center */
    ctx->m_StickCalData[0].axis[1].sMin    = (pStickCal[8] << 4) | (pStickCal[7] >> 4);        /* Y Axis min below center */

    /* Right stick */
    ctx->m_StickCalData[1].axis[0].sCenter = ((pStickCal[10] << 8) & 0xF00) | pStickCal[9];     /* X Axis center */
    ctx->m_StickCalData[1].axis[1].sCenter = (pStickCal[11] << 4) | (pStickCal[10] >> 4);      /* Y Axis center */
    ctx->m_StickCalData[1].axis[0].sMin    = ((pStickCal[13] << 8) & 0xF00) | pStickCal[12];    /* X Axis min below center */
    ctx->m_StickCalData[1].axis[1].sMin    = (pStickCal[14] << 4) | (pStickCal[13] >> 4);      /* Y Axis min below center */
    ctx->m_StickCalData[1].axis[0].sMax    = ((pStickCal[16] << 8) & 0xF00) | pStickCal[15];    /* X Axis max above center */
    ctx->m_StickCalData[1].axis[1].sMax    = (pStickCal[17] << 4) | (pStickCal[16] >> 4);      /* Y Axis max above center */

    /* Filter out any values that were uninitialized (0xFFF) in the SPI read */
    for (stick = 0; stick < 2; ++stick) {
        for (axis = 0; axis < 2; ++axis) {
            if (ctx->m_StickCalData[stick].axis[axis].sCenter == 0xFFF) {
                ctx->m_StickCalData[stick].axis[axis].sCenter = 0;
            }
            if (ctx->m_StickCalData[stick].axis[axis].sMax == 0xFFF) {
                ctx->m_StickCalData[stick].axis[axis].sMax = 0;
            }
            if (ctx->m_StickCalData[stick].axis[axis].sMin == 0xFFF) {
                ctx->m_StickCalData[stick].axis[axis].sMin = 0;
            }
        }
    }

    if (ctx->m_bIsUsingBluetooth) {
        for (stick = 0; stick < 2; ++stick) {
            for(axis = 0; axis < 2; ++axis) {
                ctx->m_StickExtents[stick].axis[axis].sMin = (Sint16)(SDL_MIN_SINT16 * 0.5f);
                ctx->m_StickExtents[stick].axis[axis].sMax = (Sint16)(SDL_MAX_SINT16 * 0.5f);
            }
        }
    } else {
        for (stick = 0; stick < 2; ++stick) {
            for(axis = 0; axis < 2; ++axis) {
                ctx->m_StickExtents[stick].axis[axis].sMin = -(Sint16)(ctx->m_StickCalData[stick].axis[axis].sMin * 0.7f);
                ctx->m_StickExtents[stick].axis[axis].sMax = (Sint16)(ctx->m_StickCalData[stick].axis[axis].sMax * 0.7f);
            }
        }
    }
    return SDL_TRUE;
}

static float fsel(float fComparand, float fValGE, float fLT)
{
    return fComparand >= 0 ? fValGE : fLT;
}

static float RemapVal(float val, float A, float B, float C, float D)
{
    if (A == B) {
        return fsel(val - B , D , C);
    }
    return C + (D - C) * (val - A) / (B - A);
}

static Sint16 ApplyStickCalibrationCentered(SDL_DriverSwitch_Context *ctx, int nStick, int nAxis, Sint16 sRawValue, Sint16 sCenter)
{
    sRawValue -= sCenter;

    if (sRawValue > ctx->m_StickExtents[nStick].axis[nAxis].sMax) {
        ctx->m_StickExtents[nStick].axis[nAxis].sMax = sRawValue;
    }
    if (sRawValue < ctx->m_StickExtents[nStick].axis[nAxis].sMin) {
        ctx->m_StickExtents[nStick].axis[nAxis].sMin = sRawValue;
    }

    if (sRawValue > 0) {
        return (Sint16)(RemapVal(sRawValue, 0, ctx->m_StickExtents[nStick].axis[nAxis].sMax, 0, SDL_MAX_SINT16));
    } else {
        return (Sint16)(RemapVal(sRawValue, ctx->m_StickExtents[nStick].axis[nAxis].sMin, 0, SDL_MIN_SINT16, 0));
    }
}

static Sint16 ApplyStickCalibration(SDL_DriverSwitch_Context *ctx, int nStick, int nAxis, Sint16 sRawValue)
{
    return ApplyStickCalibrationCentered(ctx, nStick, nAxis, sRawValue, ctx->m_StickCalData[nStick].axis[nAxis].sCenter);
}

static SDL_bool
HIDAPI_DriverSwitch_Init(SDL_Joystick *joystick, hid_device *dev, Uint16 vendor_id, Uint16 product_id, void **context)
{
    SDL_DriverSwitch_Context *ctx;
    Uint8 input_mode;

    ctx = (SDL_DriverSwitch_Context *)SDL_calloc(1, sizeof(*ctx));
    if (!ctx) {
        SDL_OutOfMemory();
        return SDL_FALSE;
    }
    ctx->dev = dev;

    *context = ctx;

    /* Initialize rumble data */
    SetNeutralRumble(&ctx->m_RumblePacket.rumbleData[0]);
    SetNeutralRumble(&ctx->m_RumblePacket.rumbleData[1]);

    /* Try setting up USB mode, and if that fails we're using Bluetooth */
    if (!BTrySetupUSB(ctx)) {
        ctx->m_bIsUsingBluetooth = SDL_TRUE;
    }

    if (!LoadStickCalibration(ctx)) {
        SDL_SetError("Couldn't load stick calibration");
        SDL_free(ctx);
        return SDL_FALSE;
    }

    if (!SetVibrationEnabled(ctx, 1)) {
        SDL_SetError("Couldn't enable vibration");
        SDL_free(ctx);
        return SDL_FALSE;
    }

    /* Set the desired input mode */
    if (ctx->m_bIsUsingBluetooth) {
        input_mode = k_eSwitchInputReportIDs_SimpleControllerState;
    } else {
        input_mode = k_eSwitchInputReportIDs_FullControllerState;
    }
    if (!SetInputMode(ctx, input_mode)) {
        SDL_SetError("Couldn't set input mode");
        SDL_free(ctx);
        return SDL_FALSE;
    }

    /* Start sending USB reports */
    if (!ctx->m_bIsUsingBluetooth) {
        /* ForceUSB doesn't generate an ACK, so don't wait for a reply */
        if (!WriteProprietary(ctx, k_eSwitchProprietaryCommandIDs_ForceUSB, NULL, 0, SDL_FALSE)) {
            SDL_SetError("Couldn't start USB reports");
            SDL_free(ctx);
            return SDL_FALSE;
        }
    }

    /* Set the LED state */
    SetHomeLED(ctx, 100);
    SetSlotLED(ctx, (joystick->instance_id % 4));

    /* Initialize the joystick capabilities */
    joystick->nbuttons = SDL_CONTROLLER_BUTTON_MAX;
    joystick->naxes = SDL_CONTROLLER_AXIS_MAX;
    joystick->epowerlevel = SDL_JOYSTICK_POWER_WIRED;

    return SDL_TRUE;
}

static int
HIDAPI_DriverSwitch_Rumble(SDL_Joystick *joystick, hid_device *dev, void *context, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms)
{
    SDL_DriverSwitch_Context *ctx = (SDL_DriverSwitch_Context *)context;

    /* Experimentally determined rumble values. These will only matter on some controllers as tested ones
     * seem to disregard these and just use any non-zero rumble values as a binary flag for constant rumble
     *
     * More information about these values can be found here:
     * https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/rumble_data_table.md
     */
    const Uint16 k_usHighFreq = 0x0074;
    const Uint8  k_ucHighFreqAmp = 0xBE;
    const Uint8  k_ucLowFreq = 0x3D;
    const Uint16 k_usLowFreqAmp = 0x806F;

    if (low_frequency_rumble) {
        EncodeRumble(&ctx->m_RumblePacket.rumbleData[0], k_usHighFreq, k_ucHighFreqAmp, k_ucLowFreq, k_usLowFreqAmp);
    } else {
        SetNeutralRumble(&ctx->m_RumblePacket.rumbleData[0]);
    }

    if (high_frequency_rumble) {
        EncodeRumble(&ctx->m_RumblePacket.rumbleData[1], k_usHighFreq, k_ucHighFreqAmp, k_ucLowFreq, k_usLowFreqAmp);
    } else {
        SetNeutralRumble(&ctx->m_RumblePacket.rumbleData[1]);
    }

    if (!WriteRumble(ctx)) {
        SDL_SetError("Couldn't send rumble packet");
        return -1;
    }

    if ((low_frequency_rumble || high_frequency_rumble) && duration_ms) {
        ctx->m_nRumbleExpiration = SDL_GetTicks() + duration_ms;
    } else {
        ctx->m_nRumbleExpiration = 0;
    }
    return 0;
}

static void HandleSimpleControllerState(SDL_Joystick *joystick, SDL_DriverSwitch_Context *ctx, SwitchSimpleStatePacket_t *packet)
{
    /* 0x8000 is the neutral value for all joystick axes */
    const Uint16 usJoystickCenter = 0x8000;
    Sint16 axis;

    if (packet->rgucButtons[0] != ctx->m_lastSimpleState.rgucButtons[0]) {
        Uint8 data = packet->rgucButtons[0];
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_A, (data & 0x01) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_B, (data & 0x02) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_X, (data & 0x04) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_Y, (data & 0x08) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_LEFTSHOULDER, (data & 0x10) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, (data & 0x20) ? SDL_PRESSED : SDL_RELEASED);

        axis = (data & 0x40) ? 32767 : -32768;
        SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_TRIGGERLEFT, axis);

        axis = (data & 0x80) ? 32767 : -32768;
        SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, axis);
    }

    if (packet->rgucButtons[1] != ctx->m_lastSimpleState.rgucButtons[1]) {
        Uint8 data = packet->rgucButtons[1];
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_BACK, (data & 0x01) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_START, (data & 0x02) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_LEFTSTICK, (data & 0x04) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_RIGHTSTICK, (data & 0x08) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_GUIDE, (data & 0x10) ? SDL_PRESSED : SDL_RELEASED);
    }

    if (packet->ucStickHat != ctx->m_lastSimpleState.ucStickHat) {
        SDL_bool dpad_up = SDL_FALSE;
        SDL_bool dpad_down = SDL_FALSE;
        SDL_bool dpad_left = SDL_FALSE;
        SDL_bool dpad_right = SDL_FALSE;

        switch (packet->ucStickHat) {
        case 0:
            dpad_up = SDL_TRUE;
            break;
        case 1:
            dpad_up = SDL_TRUE;
            dpad_right = SDL_TRUE;
            break;
        case 2:
            dpad_right = SDL_TRUE;
            break;
        case 3:
            dpad_right = SDL_TRUE;
            dpad_down = SDL_TRUE;
            break;
        case 4:
            dpad_down = SDL_TRUE;
            break;
        case 5:
            dpad_left = SDL_TRUE;
            dpad_down = SDL_TRUE;
            break;
        case 6:
            dpad_left = SDL_TRUE;
            break;
        case 7:
            dpad_up = SDL_TRUE;
            dpad_left = SDL_TRUE;
            break;
        default:
            break;
        }
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_DOWN, dpad_down);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_UP, dpad_up);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_RIGHT, dpad_right);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_LEFT, dpad_left);
    }

    axis = ApplyStickCalibrationCentered(ctx, 0, 0, packet->sJoystickLeft[0], (Sint16)usJoystickCenter);
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_LEFTX, axis);

    axis = ApplyStickCalibrationCentered(ctx, 0, 1, packet->sJoystickLeft[1], (Sint16)usJoystickCenter);
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_LEFTY, axis);

    axis = ApplyStickCalibrationCentered(ctx, 1, 0, packet->sJoystickRight[0], (Sint16)usJoystickCenter);
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_RIGHTX, axis);

    axis = ApplyStickCalibrationCentered(ctx, 1, 1, packet->sJoystickRight[1], (Sint16)usJoystickCenter);
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_RIGHTY, axis);

    ctx->m_lastSimpleState = *packet;
}

static void HandleFullControllerState(SDL_Joystick *joystick, SDL_DriverSwitch_Context *ctx, SwitchStatePacket_t *packet)
{
    Sint16 axis;

    if (packet->controllerState.rgucButtons[0] != ctx->m_lastFullState.controllerState.rgucButtons[0]) {
        Uint8 data = packet->controllerState.rgucButtons[0];
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_X, (data & 0x01) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_Y, (data & 0x02) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_A, (data & 0x04) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_B, (data & 0x08) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, (data & 0x40) ? SDL_PRESSED : SDL_RELEASED);
        axis = (data & 0x80) ? 32767 : -32768;
        SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, axis);
    }

    if (packet->controllerState.rgucButtons[1] != ctx->m_lastFullState.controllerState.rgucButtons[1]) {
        Uint8 data = packet->controllerState.rgucButtons[1];
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_BACK, (data & 0x01) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_START, (data & 0x02) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_RIGHTSTICK, (data & 0x04) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_LEFTSTICK, (data & 0x08) ? SDL_PRESSED : SDL_RELEASED);

        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_GUIDE, (data & 0x10) ? SDL_PRESSED : SDL_RELEASED);
    }

    if (packet->controllerState.rgucButtons[2] != ctx->m_lastFullState.controllerState.rgucButtons[2]) {
        Uint8 data = packet->controllerState.rgucButtons[2];
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_DOWN, (data & 0x01) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_UP, (data & 0x02) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_RIGHT, (data & 0x04) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_DPAD_LEFT, (data & 0x08) ? SDL_PRESSED : SDL_RELEASED);
        SDL_PrivateJoystickButton(joystick, SDL_CONTROLLER_BUTTON_LEFTSHOULDER, (data & 0x40) ? SDL_PRESSED : SDL_RELEASED);
        axis = (data & 0x80) ? 32767 : -32768;
        SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_TRIGGERLEFT, axis);
    }

    axis = packet->controllerState.rgucJoystickLeft[0] | ((packet->controllerState.rgucJoystickLeft[1] & 0xF) << 8);
    axis = ApplyStickCalibration(ctx, 0, 0, axis);
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_LEFTX, axis);

    axis = ((packet->controllerState.rgucJoystickLeft[1] & 0xF0) >> 4) | (packet->controllerState.rgucJoystickLeft[2] << 4);
    axis = ApplyStickCalibration(ctx, 0, 1, axis);
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_LEFTY, ~axis);

    axis = packet->controllerState.rgucJoystickRight[0] | ((packet->controllerState.rgucJoystickRight[1] & 0xF) << 8);
    axis = ApplyStickCalibration(ctx, 1, 0, axis);
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_RIGHTX, axis);

    axis = ((packet->controllerState.rgucJoystickRight[1] & 0xF0) >> 4) | (packet->controllerState.rgucJoystickRight[2] << 4);
    axis = ApplyStickCalibration(ctx, 1, 1, axis);
    SDL_PrivateJoystickAxis(joystick, SDL_CONTROLLER_AXIS_RIGHTY, ~axis);

    /* High nibble of battery/connection byte is battery level, low nibble is connection status
     * LSB of connection nibble is USB/Switch connection status
     */
    if (packet->controllerState.ucBatteryAndConnection & 0x1) {
        joystick->epowerlevel = SDL_JOYSTICK_POWER_WIRED;
    } else {
        /* LSB of the battery nibble is used to report charging.
         * The battery level is reported from 0(empty)-8(full)
         */
        int level = (packet->controllerState.ucBatteryAndConnection & 0xE0) >> 4;
        if (level == 0) {
            joystick->epowerlevel = SDL_JOYSTICK_POWER_EMPTY;
        } else if (level <= 2) {
            joystick->epowerlevel = SDL_JOYSTICK_POWER_LOW;
        } else if (level <= 6) {
            joystick->epowerlevel = SDL_JOYSTICK_POWER_MEDIUM;
        } else {
            joystick->epowerlevel = SDL_JOYSTICK_POWER_FULL;
        }
    }

    ctx->m_lastFullState = *packet;
}

static SDL_bool
HIDAPI_DriverSwitch_Update(SDL_Joystick *joystick, hid_device *dev, void *context)
{
    SDL_DriverSwitch_Context *ctx = (SDL_DriverSwitch_Context *)context;
    int size;

    while ((size = ReadInput(ctx)) > 0) {
        switch (ctx->m_rgucReadBuffer[0]) {
        case k_eSwitchInputReportIDs_SimpleControllerState:
            HandleSimpleControllerState(joystick, ctx, (SwitchSimpleStatePacket_t *)&ctx->m_rgucReadBuffer[1]);
            break;
        case k_eSwitchInputReportIDs_FullControllerState:
            HandleFullControllerState(joystick, ctx, (SwitchStatePacket_t *)&ctx->m_rgucReadBuffer[1]);
            break;
        default:
            break;
        }
    }

    if (ctx->m_nRumbleExpiration) {
        Uint32 now = SDL_GetTicks();
        if (SDL_TICKS_PASSED(now, ctx->m_nRumbleExpiration)) {
            HIDAPI_DriverSwitch_Rumble(joystick, dev, context, 0, 0, 0);
        }
    }

    return (size >= 0);
}

static void
HIDAPI_DriverSwitch_Quit(SDL_Joystick *joystick, hid_device *dev, void *context)
{
    SDL_DriverSwitch_Context *ctx = (SDL_DriverSwitch_Context *)context;

    /* Restore simple input mode for other applications */
    SetInputMode(ctx, k_eSwitchInputReportIDs_SimpleControllerState);

    SDL_free(context);
}

SDL_HIDAPI_DeviceDriver SDL_HIDAPI_DriverSwitch =
{
    SDL_HINT_JOYSTICK_HIDAPI_SWITCH,
    SDL_TRUE,
    HIDAPI_DriverSwitch_IsSupportedDevice,
    HIDAPI_DriverSwitch_GetDeviceName,
    HIDAPI_DriverSwitch_Init,
    HIDAPI_DriverSwitch_Rumble,
    HIDAPI_DriverSwitch_Update,
    HIDAPI_DriverSwitch_Quit
};

#endif /* SDL_JOYSTICK_HIDAPI_SWITCH */

#endif /* SDL_JOYSTICK_HIDAPI */

/* vi: set ts=4 sw=4 expandtab: */
