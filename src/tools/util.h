/***************************************************************************
 *   Copyright (C) 20010~2010 by CSSlayer                                  *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/** 
 * @file util.h
 * @brief 通用函数
 * @author CSSlayer
 * @version 4.0.0
 * @date 2010-05-02
 */

#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>

#include <libintl.h>

#include "fcitx-config/uthash.h"
#define _(msgid) gettext(msgid)

typedef enum ErrorLevel
{
    DEBUG,
    ERROR,
    INFO,
    FATAL,
    WARNING
} ErrorLevel;

typedef struct StringHashSet {
    char *name;
    UT_hash_handle hh;
} StringHashSet;

char *trim(char *s);
FILE *GetXDGFile(const char *fileName, char **path, const char *mode, size_t len);
char **GetXDGPath(
        size_t *len,
        const char* homeEnv,
        const char* homeDefault,
        const char* suffixHome,
        const char* dirsDefault,
        const char* suffixGlobal);
void *malloc0(size_t bytes);

void *custom_bsearch(const void *key, const void *base,
        size_t nmemb, size_t size, int accurate,
        int (*compar)(const void *, const void *));

void FcitxInitThread();
int FcitxLock();
int FcitxUnlock();
FILE *GetXDGFileConfig(const char *fileName, const char *mode);
FILE *GetXDGFileData(const char *fileName, const char *mode);
FILE *GetXDGFile(const char *fileName, char **path, const char *mode, size_t len);
void FreeXDGPath(char **path);
#define FcitxLog(e, fmt, arg...) FcitxLogFunc(e, __FILE__, __LINE__, fmt, ##arg)
void FcitxLogFunc(ErrorLevel, const char* filename, const int line, const char* fmt, ...);
#endif