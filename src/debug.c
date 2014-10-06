

#ifndef min
#   define min(x, y) ((x < y) ? (x) : (y))
#endif

#define dv_isprint(ch) (' ' < ch && ch <= '~')

#define BYTES_PER_LINE ((24 /*header*/ + 8*20 /*hex*/ + 4*4/*hex spaces*/ + 6*16/*ascii*/ + 3/*ascii spaces*/ + 3/*round up for spaces and \n*/) & ~3)

static const char lookup_hex[] = "0123456789abcdef";
#define hex(p, v) ((p)[0] = lookup_hex[((v) & 0xFF) >> 4], (p)[1] = lookup_hex[v & 0x0F], (p) + 2)

#if __LITTLE_ENDIAN__ || __ARMEL__ || defined __i386__ || defined __amd64__ || defined _M_IX86 || defined _M_X64
#define u32(p, h, mh, ml, l) (*((uint32_t*) p) = ((uint32_t) (h)) | ((uint32_t) (mh) << 8) | ((uint32_t) (ml) << 16) | ((uint32_t) (l) << 24), (p) + 4)
#elif __BIG_ENDIAN__ || __ARMEB__
#define u32(p, h, mh, ml, l) (*((uint32_t*) p) = ((uint32_t) (h) << 24) | ((uint32_t) (mh) << 16) | ((uint32_t) (ml) << 8) | ((uint32_t) (l)), (p) + 4)
#else
#error
#endif

#define NORMAL 0
#define RED 1
#define YELLOW 2
#define CYAN 3

static char* set_normal(char* p, int* color)
{
    if (*color != NORMAL) {
        *color = NORMAL;
        p = u32(p, '\033', '[', '0', 'm');
    }
    return p;
}

static char* set_red(char* p, int* color)
{
    if (*color != RED) {
        *color = RED;
        p = u32(p, '\033', '[', 'm', '\033');
        p = u32(p, '[', '3', '1', 'm');
    }
    return p;
}

