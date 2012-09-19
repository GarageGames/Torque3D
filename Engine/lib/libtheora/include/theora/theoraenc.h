/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggTheora SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE Theora SOURCE CODE IS COPYRIGHT (C) 2002-2003                *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

  function:
  last mod: $Id: theora.h,v 1.8 2004/03/15 22:17:32 derf Exp $

 ********************************************************************/

/**\file
 * The <tt>libtheoraenc</tt> C encoding API.*/

#if !defined(_O_THEORA_THEORAENC_H_)
# define _O_THEORA_THEORAENC_H_ (1)
# include <stddef.h>
# include <ogg/ogg.h>
# include "codec.h"

#if defined(__cplusplus)
extern "C" {
#endif



/**\name th_encode_ctl() codes
 * \anchor encctlcodes
 * These are the available request codes for th_encode_ctl().
 * By convention, these are even, to distinguish them from the
 *  \ref decctlcodes "decoder control codes".
 * Keep any experimental or vendor-specific values above \c 0x8000.*/
/*@{*/
/**Sets the Huffman tables to use.
 * The tables are copied, not stored by reference, so they can be freed after
 *  this call.
 * <tt>NULL</tt> may be specified to revert to the default tables.
 *
 * \param[in] _buf <tt>#th_huff_code[#TH_NHUFFMAN_TABLES][#TH_NDCT_TOKENS]</tt>
 * \retval TH_EFAULT \a _enc_ctx is <tt>NULL</tt>.
 * \retval TH_EINVAL Encoding has already begun or one or more of the given
 *                     tables is not full or prefix-free, \a _buf is
 *                     <tt>NULL</tt> and \a _buf_sz is not zero, or \a _buf is
 *                     non-<tt>NULL</tt> and \a _buf_sz is not
 *                     <tt>sizeof(#th_huff_code)*#TH_NHUFFMAN_TABLES*#TH_NDCT_TOKENS</tt>.
 * \retval TH_IMPL   Not supported by this implementation.*/
#define TH_ENCCTL_SET_HUFFMAN_CODES (0)
/**Sets the quantization parameters to use.
 * The parameters are copied, not stored by reference, so they can be freed
 *  after this call.
 * <tt>NULL</tt> may be specified to revert to the default parameters.
 * For the current encoder, <tt>scale[ci!=0][qi]</tt> must be no greater than
 *  <tt>scale[ci!=0][qi-1]</tt> and <tt>base[qti][pli][qi][ci]</tt> must be no
 *  greater than <tt>base[qti][pli][qi-1][ci]</tt>.
 * These two conditions ensure that the actual quantizer for a given \a qti,
 *  \a pli, and \a ci does not increase as \a qi increases.
 *
 * \param[in] _buf #th_quant_info
 * \retval TH_EFAULT \a _enc_ctx is <tt>NULL</tt>.
 * \retval TH_EINVAL Encoding has already begun, the quantization parameters
 *                    do not meet one of the above stated conditions, \a _buf
 *                    is <tt>NULL</tt> and \a _buf_sz is not zero, or \a _buf
 *                    is non-<tt>NULL</tt> and \a _buf_sz is not
 *                    <tt>sizeof(#th_quant_info)</tt>.
 * \retval TH_IMPL   Not supported by this implementation.*/
#define TH_ENCCTL_SET_QUANT_PARAMS (2)
/**Sets the maximum distance between key frames.
 * This can be changed during an encode, but will be bounded by
 *  <tt>1<<th_info#keyframe_granule_shift</tt>.
 * If it is set before encoding begins, th_info#keyframe_granule_shift will
 *  be enlarged appropriately.
 *
 * \param[in]  _buf <tt>ogg_uint32_t</tt>: The maximum distance between key
 *                   frames.
 * \param[out] _buf <tt>ogg_uint32_t</tt>: The actual maximum distance set.
 * \retval TH_EFAULT \a _enc_ctx or \a _buf is <tt>NULL</tt>.
 * \retval TH_EINVAL \a _buf_sz is not <tt>sizeof(ogg_uint32_t)</tt>.
 * \retval TH_IMPL   Not supported by this implementation.*/
#define TH_ENCCTL_SET_KEYFRAME_FREQUENCY_FORCE (4)
/**Disables any encoder features that would prevent lossless transcoding back
 *  to VP3.
 * This primarily means disabling block-level QI values and not using 4MV mode
 *  when any of the luma blocks in a macro block are not coded.
 * It also includes using the VP3 quantization tables and Huffman codes; if you
 *  set them explicitly after calling this function, the resulting stream will
 *  not be VP3-compatible.
 * If you enable VP3-compatibility when encoding 4:2:2 or 4:4:4 source
 *  material, or when using a picture region smaller than the full frame (e.g.
 *  a non-multiple-of-16 width or height), then non-VP3 bitstream features will
 *  still be disabled, but the stream will still not be VP3-compatible, as VP3
 *  was not capable of encoding such formats.
 * If you call this after encoding has already begun, then the quantization
 *  tables and codebooks cannot be changed, but the frame-level features will
 *  be enabled or disabled as requested.
 *
 * \param[in]  _buf <tt>int</tt>: a non-zero value to enable VP3 compatibility,
 *                   or 0 to disable it (the default).
 * \param[out] _buf <tt>int</tt>: 1 if all bitstream features required for
 *                   VP3-compatibility could be set, and 0 otherwise.
 *                  The latter will be returned if the pixel format is not
 *                   4:2:0, the picture region is smaller than the full frame,
 *                   or if encoding has begun, preventing the quantization
 *                   tables and codebooks from being set.
 * \retval TH_EFAULT \a _enc_ctx or \a _buf is <tt>NULL</tt>.
 * \retval TH_EINVAL \a _buf_sz is not <tt>sizeof(int)</tt>.
 * \retval TH_IMPL   Not supported by this implementation.*/
#define TH_ENCCTL_SET_VP3_COMPATIBLE (10)
/**Gets the maximum speed level.
 * Higher speed levels favor quicker encoding over better quality per bit.
 * Depending on the encoding mode, and the internal algorithms used, quality
 *  may actually improve, but in this case bitrate will also likely increase.
 * In any case, overall rate/distortion performance will probably decrease.
 * The maximum value, and the meaning of each value, may change depending on
 *  the current encoding mode (VBR vs. CQI, etc.).
 *
 * \param[out] _buf int: The maximum encoding speed level.
 * \retval TH_EFAULT \a _enc_ctx or \a _buf is <tt>NULL</tt>.
 * \retval TH_EINVAL \a _buf_sz is not <tt>sizeof(int)</tt>.
 * \retval TH_IMPL   Not supported by this implementation in the current
 *                    encoding mode.*/
#define TH_ENCCTL_GET_SPLEVEL_MAX (12)
/**Sets the speed level.
 * By default, the slowest speed (0) is used.
 *
 * \param[in] _buf int: The new encoding speed level.
 *                      0 is slowest, larger values use less CPU.
 * \retval TH_EFAULT \a _enc_ctx or \a _buf is <tt>NULL</tt>.
 * \retval TH_EINVAL \a _buf_sz is not <tt>sizeof(int)</tt>, or the
 *                    encoding speed level is out of bounds.
 *                   The maximum encoding speed level may be
 *                    implementation- and encoding mode-specific, and can be
 *                    obtained via #TH_ENCCTL_GET_SPLEVEL_MAX.
 * \retval TH_IMPL   Not supported by this implementation in the current
 *                    encoding mode.*/
#define TH_ENCCTL_SET_SPLEVEL (14)
/*@}*/



/**The quantization parameters used by VP3.*/
extern const th_quant_info TH_VP31_QUANT_INFO;

/**The Huffman tables used by VP3.*/
extern const th_huff_code
 TH_VP31_HUFF_CODES[TH_NHUFFMAN_TABLES][TH_NDCT_TOKENS];



/**\name Encoder state
   The following data structure is opaque, and its contents are not publicly
    defined by this API.
   Referring to its internals directly is unsupported, and may break without
    warning.*/
/*@{*/
/**The encoder context.*/
typedef struct th_enc_ctx    th_enc_ctx;
/*@}*/



/**\defgroup encfuncs Functions for Encoding*/
/*@{*/
/**\name Functions for encoding
 * You must link to <tt>libtheoraenc</tt> and <tt>libtheoradec</tt>
 *  if you use any of the functions in this section.
 *
 * The functions are listed in the order they are used in a typical encode.
 * The basic steps are:
 * - Fill in a #th_info structure with details on the format of the video you
 *    wish to encode.
 * - Allocate a #th_enc_ctx handle with th_encode_alloc().
 * - Perform any additional encoder configuration required with
 *    th_encode_ctl().
 * - Repeatedly call th_encode_flushheader() to retrieve all the header
 *    packets.
 * - For each uncompressed frame:
 *   - Submit the uncompressed frame via th_encode_ycbcr_in()
 *   - Repeatedly call th_encode_packetout() to retrieve any video data packets
 *      that are ready.
 * - Call th_encode_free() to release all encoder memory.*/
/*@{*/
/**Allocates an encoder instance.
 * \param _info A #th_info struct filled with the desired encoding parameters.
 * \return The initialized #th_enc_ctx handle.
 * \retval NULL If the encoding parameters were invalid.*/
extern th_enc_ctx *th_encode_alloc(const th_info *_info);
/**Encoder control function.
 * This is used to provide advanced control the encoding process.
 * \param _enc    A #th_enc_ctx handle.
 * \param _req    The control code to process.
 *                See \ref encctlcodes "the list of available control codes"
 *                 for details.
 * \param _buf    The parameters for this control code.
 * \param _buf_sz The size of the parameter buffer.*/
extern int th_encode_ctl(th_enc_ctx *_enc,int _req,void *_buf,size_t _buf_sz);
/**Outputs the next header packet.
 * This should be called repeatedly after encoder initialization until it
 *  returns 0 in order to get all of the header packets, in order, before
 *  encoding actual video data.
 * \param _enc      A #th_enc_ctx handle.
 * \param _comments The metadata to place in the comment header, when it is
 *                   encoded.
 * \param _op       An <tt>ogg_packet</tt> structure to fill.
 *                  All of the elements of this structure will be set,
 *                   including a pointer to the header data.
 *                  The memory for the header data is owned by
 *                   <tt>libtheoraenc</tt>, and may be invalidated when the
 *                   next encoder function is called.
 * \return A positive value indicates that a header packet was successfully
 *          produced.
 * \retval 0         No packet was produced, and no more header packets remain.
 * \retval TH_EFAULT \a _enc, \a _comments, or \a _op was <tt>NULL</tt>.*/
extern int th_encode_flushheader(th_enc_ctx *_enc,
 th_comment *_comments,ogg_packet *_op);
/**Submits an uncompressed frame to the encoder.
 * \param _enc   A #th_enc_ctx handle.
 * \param _ycbcr A buffer of Y'CbCr data to encode.
 * \retval 0         Success.
 * \retval TH_EFAULT \a _enc or \a _ycbcr is <tt>NULL</tt>.
 * \retval TH_EINVAL The buffer size does not match the frame size the encoder
 *                    was initialized with, or encoding has already
 *                    completed.*/
extern int th_encode_ycbcr_in(th_enc_ctx *_enc,th_ycbcr_buffer _ycbcr);
/**Retrieves encoded video data packets.
 * This should be called repeatedly after each frame is submitted to flush any
 *  encoded packets, until it returns 0.
 * The encoder will not buffer these packets as subsequent frames are
 *  compressed, so a failure to do so will result in lost video data.
 * \note Currently the encoder operates in a one-frame-in, one-packet-out
 *        manner.
 *       However, this may be changed in the future.
 * \param _enc  A #th_enc_ctx handle.
 * \param _last Set this flag to a non-zero value if no more uncompressed
 *               frames will be submitted.
 *              This ensures that a proper EOS flag is set on the last packet.
 * \param _op   An <tt>ogg_packet</tt> structure to fill.
 *              All of the elements of this structure will be set, including a
 *               pointer to the video data.
 *              The memory for the video data is owned by
 *               <tt>libtheoraenc</tt>, and may be invalidated when the next
 *               encoder function is called.
 * \return A positive value indicates that a video data packet was successfully
 *          produced.
 * \retval 0         No packet was produced, and no more encoded video data
 *                    remains.
 * \retval TH_EFAULT \a _enc or \a _op was <tt>NULL</tt>.*/
extern int th_encode_packetout(th_enc_ctx *_enc,int _last,ogg_packet *_op);
/**Frees an allocated encoder instance.
 * \param _enc A #th_enc_ctx handle.*/
extern void th_encode_free(th_enc_ctx *_enc);
/*@}*/
/*@}*/



#if defined(__cplusplus)
}
#endif

#endif
