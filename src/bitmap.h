#include "pebble.h"

static inline GColor bmpGetPixel(const GBitmap *bmp, int x, int y) {
    if (x >= bmp->bounds.size.w || y >= bmp->bounds.size.h || x < 0 || y < 0) return -1;
    int byteoffset = y*bmp->row_size_bytes + x/8;
    return ((((uint8_t *)bmp->addr)[byteoffset] & (1<<(x%8))) != 0);
}

static inline void bmpPutPixel(GBitmap *bmp, int x, int y, GColor c) {
    if (x >= bmp->bounds.size.w || y >= bmp->bounds.size.h || x < 0 || y < 0) return;
    int byteoffset = y*bmp->row_size_bytes + x/8;
    ((uint8_t *)bmp->addr)[byteoffset] &= ~(1<<(x%8));
    if (c == GColorWhite) ((uint8_t *)bmp->addr)[byteoffset] |= (1<<(x%8));
}

static void bmpCopy(const GBitmap *src, GBitmap *dst) {
    for (int y=0; y<dst->bounds.size.h; y++) {
        for (int x=0; x<dst->bounds.size.w; x++) {
            bmpPutPixel(dst, x, y, bmpGetPixel(src, x, y));
        }
    }
}

static inline void bmpFillRect(GBitmap *bmp, GRect rect, GColor c) {
    int i, j;
    int xe = rect.origin.x + rect.size.w;
    int ye = rect.origin.y + rect.size.h;
 
    for (j=rect.origin.y ; j<ye; j++) {
        for (i=rect.origin.x ; i<xe; i++) {
            bmpPutPixel(bmp, i, j, c);
        }
    }    
}

static inline void bmpFill(GBitmap *bmp, GColor c) {
	int i, l = bmp->row_size_bytes*bmp->bounds.size.h;
	uint8_t p = 0xff * c;
	uint8_t *d = bmp->addr;
	
	for (i=0; i<l; i++) {
		d[i] = p;
	}
}
