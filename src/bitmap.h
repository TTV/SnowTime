#include "pebble.h"

static inline GColor8 bmpGetPixel(const GBitmap *bmp, int x, int y) {
    int clr;
    GRect r = gbitmap_get_bounds(bmp);
    if (gbitmap_get_format(bmp) != GBitmapFormat8Bit)
        clr = -1;
    else if (x >= r.size.w || y >= r.size.h || x < 0 || y < 0)
        clr = -1;
    else
        clr = gbitmap_get_data(bmp)[y * gbitmap_get_bytes_per_row(bmp) + x];
    return (GColor8){.argb = clr};
}

static inline void bmpPutPixel(GBitmap *bmp, int x, int y, GColor8 c) {
    GRect r = gbitmap_get_bounds(bmp);
    if (gbitmap_get_format(bmp) != GBitmapFormat8Bit)
        return;
    if (x >= r.size.w || y >= r.size.h || x < 0 || y < 0)
        return;
    gbitmap_get_data(bmp)[y * gbitmap_get_bytes_per_row(bmp) + x] = c.argb;
}

static void bmpCopy(const GBitmap *src, GBitmap *dst) {
    if (gbitmap_get_format(dst) != GBitmapFormat8Bit)
        return;
    if (gbitmap_get_format(src) != GBitmapFormat8Bit) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Bad SRC img format %d, should be %d", gbitmap_get_format(src), GBitmapFormat8Bit);
        return;
    }
    GRect r = gbitmap_get_bounds(dst);
    for (int y = 0; y < r.size.h; y++) {
        for (int x = 0; x < r.size.w; x++) {
            bmpPutPixel(dst, x, y, bmpGetPixel(src, x, y));
        }
    }
}

static inline void bmpFillRect(GBitmap *bmp, GRect rect, GColor c) {
    if (gbitmap_get_format(bmp) != GBitmapFormat8Bit)
        return;

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
    if (gbitmap_get_format(bmp) != GBitmapFormat8Bit)
        return;

    GRect r = gbitmap_get_bounds(bmp);
	int i, l = gbitmap_get_bytes_per_row(bmp) * r.size.h;
	uint8_t *d = gbitmap_get_data(bmp);
	
	for (i=0; i<l; i++) {
		d[i] = c.argb;
	}
}
