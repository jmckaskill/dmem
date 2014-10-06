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

#include <dmem/char.h>
#include "test.h"

extern dv_char_mask dv_url_mask;
extern dv_char_mask dv_quote_mask;

int main(void)
{
    d_vector(char) p = DV_INIT;
    d_string s;
    dv_char_mask u;
    int i;

    u = dv_create_mask(C("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_~."));
    for (i = 0; i < 8; i++) {
        check_int(u.d[i], dv_url_mask.d[i]);
    }

    u = dv_create_mask(C(" '#%&*+,-./:;<=>?@[]^_{|}~()!-_~.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"));
    for (i = 0; i < 4; i++) {
        check_int(u.d[i], dv_quote_mask.d[i]);
    }

    dv_clear(&p);
    dv_hex_encode(&p, C("\x05\x0F\xFF\x00\x01"));
    check_string(p, C("050fff0001"));

    dv_clear(&p);
    dv_hex_decode(&p, C("050fff0001"));
    check_string(p, C("\x05\x0F\xFF\x00\x01"));

    dv_clear(&p);
    dv_quote(&p, C("foo \xC2\xB1\n\x01""b'\""), NULL);
    check_string(p, C("\"foo \xC2\xB1\\n\\x01b'\\\"\""));

    dv_clear(&p);
    u = dv_create_mask(C("fo"));
    dv_quote(&p, C("foob"), &u);
    check_string(p, C("\"foo\\x62\""));

    dv_clear(&p);
    dv_url_decode(&p, C("foo%20-%7B"));
    check_string(p, C("foo -\x7B"));

    dv_clear(&p);
    dv_url_encode(&p, C("foo -\x7B/~"), NULL);
    check_string(p, C("foo%20-%7B%2F~"));

    dv_clear(&p);
    u = dv_create_mask(C("fo"));
    dv_url_encode(&p, C("foob"), &u);
    check_string(p, C("foo%62"));

    dv_set(&p, C("stringg with\x00things\n\nto find"));

    s = p;
    check_string(dv_split_char(&s, 't'), C("s"));
    check_int(s.size, p.size - 2);
    check_int(s.data[0], 'r');
    s = p;
    check_string(dv_split_char(&s, 'g'), C("strin"));
    check_int(s.data[0], 'g');
    check_string(dv_split_char(&s, 'g'), C(""));
    check_int(s.data[0], ' ');
    check_string(dv_split_char(&s, '\x00'), C(" with"));
    check_int(s.data[0], 't');
    check_string(dv_split_char(&s, '\r'), C("things\n\nto find"));
    check_int(s.size, 0);

    s = p;
    check_string(dv_split_one_of(&s, C("\x00h")), C("stringg wit"));
    check_int(s.data[0], '\x00');
    check_string(dv_split_one_of(&s, C("\x00h")), C(""));
    check_int(s.data[0], 't');
    check_string(dv_split_one_of(&s, C("\t")), C("things\n\nto find"));
    check_int(s.size, 0);

    s = p;
    check_string(dv_split_string(&s, C("things")), C("stringg with\x00"));
    check_int(s.data[0], '\n');
    check_string(dv_split_string(&s, C("things")), C("\n\nto find"));
    check_int(s.size, 0);

    dv_set(&p, C(" foo   \t  \r\n  "));
    check_string(dv_strip_whitespace(p), C("foo"));

    dv_set(&p, C("0123456789"));
    check_string(dv_left(p, 2), C("01"));
    check_string(dv_right(p, 7), C("789"));
    check_string(dv_slice(p, 2, 2), C("23"));

    check_int(dv_to_integer(C(" -234 "), 10, 0), -234);
    check(dv_to_number(C(" 234.1")) == 234.1);

    dv_free(p);
    dv_init(&p);
    dv_set(&p, C("23"));
    dv_print(&p, "foo %d %s", 2, "bar");
    check_string(p, C("23foo 2 bar"));

#define TEST(pfx, from, to) \
    dv_set(&p, C(pfx)); \
    check_string((dv_clean_path(&p, C(from)), p), C(to))

    TEST("# ", "", "# .");
    TEST("# ", ".", "# .");
    TEST("# ", "..", "# ..");
    TEST("# ", "foo", "# foo");
    TEST("# ", "/", "# /");
    TEST("# ", "/.", "# /");
    TEST("# ", "/..", "# /");
    TEST("# ", "/foo", "# /foo");
    TEST("# ", "./", "# .");
    TEST("# ", "./.", "# .");
    TEST("# ", "./..", "# ..");
    TEST("# ", "./foo", "# foo");
    TEST("# ", "../", "# ..");
    TEST("# ", "../.", "# ..");
    TEST("# ", "../..", "# ../..");
    TEST("# ", "../foo", "# ../foo");
    TEST("# ", "bar/", "# bar");
    TEST("# ", "bar/.", "# bar");
    TEST("# ", "bar/..", "# .");
    TEST("# ", "bar/foo", "# bar/foo");

#undef TEST
#define TEST(pfx, a, b, result) \
    dv_set(&p, C(pfx)); \
    dv_append(&p, C(a)); \
    check_string((dv_join_path(&p, p.size - strlen(a), C(b)), p), C(result))

    TEST("# ", ".", "", "# .");
    TEST("# ", ".", ".", "# .");
    TEST("# ", ".", "..", "# ..");
    TEST("# ", ".", "foo", "# foo");
    TEST("# ", ".", "/", "# /");
    TEST("# ", ".", "/.", "# /");
    TEST("# ", ".", "/..", "# /");
    TEST("# ", ".", "/foo", "# /foo");
    TEST("# ", ".", "./", "# .");
    TEST("# ", ".", "./.", "# .");
    TEST("# ", ".", "./..", "# ..");
    TEST("# ", ".", "./foo", "# foo");
    TEST("# ", ".", "../", "# ..");
    TEST("# ", ".", "../.", "# ..");
    TEST("# ", ".", "../..", "# ../..");
    TEST("# ", ".", "../foo", "# ../foo");
    TEST("# ", ".", "bar/", "# bar");
    TEST("# ", ".", "bar/.", "# bar");
    TEST("# ", ".", "bar/..", "# .");
    TEST("# ", ".", "bar/foo", "# bar/foo");

    TEST("# ", "..", "", "# ..");
    TEST("# ", "..", ".", "# ..");
    TEST("# ", "..", "..", "# ../..");
    TEST("# ", "..", "foo", "# ../foo");
    TEST("# ", "..", "/", "# /");
    TEST("# ", "..", "/.", "# /");
    TEST("# ", "..", "/..", "# /");
    TEST("# ", "..", "/foo", "# /foo");
    TEST("# ", "..", "./", "# ..");
    TEST("# ", "..", "./.", "# ..");
    TEST("# ", "..", "./..", "# ../..");
    TEST("# ", "..", "./foo", "# ../foo");
    TEST("# ", "..", "../", "# ../..");
    TEST("# ", "..", "../.", "# ../..");
    TEST("# ", "..", "../..", "# ../../..");
    TEST("# ", "..", "../foo", "# ../../foo");
    TEST("# ", "..", "bar/", "# ../bar");
    TEST("# ", "..", "bar/.", "# ../bar");
    TEST("# ", "..", "bar/..", "# ..");
    TEST("# ", "..", "bar/foo", "# ../bar/foo");


    TEST("# ", "var", "", "# var");
    TEST("# ", "var", ".", "# var");
    TEST("# ", "var", "..", "# .");
    TEST("# ", "var", "foo", "# var/foo");
    TEST("# ", "var", "/", "# /");
    TEST("# ", "var", "/.", "# /");
    TEST("# ", "var", "/..", "# /");
    TEST("# ", "var", "/foo", "# /foo");
    TEST("# ", "var", "./", "# var");
    TEST("# ", "var", "./.", "# var");
    TEST("# ", "var", "./..", "# .");
    TEST("# ", "var", "./foo", "# var/foo");
    TEST("# ", "var", "../", "# .");
    TEST("# ", "var", "../.", "# .");
    TEST("# ", "var", "../..", "# ..");
    TEST("# ", "var", "../foo", "# foo");
    TEST("# ", "var", "bar/", "# var/bar");
    TEST("# ", "var", "bar/.", "# var/bar");
    TEST("# ", "var", "bar/..", "# var");
    TEST("# ", "var", "bar/foo", "# var/bar/foo");

    TEST("# ", "/", "", "# /");
    TEST("# ", "/", ".", "# /");
    TEST("# ", "/", "..", "# /");
    TEST("# ", "/", "foo", "# /foo");
    TEST("# ", "/", "/", "# /");
    TEST("# ", "/", "/.", "# /");
    TEST("# ", "/", "/..", "# /");
    TEST("# ", "/", "/foo", "# /foo");
    TEST("# ", "/", "./", "# /");
    TEST("# ", "/", "./.", "# /");
    TEST("# ", "/", "./..", "# /");
    TEST("# ", "/", "./foo", "# /foo");
    TEST("# ", "/", "../", "# /");
    TEST("# ", "/", "../.", "# /");
    TEST("# ", "/", "../..", "# /");
    TEST("# ", "/", "../foo", "# /foo"); /* this one may be a bit implementation dependant */
    TEST("# ", "/", "bar/", "# /bar");
    TEST("# ", "/", "bar/.", "# /bar");
    TEST("# ", "/", "bar/..", "# /");
    TEST("# ", "/", "bar/foo", "# /bar/foo");

    TEST("# ", "/boo", "", "# /boo");
    TEST("# ", "/boo", ".", "# /boo");
    TEST("# ", "/boo", "..", "# /");
    TEST("# ", "/boo", "foo", "# /boo/foo");
    TEST("# ", "/boo", "/", "# /");
    TEST("# ", "/boo", "/.", "# /");
    TEST("# ", "/boo", "/..", "# /");
    TEST("# ", "/boo", "/foo", "# /foo");
    TEST("# ", "/boo", "./", "# /boo");
    TEST("# ", "/boo", "./.", "# /boo");
    TEST("# ", "/boo", "./..", "# /");
    TEST("# ", "/boo", "./foo", "# /boo/foo");
    TEST("# ", "/boo", "../", "# /");
    TEST("# ", "/boo", "../.", "# /");
    TEST("# ", "/boo", "../..", "# /");
    TEST("# ", "/boo", "../foo", "# /foo");
    TEST("# ", "/boo", "bar/", "# /boo/bar");
    TEST("# ", "/boo", "bar/.", "# /boo/bar");
    TEST("# ", "/boo", "bar/..", "# /boo");
    TEST("# ", "/boo", "bar/foo", "# /boo/bar/foo");

    TEST("# ", "../..", "", "# ../..");
    TEST("# ", "../..", ".", "# ../..");
    TEST("# ", "../..", "..", "# ../../..");
    TEST("# ", "../..", "foo", "# ../../foo");
    TEST("# ", "../..", "/", "# /");
    TEST("# ", "../..", "/.", "# /");
    TEST("# ", "../..", "/..", "# /");
    TEST("# ", "../..", "/foo", "# /foo");
    TEST("# ", "../..", "./", "# ../..");
    TEST("# ", "../..", "./.", "# ../..");
    TEST("# ", "../..", "./..", "# ../../..");
    TEST("# ", "../..", "./foo", "# ../../foo");
    TEST("# ", "../..", "../", "# ../../..");
    TEST("# ", "../..", "../.", "# ../../..");
    TEST("# ", "../..", "../..", "# ../../../..");
    TEST("# ", "../..", "../foo", "# ../../../foo");
    TEST("# ", "../..", "bar/", "# ../../bar");
    TEST("# ", "../..", "bar/.", "# ../../bar");
    TEST("# ", "../..", "bar/..", "# ../..");
    TEST("# ", "../..", "bar/foo", "# ../../bar/foo");

    TEST("# ", "../boo", "", "# ../boo");
    TEST("# ", "../boo", ".", "# ../boo");
    TEST("# ", "../boo", "..", "# ..");
    TEST("# ", "../boo", "foo", "# ../boo/foo");
    TEST("# ", "../boo", "/", "# /");
    TEST("# ", "../boo", "/.", "# /");
    TEST("# ", "../boo", "/..", "# /");
    TEST("# ", "../boo", "/foo", "# /foo");
    TEST("# ", "../boo", "./", "# ../boo");
    TEST("# ", "../boo", "./.", "# ../boo");
    TEST("# ", "../boo", "./..", "# ..");
    TEST("# ", "../boo", "./foo", "# ../boo/foo");
    TEST("# ", "../boo", "../", "# ..");
    TEST("# ", "../boo", "../.", "# ..");
    TEST("# ", "../boo", "../..", "# ../..");
    TEST("# ", "../boo", "../foo", "# ../foo");
    TEST("# ", "../boo", "bar/", "# ../boo/bar");
    TEST("# ", "../boo", "bar/.", "# ../boo/bar");
    TEST("# ", "../boo", "bar/..", "# ../boo");
    TEST("# ", "../boo", "bar/foo", "# ../boo/bar/foo");

    TEST("# ", "abc/cde", "", "# abc/cde");
    TEST("# ", "abc/cde", ".", "# abc/cde");
    TEST("# ", "abc/cde", "..", "# abc");
    TEST("# ", "abc/cde", "foo", "# abc/cde/foo");
    TEST("# ", "abc/cde", "/", "# /");
    TEST("# ", "abc/cde", "/.", "# /");
    TEST("# ", "abc/cde", "/..", "# /");
    TEST("# ", "abc/cde", "/foo", "# /foo");
    TEST("# ", "abc/cde", "./", "# abc/cde");
    TEST("# ", "abc/cde", "./.", "# abc/cde");
    TEST("# ", "abc/cde", "./..", "# abc");
    TEST("# ", "abc/cde", "./foo", "# abc/cde/foo");
    TEST("# ", "abc/cde", "../", "# abc");
    TEST("# ", "abc/cde", "../.", "# abc");
    TEST("# ", "abc/cde", "../..", "# .");
    TEST("# ", "abc/cde", "../foo", "# abc/foo");
    TEST("# ", "abc/cde", "bar/", "# abc/cde/bar");
    TEST("# ", "abc/cde", "bar/.", "# abc/cde/bar");
    TEST("# ", "abc/cde", "bar/..", "# abc/cde");
    TEST("# ", "abc/cde", "bar/foo", "# abc/cde/bar/foo");


    return 0;
}


