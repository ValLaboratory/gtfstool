/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * The MIT License
 *
 * Copyright (c) 2018-2019 Val Laboratory Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _WIN32
#include <mbctype.h>
#include <mbstring.h>
#else
#include <dirent.h>
#endif

#include "common.h"

static int exist_dir(const char* dir)
{
    struct stat statbuf;
    
    if (stat(dir, &statbuf) == 0)
        return 1;   // exist
    return 0;
}

void catpath(char* dir, const char* name)
{
    size_t len;
    int nameptr;
    
    if (name[0] == '\\' || name[0] == '/')
        nameptr = 1 ;
    else
        nameptr = 0 ;
    
    len = strlen(dir) ;
    
#ifdef _WIN32
    if  (len == 0 ||
         (_mbsbtype((unsigned char*)dir, len-1) == _MBC_SINGLE && (dir[len-1] == '\\' ||
                                                                   dir[len-1] == '/'  ||
                                                                   dir[len-1] == ':'))) {
    } else {
        strcat(dir, "\\") ;
    }
#else
    if (len > 0 && dir[len-1] != '/')
        strcat(dir, "/");
#endif
    
    strcat(dir, &name[nameptr]) ;
}

int makedir(const char* dir)
{
    char dtmp[MAX_PATH];
    char* tok;
    char* path;
    size_t len;

    if (*dir == '\0')
        return 0;
    if (exist_dir(dir))
        return 0;
    
    len = strlen(dir) + 1;
    path = malloc(len + 1);
    strncpy(dtmp, dir, sizeof(dtmp));
    
#ifdef _WIN32
    tok = strtok(dtmp, "\\");
    snprintf(path, len, "\\%s", tok);
#else
    tok = strtok(dtmp, "/");
    snprintf(path, len, "/%s", tok);
#endif

    while (1) {
        struct stat sb;

        if (stat(path, &sb) < 0) {
#ifdef _WIN32
            if (mkdir(path) != 0) {
                fprintf(stderr, "mkdir error: %s\n", dir);
                return -1;
            }
#else
            if (mkdir(path, 0777) != 0) {
                fprintf(stderr, "mkdir error: %s\n", dir);
                return -1;
            }
#endif
        }
        
        if (! S_ISDIR(sb.st_mode))
            return -1;

#ifdef _WIN32
        tok = strtok(NULL, "\\");
#else
        tok = strtok(NULL, "/");
#endif
        if (tok)
            catpath(path, tok);
        else
            break;  // terminate
    }
    return 0;
}

void rmfile(const char* name)
{
    remove(name);
}

void remove_files(char* dir)
{
#ifndef _WIN32
    DIR* dp;
    struct dirent* ent;
    struct stat sb;
    
    if ((dp = opendir(dir)) == NULL) {
        perror(dir);
        return;
    }
    while ((ent = readdir(dp)) != NULL) {
        char path[MAX_PATH];

        strcpy(path, dir);
        catpath(path, ent->d_name);
        stat(path, &sb);
        if (! S_ISDIR(sb.st_mode))
            rmfile(path);
    }
    closedir(dp);
#else
    err_write("windows is not support remove_files() function.");
#endif  // _WIN32
}

char* takedir(const char* fullpath, char* dir)
{
    char* dirend;
    size_t i;

    *dir = '\0';
#ifdef _WIN32
    dirend = strrchr(fullpath,'\\');
#else
    dirend = strrchr(fullpath,'/');
#endif

    if (dirend == NULL) {
        // filename only
        return NULL;
    }

    for (i = 0; i < strlen(fullpath); i++) {
        dir[i] = fullpath[i];
        if (fullpath+i == dirend) {
            dir[i] = '\0';
            break;
        }
    }
    return dir;
}
