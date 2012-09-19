#ifndef MD5_H
#define MD5_H


struct MD5Context {
        int buf[4];
        int bits[2];
        unsigned char in[64];
};

extern void MD5Init(struct MD5Context *ctx);
extern void MD5Update(struct MD5Context *ctx, unsigned char *buf, unsigned len);
extern void MD5Final(unsigned char digest[16], struct MD5Context *ctx);
extern void MD5Transform(int but[4], int in[16]);

typedef MD5Context MD5_CTX;

#endif /* !MD5_H */
