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

#include <dmem/vector.h>
#include "test.h"

DVECTOR_INIT(int, int);

static int generations;
static d_vector(int) generation;

static d_slice(int) generate() {
	dv_resize(&generation, 3);
	generation.data[0] = 1;
	generation.data[1] = 2;
	generation.data[2] = 3;
	generations++;
	return generation;
}

int main(void)
{
	d_vector(int) v = DV_INIT;
	int *p;
	int int3[3] = {1,2,3};
	int idx;

	check_int(v.size, 0);
	check(v.data == NULL);

	dv_reserve(&v, 1);
	check(v.data != NULL);
	check_int(v.size, 0);
	check(dv_reserved(v) >= 1);

	dv_free(v);
	dv_init(&v);
	check_int(v.size, 0);
	check(v.data == NULL);

	dv_resize(&v, 2);
	check_int(v.size, 2);
	check(v.data != NULL);
	v.data[0] = 50000;
	v.data[1] = 50000;

	dv_resize(&v, 1);
	check_int(v.size, 1);
	check_int(v.data[0], 50000);
	check_int(v.data[1], 0);

	dv_clear(&v);
	check_int(v.size, 0);
	check(v.data != NULL);
	check_int(v.data[0], 0);

	dv_resize(&v, 3);
	v.data[0] = 1;
	v.data[1] = 2;
	v.data[2] = 3;
	p = dv_insert_buffer(&v, 1, 2);
    check(p == &v.data[1]);
	check_int(v.size, 5);
	check_int(v.data[0], 1);
	check_int(v.data[1], 2);
	check_int(v.data[2], 3);
	check_int(v.data[3], 2);
	check_int(v.data[4], 3);

	dv_resize(&v, 2);
	p = dv_insert_zeroed(&v, 1, 2);
    check(p == &v.data[1]);
	check_int(v.size, 4);
	check_int(v.data[0], 1);
	check_int(v.data[1], 0);
	check_int(v.data[2], 0);
	check_int(v.data[3], 2);

	dv_resize(&v, 5);
	v.data[4] = 4;
	dv_resize(&v, 1);
	p = dv_append_buffer(&v, 4);
    check(p == &v.data[1]);
	check_int(v.size, 5);
	check_int(v.data[4], 4);

	dv_resize(&v, 5);
	v.data[4] = 4;
	dv_resize(&v, 1);
	p = dv_append_zeroed(&v, 4);
    check(p == &v.data[1]);
	check_int(v.size, 5);
	check_int(v.data[4], 0);

	dv_resize(&v, 1);
	v.data[0] = 64;
	dv_append2(&v, int3, 3);
	check_int(v.size, 4);
	check_int(v.data[0], 64);
	check_int(v.data[1], 1);
	check_int(v.data[2], 2);
	check_int(v.data[3], 3);

	dv_resize(&v, 1);
	dv_append1(&v, 56);
	check_int(v.size, 2);
	check_int(v.data[1], 56);
	check_int(v.data[2], 0);

	dv_resize(&v, 1);
	generations = 0;
	v.data[0] = 64;
	dv_append(&v, generate());
	check_int(generations, 1);
	check_int(v.size, 4);
	check_int(v.data[0], 64);
	check_int(v.data[1], 1);
	check_int(v.data[2], 2);
	check_int(v.data[3], 3);

	dv_resize(&v, 1);
	v.data[0] = 64;
	dv_set2(&v, int3, 3);
	check_int(v.size, 3);
	check_int(v.data[0], 1);
	check_int(v.data[1], 2);
	check_int(v.data[2], 3);

	dv_resize(&v, 1);
	generations = 0;
	v.data[0] = 64;
	dv_set(&v, generate());
	check_int(generations, 1);
	check_int(v.size, 3);
	check_int(v.data[0], 1);
	check_int(v.data[1], 2);
	check_int(v.data[2], 3);

	dv_resize(&v, 2);
	generations = 0;
	v.data[0] = 64;
	v.data[1] = 36;
	dv_insert(&v, 1, generate());
	check_int(generations, 1);
	check_int(v.size, 5);
	check_int(v.data[0], 64);
	check_int(v.data[1], 1);
	check_int(v.data[2], 2);
	check_int(v.data[3], 3);
	check_int(v.data[4], 36);

	dv_resize(&v, 2);
	generations = 0;
	v.data[0] = 64;
	v.data[1] = 36;
	dv_insert2(&v, 1, int3, 3);
	check_int(v.size, 5);
	check_int(v.data[0], 64);
	check_int(v.data[1], 1);
	check_int(v.data[2], 2);
	check_int(v.data[3], 3);
	check_int(v.data[4], 36);

	dv_erase(&v, 1, 3);
	check_int(v.size, 2);
	check_int(v.data[0], 64);
	check_int(v.data[1], 36);

	dv_resize(&v, 3);
	v.data[0] = 45;
	v.data[1] = 35;
	v.data[2] = 45;
	dv_remove(&v, 45);
	check_int(v.size, 1);
	check_int(v.data[0], 35);

	dv_resize(&v, 3);
	v.data[0] = 45;
	v.data[1] = 35;
	v.data[2] = 45;
	dv_find(v, 35, &idx);
	check_int(idx, 1);
	dv_find(v, 55, &idx);
	check_int(idx, -1);

	dv_resize(&v, 3);
	v.data[0] = 1;
	v.data[1] = 2;
	v.data[2] = 3;
	check(dv_begins_with(v, generate()));
	check(dv_equals(v, generate()));
	check(dv_cmp(v, generate()) == 0);
	check(dv_ends_with(v, generate()));

	dv_resize(&v, 4);
	v.data[3] = 5;
	check(dv_begins_with(v, generate()));
	check(dv_cmp(v, generate()) > 0);
	check(!dv_ends_with(v, generate()));

	v.data[2] = 1;
	check(!dv_begins_with(v, generate()));
	check(dv_cmp(v, generate()) < 0);

	dv_resize(&v, 4);
	v.data[0] = 5;
	v.data[1] = 1;
	v.data[2] = 2;
	v.data[3] = 3;
	check(dv_ends_with(v, generate()));

	v.data[2] = 5;
	check(!dv_ends_with(v, generate()));

	return 0;
}
