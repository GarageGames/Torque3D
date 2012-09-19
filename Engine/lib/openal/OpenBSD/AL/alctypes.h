#ifndef _ALCTYPES_H_
#define _ALCTYPES_H_

typedef enum {
  ALC_INVALID,

  ALC_FREQUENCY,     /* followed by <int> Hz */
  ALC_RESOLUTION,    /* followed by <int> bits   */

  ALC_BUFFERSIZE,    /* followed by <int> bytes  */
  ALC_CHANNELS,      /* followed by <int> hardware channels */
  /* Angst: differentiate channels by categories */

  ALC_REFRESH,       /* followed by <int> Hz     */
  ALC_MIXAHEAD,      /* followed by <int> msec   */

  ALC_SOURCES,	     /* followed by ### of sources */
  ALC_BUFFERS,	     /* followed by ### of buffers */

  ALC_CD,	     /* do we want to control the CD? */

  ALC_SYNC,	      /* synchronous (need alcUpdateContext) */

  /* errors */
  ALC_NO_ERROR,
  ALC_INVALID_DEVICE,     /* No device */
  ALC_INVALID_CONTEXT     /* invalid context ID */
} ALCenum;

#endif /* _ALCTYPES_H */
