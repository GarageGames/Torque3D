#ifndef NFCFILTER_H
#define NFCFILTER_H

#include "alMain.h"

typedef struct NfcFilter {
    float g;
    float coeffs[MAX_AMBI_ORDER*2 + 1];
    float history[MAX_AMBI_ORDER];
} NfcFilter;

/* NOTE:
 * w0 = speed_of_sound / (source_distance * sample_rate);
 * w1 = speed_of_sound / (control_distance * sample_rate);
 *
 * Generally speaking, the control distance should be approximately the average
 * speaker distance, or based on the reference delay if outputing NFC-HOA. It
 * must not be negative, 0, or infinite. The source distance should not be too
 * small relative to the control distance.
 */

/* Near-field control filter for first-order ambisonic channels (1-3). */
void NfcFilterCreate1(NfcFilter *nfc, const float w0, const float w1);
void NfcFilterAdjust1(NfcFilter *nfc, const float w0);
void NfcFilterUpdate1(NfcFilter *nfc, float *restrict dst, const float *restrict src, const int count);

/* Near-field control filter for second-order ambisonic channels (4-8). */
void NfcFilterCreate2(NfcFilter *nfc, const float w0, const float w1);
void NfcFilterAdjust2(NfcFilter *nfc, const float w0);
void NfcFilterUpdate2(NfcFilter *nfc, float *restrict dst, const float *restrict src, const int count);

/* Near-field control filter for third-order ambisonic channels (9-15). */
void NfcFilterCreate3(NfcFilter *nfc, const float w0, const float w1);
void NfcFilterAdjust3(NfcFilter *nfc, const float w0);
void NfcFilterUpdate3(NfcFilter *nfc, float *restrict dst, const float *restrict src, const int count);

#endif /* NFCFILTER_H */
