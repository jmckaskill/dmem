/* vim: ts=4 sw=4 sts=4 et
 *
 * Copyright (c) 2009 James R. McKaskill
 *
 * This software is licensed under the stock MIT license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * ----------------------------------------------------------------------------
 */

#define DMEM_LIBRARY
#include <dmem/zlib.h>

int dz_deflate(d_Vector(char)* out, z_stream* z, d_Slice(char) str, int flush)
{
    int err;
    int bufsz = deflateBound(z, str.size);

    if (bufsz < 512) {
        bufsz = 512;
    }

    z->next_in = (uint8_t*) str.data;
    z->avail_in = str.size;

    for (;;) {
        dv_reserve(out, bufsz);
        z->next_out = (uint8_t*) out->data + out->size;
        z->avail_out = dv_reserved(*out) - out->size;

        err = deflate(z, flush);

        // Someone trying to force a flush with a empty string, but the stream
        // was already flushed.
        if (err == Z_BUF_ERROR && str.size == 0) {
            err = Z_OK;
        }

        if (err != Z_OK && err != Z_STREAM_END) {
            return err;
        }

        dv_resize(out, z->next_out - (uint8_t*) out->data);

        if (z->avail_out > 0 || z->avail_in > 0) {
            continue;
        }

        if (err == (flush == Z_FINISH ? Z_STREAM_END : Z_OK)) {
            return 0;
        }
    }
}

int dz_inflate(d_Vector(char)* out, z_stream* z, d_Slice(char) str)
{
    int err;
    int bufsz = str.size * 2;

    z->next_in = (uint8_t*) str.data;
    z->avail_in = str.size;

    for (;;) {
        dv_reserve(out, bufsz);
        z->next_out = (uint8_t*) out->data + out->size;
        z->avail_out = dv_reserved(*out) - out->size;

        err = inflate(z, Z_SYNC_FLUSH);
        if (err != Z_OK && err != Z_STREAM_END) {
            return err;
        }

        dv_resize(out, z->next_out - (uint8_t*) out->data);

        if (z->avail_in == 0) {
            return err;
        }
    }
}

