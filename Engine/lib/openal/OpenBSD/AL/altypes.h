#ifndef _AL_TYPES_H_
#define _AL_TYPES_H_

/** OpenAL bool type. */
typedef char ALboolean;

/** OpenAL 8bit signed byte. */
typedef signed char ALbyte;

/** OpenAL 8bit unsigned byte. */
typedef unsigned char ALubyte;

/** OpenAL 16bit signed short integer type. */
typedef short ALshort;

/** OpenAL 16bit unsigned short integer type. */
typedef unsigned short ALushort;

/** OpenAL 32bit unsigned integer type. */
typedef unsigned int ALuint;

/** OpenAL 32bit signed integer type. */
typedef int ALint;

/** OpenAL 32bit floating point type. */
typedef float ALfloat;

/** OpenAL 64bit double point type. */
typedef double ALdouble;

/** OpenAL 32bit type. */
typedef signed int ALsizei;

/** OpenAL void type (for params, not returns). */
typedef void ALvoid;

/** OpenAL enumerations. */
typedef int ALenum;

/* Enumerant values begin at column 50. No tabs. */

/* bad value */
#define AL_INVALID                                -1

/* Boolean False. */
#define AL_FALSE                                  0

/** Boolean True. */
#define AL_TRUE                                   1

/**
 * Indicate the type of AL_SOURCE.
 * Sources can be spatialized 
 */
#define AL_SOURCE_TYPE                            0x0200

/** Indicate Source has relative coordinates. */
#define AL_SOURCE_RELATIVE                        0x0202

/**
 * Directional source, inner cone angle, in degrees.
 * Range:    [0-360] 
 * Default:  360
 */
#define AL_CONE_INNER_ANGLE                       0x1001

/**
 * Directional source, outer cone angle, in degrees.
 * Range:    [0-360] 
 * Default:  360
 */
#define AL_CONE_OUTER_ANGLE                       0x1002

/**
 * Specify the pitch to be applied, either at source,
 *  or on mixer results, at listener.
 * Range:   [0.5-2.0]
 * Default: 1.0
 */
#define AL_PITCH                                  0x1003
  
/** 
 * Specify the current location in three dimensional space.
 * OpenAL, like OpenGL, uses a right handed coordinate system,
 *  where in a frontal default view X (thumb) points right, 
 *  Y points up (index finger), and Z points towards the
 *  viewer/camera (middle finger). 
 * To switch from a left handed coordinate system, flip the
 *  sign on the Z coordinate.
 * Listener position is always in the world coordinate system.
 */ 
#define AL_POSITION                               0x1004
  
/** Specify the current direction. */
#define AL_DIRECTION                              0x1005
  
/** Specify the current velocity in three dimensional space. */
#define AL_VELOCITY                               0x1006

/**
 * Indicate whether source is looping.
 * Type: ALboolean?
 * Range:   [AL_TRUE, AL_FALSE]
 * Default: FALSE.
 */
#define AL_SOURCE_LOOPING                         0x1007

/**
 * Indicate whether source is meant to be streaming.
 * Type: ALboolean?
 * Range:   [AL_TRUE, AL_FALSE]
 * Default: FALSE.
 */
#define AL_STREAMING                              0x1008

/**
 * Indicate the buffer to provide sound samples. 
 * Type: ALuint.
 * Range: any valid Buffer id.
 */
#define AL_BUFFER                                 0x1009
  
/**
 * Indicate the gain (volume amplification) applied. 
 * Type:   ALfloat.
 * Range:  ]0.0-  ]
 * A value of 1.0 means un-attenuated/unchanged.
 * Each division by 2 equals an attenuation of -6dB.
 * Each multiplicaton with 2 equals an amplification of +6dB.
 * A value of 0.0 is meaningless with respect to a logarithmic
 *  scale; it is interpreted as zero volume - the channel
 *  is effectively disabled.
 */
#define AL_GAIN                                   0x100A

/**
 * Indicate the gain (volume amplification) applied, in a 
 * normalized linear scale.  This affects the value retrieved
 * by AL_GAIN.
 *
 * Type:   ALfloat.
 * Range:  ]0.0-  ]
 * A value of 1.0 means un-attenuated/unchanged.
 * A value of 0.0 is  interpreted as zero volume - the channel
 *  is effectively disabled.
 */
#define AL_GAIN_LINEAR                            0x100B

/* byte offset into source (in canon format).  -1 if source
 * is not playing.  Don't set this, get this.
 *
 * Type:  ALint
 * Range: -1 - +inf
 */
#define AL_BYTE_LOKI                              0x100C

/*
 * Indicate minimum source attenuation
 * Type: ALfloat
 * Range:  [0.0 - 1.0]
 *
 */
#define AL_SOURCE_ATTENUATION_MIN                 0x100D

/*
 * Indicate maximum source attenuation
 * Type: ALfloat
 * Range:  [0.0 - 1.0]
 *
 */