static char* do_line(char* p, int off, const char* d, int n, int* color)
{
    uint8_t* u = (uint8_t*) d;
    int i;

    /* 8 bytes */
    if (color) {
        *color = CYAN;
        p = u32(p, '\t', '\033', '[', '3');
        p = u32(p, '6', 'm', '0', 'x');
    } else {
        p = u32(p, ' ', '\t', '0', 'x');
    }

    /* 8 bytes */
    p = hex(p, off >> 8);
    p = hex(p, off);

    /* 8 bytes */
    p = set_normal(p, color);
    p = u32(p, ' ', ' ', ' ', ' ');

    /* up to 20 bytes per loop */
    for (i = 0; i+2 <= n; i+= 2) {
        if ((i & 3) == 0 && i > 0) {
            p = u32(p, ' ', '\b', ' ', ' ');
        }

        if (!color) {
            /* abcd - 4 bytes */
            p = hex(p, u[i]);
            p = hex(p, u[i+1]);

        } else if (dv_isprint(d[i]) && dv_isprint(d[i+1])) {
            /* NORMAL abcd - 8 bytes */
            p = set_normal(p, color);
            p = hex(p, u[i]);
            p = hex(p, u[i+1]);

        } else if (!dv_isprint(d[i]) && !dv_isprint(d[i+1])) {
            /* RED abcd - 12 bytes */
            p = set_red(p, color);
            p = hex(p, u[i]);
            p = hex(p, u[i+1]);

        } else if (dv_isprint(u[i])) {
            /* NORMAL ab RED cd - 20 bytes */
            p = set_normal(p, color);
            p = hex(p, u[i]);
            *(p++) = ' ';
            *(p++) = '\b';
            p = set_red(p, color);
            p = hex(p, u[i+1]);
            *(p++) = ' ';
            *(p++) = '\b';

        } else {
            /* RED ab NORMAL cd - 20 bytes */
            p = set_red(p, color);
            p = hex(p, u[i]);
            *(p++) = ' ';
            *(p++) = '\b';
            p = set_normal(p, color);
            p = hex(p, u[i+1]);
            *(p++) = ' ';
            *(p++) = '\b';
        }
    }

    /* Pad out to an even number of bytes - not used in worst case */
    if (n & 1) {
        if ((i & 3) == 0) {
            p = u32(p, ' ', '\b', ' ', ' ');
        }

        if (color && dv_isprint(d[i])) {
            p = set_normal(p, color);
        } else if (color) {
            p = set_red(p, color);
        }

        p = hex(p, u[i]);
        *(p++) = ' ';
        *(p++) = ' ';
        i += 2;
    }

    /* Pad out to 16 bytes - not used in worst case */
    for (; i < 16; i += 2) {
        if ((i & 3) == 0) {
            p = u32(p, ' ', '\b', ' ', ' ');
        }
        p = u32(p, ' ', ' ', ' ', ' ');
    }

    p = u32(p, ' ', ' ', ' ', ' ');

    /* up to 6 bytes per loop */
    for (i = 0; i < n; i++) {
        if ((i & 3) == 0 && i > 0) {
            *(p++) = ' ';
        }

        if (!color) {
            *(p++) = dv_isprint(d[i]) ? d[i] : '.';

        } else if (dv_isprint(d[i])) {
            if (*color != NORMAL) {
                *color = NORMAL;
                *(p++) = '\033';
                *(p++) = '[';
                *(p++) = 'm';
            }
            *(p++) = d[i];

        } else if (d[i] == '\0' || d[i] == '\n') {
            if (*color != YELLOW) {
                *color = YELLOW;
                *(p++) = '\033';
                *(p++) = '[';
                *(p++) = '3';
                *(p++) = '3';
                *(p++) = 'm';
            }
            *(p++) = d[i] ? 'N' : '0';

        } else {
            if (*color != RED) {
                *color = RED;
                *(p++) = '\033';
                *(p++) = '[';
                *(p++) = '3';
                *(p++) = '1';
                *(p++) = 'm';
            }
            *(p++) = '.';
        }
    }

    /* realign up to 4 bytes */
    switch ((uintptr_t) p & 3) {
    case 3:
        *(p++) = '\n';
        break;

    case 2:
        *(p++) = ' ';
        *(p++) = '\n';
        break;

    case 1:
        *(p++) = ' ';
        *(p++) = '\b';
        *(p++) = '\n';
        break;

    case 0:
    default:
        if (color) {
            *color = NORMAL;
            p = u32(p, '\033', '[', 'm', '\n');
        } else {
            p = u32(p, ' ', '\b', ' ', '\n');
        }
        break;
    }

    return set_normal(p, color);
}

void dv_hex_dump(d_vector(char)* s, d_string data, bool colors)
{
    int i, color;

    if (!data.size) {
        return;
    }

    /* align to a 4 byte boundary with spaces that are consumed by the first
     * tab */
    switch (s->size & 3) {
    case 1:
        dv_append1(s, ' ');
    case 2:
        dv_append1(s, ' ');
    case 3:
        dv_append1(s, ' ');
    default:
        break;
    }

    for (i = 0; i < data.size; i += 16) {
        int n = min(16, data.size - i);
        char* p = dv_reserve(s, BYTES_PER_LINE);
        char* e = do_line(p, i, &data.data[i], n, colors ? &color : NULL);
        assert(e - p <= BYTES_PER_LINE);
        dv_resize(s, (int) (e - data.data));
    }
}

void dv_log(d_string data, const char* format, ...)
{
    /* use a uint32_t to ensure the buffer is 4 byte aligned */
    uint32_t buf[BYTES_PER_LINE/4];
    int i, color;
    va_list ap;
    static int colored = -1;

    if (colored == -1) {
#if defined _WIN32 || defined __UCLIBC__
        colored = 0;
#else
        colored = isatty(fileno(stderr));
#endif
    }

    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    fputc('\n', stderr);

    for (i = 0; i < data.size; i += 16) {
        int n = min(16, data.size - i);
        char* e = do_line((char*)&buf[0], i, &data.data[i], n, colored ? &color : NULL);
        assert(e - (char*)&buf[0] <= BYTES_PER_LINE);
        fwrite(buf, 1, e - (char*)&buf[0], stderr);
    }
}

