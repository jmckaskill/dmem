
struct dv_dir_i {
    dv_dir h;

    d_Vector(char) path;

#ifdef _WIN32
    d_Vector(wchar) wpath;
    WIN32_FIND_DATAW data;
#else
    DIR* dir;
#endif
};
int dv_read(d_Vector(char)* v, int fd)
{
    int size, begin = v->size;
    DWORD hsz, lsz;

    lsz = GetFileSize(fd, &hsz);

    if (hsz > 0 || lsz > INT_MAX/2) {
        size = INT_MAX/2;
    } else {
        size = (int) lsz;
    }

    dv_reserve(v, begin + size);

    for (;;) {
        DWORD r;
        BOOL ret = ReadFile(fd, v->data + v->size, dv_reserved(v) - v->size, &r, NULL);

        if (!ret) {
            DWORD err = GetLastError();
            if (err == ERROR_HANDLE_EOF || err == ERROR_BROKEN_PIPE) {
                break;
            } else {
                dv_resize(v, begin);
                return -1;
            }
        }

        dv_resize(v, v->size + r);
    }

    return v->size - begin;
}


#ifdef _WIN32

#ifdef _WIN32_WCE
#define ROOT W("\\")
#else
#define ROOT W("\\\\?\\")
#endif

int dv_append_file_win32(d_Vector(char)* out, d_Slice(wchar) folder, d_Slice(char) relative)
{
    d_Vector(wchar) wfilename = DV_INIT;
    DWORD read;
    DWORD dwsize[2];
    uint64_t size;
    int toread;
    HANDLE file;
    char* buf;
    bool is_absolute_path = dv_begins_with(relative, C("\\")) || (relative.size > 1 && relative.data[1] == ':');

    if (is_absolute_path) {
        /* absolute path */
        dv_append(&wfilename, ROOT);

    } else if (folder.size == 0) {
        /* relative path from current working directory
         * we fake the root as CE's relative directory
         */
#ifndef _WIN32_WCE
        DWORD dirsz;
        wchar_t* buf;
        dirsz = GetCurrentDirectoryW(0, NULL);
        buf = dv_append_buffer(&wfilename, dirsz);
        GetCurrentDirectoryW(dirsz + 1, buf);
        dv_resize(&wfilename, (int) wcslen(wfilename.data));
        dv_append(&wfilename, W("\\"));
#endif

    } else {
        /* relative path from supplied folder */
        dv_append(&wfilename, folder);
        dv_append(&wfilename, W("\\"));
    }

    if (!dv_begins_with(wfilename, ROOT)) {
        dv_insert(&wfilename, 0, ROOT);
    }

    dv_append_from_utf8(&wfilename, relative);
    file = CreateFileW(wfilename.data, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    dv_free(wfilename);

    if (file == INVALID_HANDLE_VALUE) {
        return -1;
    }

    dwsize[0] = GetFileSize(file, &dwsize[1]);
    size = ((uint64_t) dwsize[1] << 32) | (uint64_t) dwsize[0];
    toread = size > INT_MAX ? INT_MAX : (int) size;
    buf = dv_append_buffer(out, toread);

    if ((toread > 0 && buf == NULL) || !ReadFile(file, buf, (DWORD) toread, &read, NULL) || read != (DWORD) toread) {
        CloseHandle(file);
        dv_erase_end(out, toread);
        return -1;
    }

    CloseHandle(file);
    return toread;
}
#endif

/* -------------------------------------------------------------------------- */


