// https://codereview.stackexchange.com/questions/8213/bitmap-implementation/8223?fbclid=IwAR1cZYeiD--kXlixY1rY8evqmVKBwAgldwF1Pksg1hDg6m656ENJnoPyU5Y#8223

#include "bitmap.h"

static bool get  (byte,   byte);
static void set  (byte *, byte);
static void reset(byte *, byte);

/* CAREFUL WITH pos AND BITMAP SIZE! */

bool bitmapGet(byte *bitmap, int pos) {
/* gets the value of the bit at pos */
    return get(bitmap[pos/BIT], pos%BIT);
}

// Set to unavailable
void bitmapSet(byte *bitmap, int pos) {
/* sets bit at pos to 0 */
    reset(&bitmap[pos/BIT], pos%BIT);
}

// Set bit to available
void bitmapReset(byte *bitmap, int pos) {
/* sets bit at pos to 1 */
    set(&bitmap[pos/BIT], pos%BIT);
}

int bitmapSearch(byte *bitmap, bool n, int size, int start) {
/* Finds the first n value in bitmap after start */
/* size is the Bitmap size in bytes */
    int i;
    /* size is now the Bitmap size in bits */
    for(i = start+1, size *= BIT; i < size; i++)
        if(bitmapGet(bitmap,i) == n)
            return i;
    return BITMAP_NOTFOUND;
}

static bool get(byte a, byte pos) {
/* pos is something from 0 to 7*/
    return (a >> pos) & 1;
}

static void set(byte *a, byte pos) {
/* pos is something from 0 to 7*/
/* sets bit to 1 */
    *a |= 1 << pos;
}

static void reset(byte *a, byte pos) {
/* pos is something from 0 to 7*/
/* sets bit to 0 */
    *a &= ~(1 << pos);
}