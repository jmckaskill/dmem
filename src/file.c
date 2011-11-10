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

/* To get off64_t and d_type */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define DMEM_LIBRARY
#include <dmem/file.h>
#include <limits.h>

#ifdef _WIN32
#include <dmem/wchar.h>
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#if _XOPEN_SOURCE >= 700 || _POSIX_C_SOURCE >= 200809L
#define HAVE_OPENAT
#endif

/* -------------------------------------------------------------------------- */

int dv_read(d_Vector(char)* v, int fd)
{
    int begin = v->size;
    struct stat st;

    if (fstat(fd, &st)) {
        return -1;
    }

    if (st.st_size + begin > INT_MAX) {
        st.st_size = INT_MAX - begin;
    }

    dv_reserve(v, begin + st.st_size);

    for (;;) {
        int r = read(fd, v->out + v->size, dv_reserved(v) - v->size);

        if (r < 0 && errno == EINTR) {
            r = 0;
        } else if (r < 0) {
            dv_resize(v, begin);
            return -1;
        } else if (r == 0) {
            break;
        }

        dv_resize(v, v->size + r);
    }

    return v->size - begin;
}

/* -------------------------------------------------------------------------- */

int dv_read_file(d_Vector(char)* v, d_Slice(char) path, dv_dir* dir)
{
    int begin = v->size;
    int fd, ret;

    /* Append the path to the vector to ensure its null terminated */

#ifdef HAVE_OPENAT
    if (dir && dir->fd >= 0) {
        dv_append(v, path);
        fd = openat(dir->fd, v->data + begin, O_RDONLY | O_CLOEXEC);
    } else
#endif
    if (dir && dir->path.size) {
        dv_clean_path(v, dir->path);
        dv_join_path(v, begin, path);
        fd = open(v->data + begin, flags);
    } else {
        dv_clean_path(v, path);
        fd = open(v->data + begin, flags);
    }

    dv_resize(v, begin);

    if (fd < 0) {
        return -1;
    }

    ret = dv_read(v, fd);
    close(fd);
    return ret;
}

/* -------------------------------------------------------------------------- */

int dv_open_dir(dv_dir* d, d_Slice(char) path)
{
    DIR* dir;
    int fd = -1;
    d_Vector(char) v = DV_INIT;

#ifdef HAVE_OPENAT
    if (dir && dir->fd >= 0) {
        dv_append(&v, path);
        fd = openat(dir->fd, path.data, O_RDONLY | O_CLOEXEC | O_DIRECTORY);
        dir = fdopendir(fd);
    } else
#endif
    if (dir && dir->path.size) {
        dv_clean_path(&v, dir->path);
        dv_join_path(&v, 0, path);
        dir = opendir(v.data);
    } else {
        dv_clean_path(&v, path);
        dir = opendir(v.data);
    }

    if (dir == NULL) {
        close(fd);
        dv_free(&v);
        return -1;
    }

    d->u = dir;
    d->path = v;
    d->fd = fd;

    if (fd < 0) {
        d->path = v;
    } else {
        d->path.data = NULL;
        d->path.size = 0;
        dv_free(v);
    }

    return &d->h;
}

/* -------------------------------------------------------------------------- */

void dv_close_dir(dv_dir* d)
{
    if (d) {
        closedir((DIR*) d->dir);
        dv_free(d->path);
        close(d->fd);
        free(d);
    }
}

/* -------------------------------------------------------------------------- */

bool dv_read_dir(dv_dir* d, d_Slice(char)* file, bool* isdir)
{
    DIR* dir = d->dir;
    struct dirent* e;

next_file:
    e = readdir(d->dir);
    if (e == NULL) {
        return false;
    }

#ifdef _DIRENT_HAVE_D_NAMLEN
    *file = C2(e->d_name, e->d_namlen);
#else
    *file = dv_char(e->d_name);
#endif

    if (dv_equals(*file, C(".")) || dv_equals(*file, C(".."))) {
        goto next_file;
    }

    if (isdir == NULL) {
        /* nothing to do */

#ifdef _DIRENT_HAVE_D_TYPE
    } else if (e->d_type != DT_UNKNOWN) {
        *isdir = e->d_type == DT_DIR;
#endif

    } else {
        struct stat st;
        int fd;

#ifdef HAVE_OPENAT
        fd = openat(d->fd, e->d_name, O_RDONLY | O_CLOEXEC);
#else
        int pathsz = d->path.size;
        dv_append1(&d->path, '/');
        dv_append(&d->path, *file);
        fd = open(d->path.data, O_RDONLY | O_CLOEXEC);
        dv_resize(&d->path, pathsz);
#endif

        if (fstat(fd, &st)) {
            close(fd);
            goto next_file;
        }

        close(fd);
        *isdir = S_ISDIR(st.st_mode);
    }

    return true;
}