#define AL_SOURCE_ATTENUATION_MAX                 0x100E

/*
 * Indicate listener orientation.
 *
 * at/up 
 */
#define AL_ORIENTATION                            0x100F


/*
 * Source state information.
 */
#define AL_SOURCE_STATE                           0x1010
#define AL_INITIAL                                0x1011
#define AL_PLAYING                                0x1012
#define AL_PAUSED                                 0x1013
#define AL_STOPPED                                0x1014


/** Sound samples: format specifier. */
#define AL_FORMAT_MONO8                           0x1100
#define AL_FORMAT_MONO16                          0x1101
#define AL_FORMAT_STEREO8                         0x1102
#define AL_FORMAT_STEREO16                        0x1103


/** 
 * Sound samples: frequency, in units of Hertz [Hz].
 * This is the number of samples per second. Half of the
 *  sample frequency marks the maximum significant
 *  frequency component.
 */
#define AL_FREQUENCY                              0x2001
#define AL_BITS                                   0x2002
#define AL_CHANNELS                               0x2003
#define AL_SIZE                                   0x2004


/** Errors: No Error. */
#define AL_NO_ERROR                               AL_FALSE

/** 
 * Invalid Name paramater passed to AL call.
 */
#define AL_INVALID_NAME                           0xA001

/** 
 * Invalid parameter passed to AL call.
 */
#define AL_ILLEGAL_ENUM                           0xA002

/** 
 * Invalid enum parameter value.
 */
#define AL_INVALID_VALUE                          0xA003

/** 
 * Illegal call.
 */
#define AL_ILLEGAL_COMMAND                        0xA004
  
/**
 * No mojo.
 */
#define AL_OUT_OF_MEMORY                          0xA005


/** Context strings: Vendor Name. */
#define AL_VENDOR                                 0xB001
#define AL_VERSION                                0xB002
#define AL_RENDERER                               0xB003
#define AL_EXTENSIONS                             0xB004


/** IASIG Level 2 Environment. */

/**  
 * Parameter:  IASIG ROOM  blah
 * Type:       intgeger
 * Range:      [-10000, 0]
 * Default:    -10000 
 */
#define AL_ENV_ROOM_IASIG                         0x3001

/**
 * Parameter:  IASIG ROOM_HIGH_FREQUENCY
 * Type:       integer
 * Range:      [-10000, 0]
 * Default:    0 
 */
#define AL_ENV_ROOM_HIGH_FREQUENCY_IASIG          0x3002

/**
 * Parameter:  IASIG ROOM_ROLLOFF_FACTOR
 * Type:       float
 * Range:      [0.0, 10.0]
 * Default:    0.0 
 */
#define AL_ENV_ROOM_ROLLOFF_FACTOR_IASIG          0x3003

/** 
 * Parameter:  IASIG  DECAY_TIME
 * Type:       float
 * Range:      [0.1, 20.0]
 * Default:    1.0 
 */
#define AL_ENV_DECAY_TIME_IASIG                   0x3004

/**
 * Parameter:  IASIG DECAY_HIGH_FREQUENCY_RATIO
 * Type:       float
 * Range:      [0.1, 2.0]
 * Default:    0.5
 */
#define AL_ENV_DECAY_HIGH_FREQUENCY_RATIO_IASIG   0x3005

/**
 * Parameter:  IASIG REFLECTIONS
 * Type:       integer
 * Range:      [-10000, 1000]
 * Default:    -10000
 */
#define AL_ENV_REFLECTIONS_IASIG                  0x3006

/**
 * Parameter:  IASIG REFLECTIONS_DELAY
 * Type:       float
 * Range:      [0.0, 0.3]
 * Default:    0.02
 */
#define AL_ENV_REFLECTIONS_DELAY_IASIG            0x3006

/**
 * Parameter:  IASIG REVERB
 * Type:       integer
 * Range:      [-10000,2000]
 * Default:    -10000
 */
#define AL_ENV_REVERB_IASIG                       0x3007

/**
 * Parameter:  IASIG REVERB_DELAY
 * Type:       float
 * Range:      [0.0, 0.1]
 * Default:    0.04
 */
#define AL_ENV_REVERB_DELAY_IASIG                 0x3008

/**
 * Parameter:  IASIG DIFFUSION
 * Type:       float
 * Range:      [0.0, 100.0]
 * Default:    100.0
 */
#define AL_ENV_DIFFUSION_IASIG                    0x3009

/**
 * Parameter:  IASIG DENSITY
 * Type:       float
 * Range:      [0.0, 100.0]
 * Default:    100.0
 */
#define AL_ENV_DENSITY_IASIG                      0x300A
  
  /**
 * Parameter:  IASIG HIGH_FREQUENCY_REFERENCE
 * Type:       float
 * Range:      [20.0, 20000.0]
 * Default:    5000.0
 */
#define AL_ENV_HIGH_FREQUENCY_REFERENCE_IASIG     0x300B

#endif
