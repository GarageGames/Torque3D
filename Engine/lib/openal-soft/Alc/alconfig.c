/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2007 by authors.
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */

#ifdef _WIN32
#ifdef __MINGW32__
#define _WIN32_IE 0x501
#else
#define _WIN32_IE 0x400
#endif
#endif

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef _WIN32_IE
#include <windows.h>
#include <shlobj.h>
#endif

#include "alMain.h"
#include "alconfig.h"
#include "compat.h"
#include "bool.h"


typedef struct ConfigEntry {
    char *key;
    char *value;
} ConfigEntry;

typedef struct ConfigBlock {
    ConfigEntry *entries;
    unsigned int entryCount;
} ConfigBlock;
static ConfigBlock cfgBlock;


static char *lstrip(char *line)
{
    while(isspace(line[0]))
        line++;
    return line;
}

static char *rstrip(char *line)
{
    size_t len = strlen(line);
    while(len > 0 && isspace(line[len-1]))
        len--;
    line[len] = 0;
    return line;
}

static int readline(FILE *f, char **output, size_t *maxlen)
{
    size_t len = 0;
    int c;

    while((c=fgetc(f)) != EOF && (c == '\r' || c == '\n'))
        ;
    if(c == EOF)
        return 0;

    do {
        if(len+1 >= *maxlen)
        {
            void *temp = NULL;
            size_t newmax;

            newmax = (*maxlen ? (*maxlen)<<1 : 32);
            if(newmax > *maxlen)
                temp = realloc(*output, newmax);
            if(!temp)
            {
                ERR("Failed to realloc "SZFMT" bytes from "SZFMT"!\n", newmax, *maxlen);
                return 0;
            }

            *output = temp;
            *maxlen = newmax;
        }
        (*output)[len++] = c;
        (*output)[len] = '\0';
    } while((c=fgetc(f)) != EOF && c != '\r' && c != '\n');

    return 1;
}


static char *expdup(const char *str)
{
    char *output = NULL;
    size_t maxlen = 0;
    size_t len = 0;

    while(*str != '\0')
    {
        const char *addstr;
        size_t addstrlen;
        size_t i;

        if(str[0] != '$')
        {
            const char *next = strchr(str, '$');
            addstr = str;
            addstrlen = next ? (size_t)(next-str) : strlen(str);

            str += addstrlen;
        }
        else
        {
            str++;
            if(*str == '$')
            {
                const char *next = strchr(str+1, '$');
                addstr = str;
                addstrlen = next ? (size_t)(next-str) : strlen(str);

                str += addstrlen;
            }
            else
            {
                bool hasbraces;
                char envname[1024];
                size_t k = 0;

                hasbraces = (*str == '{');
                if(hasbraces) str++;

                while((isalnum(*str) || *str == '_') && k < sizeof(envname)-1)
                    envname[k++] = *(str++);
                envname[k++] = '\0';

                if(hasbraces && *str != '}')
                    continue;

                if(hasbraces) str++;
                if((addstr=getenv(envname)) == NULL)
                    continue;
                addstrlen = strlen(addstr);
            }
        }
        if(addstrlen == 0)
            continue;

        if(addstrlen >= maxlen-len)
        {
            void *temp = NULL;
            size_t newmax;

            newmax = len+addstrlen+1;
            if(newmax > maxlen)
                temp = realloc(output, newmax);
            if(!temp)
            {
                ERR("Failed to realloc "SZFMT" bytes from "SZFMT"!\n", newmax, maxlen);
                return output;
            }

            output = temp;
            maxlen = newmax;
        }

        for(i = 0;i < addstrlen;i++)
            output[len++] = addstr[i];
        output[len] = '\0';
    }

    return output ? output : calloc(1, 1);
}


static void LoadConfigFromFile(FILE *f)
{
    char curSection[128] = "";
    char *buffer = NULL;
    size_t maxlen = 0;
    ConfigEntry *ent;

    while(readline(f, &buffer, &maxlen))
    {
        char *line, *comment;
        char key[256] = "";
        char value[256] = "";

        line = rstrip(lstrip(buffer));
        if(!line[0]) continue;

        if(line[0] == '[')
        {
            char *section = line+1;
            char *endsection;

            endsection = strchr(section, ']');
            if(!endsection || section == endsection)
            {
                ERR("config parse error: bad line \"%s\"\n", line);
                continue;
            }
            if(endsection[1] != 0)
            {
                char *end = endsection+1;
                while(isspace(*end))
                    ++end;
                if(*end != 0 && *end != '#')
                {
                    ERR("config parse error: bad line \"%s\"\n", line);
                    continue;
                }
            }
            *endsection = 0;

            if(strcasecmp(section, "general") == 0)
                curSection[0] = 0;
            else
            {
                size_t len, p = 0;
                do {
                    char *nextp = strchr(section, '%');
                    if(!nextp)
                    {
                        strncpy(curSection+p, section, sizeof(curSection)-1-p);
                        break;
                    }

                    len = nextp - section;
                    if(len > sizeof(curSection)-1-p)
                        len = sizeof(curSection)-1-p;
                    strncpy(curSection+p, section, len);
                    p += len;
                    section = nextp;

                    if(((section[1] >= '0' && section[1] <= '9') ||
                        (section[1] >= 'a' && section[1] <= 'f') ||
                        (section[1] >= 'A' && section[1] <= 'F')) &&
                       ((section[2] >= '0' && section[2] <= '9') ||
                        (section[2] >= 'a' && section[2] <= 'f') ||
                        (section[2] >= 'A' && section[2] <= 'F')))
                    {
                        unsigned char b = 0;
                        if(section[1] >= '0' && section[1] <= '9')
                            b = (section[1]-'0') << 4;
                        else if(section[1] >= 'a' && section[1] <= 'f')
                            b = (section[1]-'a'+0xa) << 4;
                        else if(section[1] >= 'A' && section[1] <= 'F')
                            b = (section[1]-'A'+0x0a) << 4;
                        if(section[2] >= '0' && section[2] <= '9')
                            b |= (section[2]-'0');
                        else if(section[2] >= 'a' && section[2] <= 'f')
                            b |= (section[2]-'a'+0xa);
                        else if(section[2] >= 'A' && section[2] <= 'F')
                            b |= (section[2]-'A'+0x0a);
                        if(p < sizeof(curSection)-1)
                            curSection[p++] = b;
                        section += 3;
                    }
                    else if(section[1] == '%')
                    {
                        if(p < sizeof(curSection)-1)
                            curSection[p++] = '%';
                        section += 2;
                    }
                    else
                    {
                        if(p < sizeof(curSection)-1)
                            curSection[p++] = '%';
                        section += 1;
                    }
                    if(p < sizeof(curSection)-1)
                        curSection[p] = 0;
                } while(p < sizeof(curSection)-1 && *section != 0);
                curSection[sizeof(curSection)-1] = 0;
            }

            continue;
        }

        comment = strchr(line, '#');
        if(comment) *(comment++) = 0;
        if(!line[0]) continue;

        if(sscanf(line, "%255[^=] = \"%255[^\"]\"", key, value) == 2 ||
           sscanf(line, "%255[^=] = '%255[^\']'", key, value) == 2 ||
           sscanf(line, "%255[^=] = %255[^\n]", key, value) == 2)
        {
            /* sscanf doesn't handle '' or "" as empty values, so clip it
             * manually. */
            if(strcmp(value, "\"\"") == 0 || strcmp(value, "''") == 0)
                value[0] = 0;
        }
        else if(sscanf(line, "%255[^=] %255[=]", key, value) == 2)
        {
            /* Special case for 'key =' */
            value[0] = 0;
        }
        else
        {
            ERR("config parse error: malformed option line: \"%s\"\n\n", line);
            continue;
        }
        rstrip(key);

        if(curSection[0] != 0)
        {
            size_t len = strlen(curSection);
            memmove(&key[len+1], key, sizeof(key)-1-len);
            key[len] = '/';
            memcpy(key, curSection, len);
        }

        /* Check if we already have this option set */
        ent = cfgBlock.entries;
        while((unsigned int)(ent-cfgBlock.entries) < cfgBlock.entryCount)
        {
            if(strcasecmp(ent->key, key) == 0)
                break;
            ent++;
        }

        if((unsigned int)(ent-cfgBlock.entries) >= cfgBlock.entryCount)
        {
            /* Allocate a new option entry */
            ent = realloc(cfgBlock.entries, (cfgBlock.entryCount+1)*sizeof(ConfigEntry));
            if(!ent)
            {
                 ERR("config parse error: error reallocating config entries\n");
                 continue;
            }
            cfgBlock.entries = ent;
            ent = cfgBlock.entries + cfgBlock.entryCount;
            cfgBlock.entryCount++;

            ent->key = strdup(key);
            ent->value = NULL;
        }

        free(ent->value);
        ent->value = expdup(value);

        TRACE("found '%s' = '%s'\n", ent->key, ent->value);
    }

    free(buffer);
}

#ifdef _WIN32
void ReadALConfig(void)
{
    al_string ppath = AL_STRING_INIT_STATIC();
    WCHAR buffer[MAX_PATH];
    const WCHAR *str;
    FILE *f;

    if(SHGetSpecialFolderPathW(NULL, buffer, CSIDL_APPDATA, FALSE) != FALSE)
    {
        al_string filepath = AL_STRING_INIT_STATIC();
        alstr_copy_wcstr(&filepath, buffer);
        alstr_append_cstr(&filepath, "\\alsoft.ini");

        TRACE("Loading config %s...\n", alstr_get_cstr(filepath));
        f = al_fopen(alstr_get_cstr(filepath), "rt");
        if(f)
        {
            LoadConfigFromFile(f);
            fclose(f);
        }
        alstr_reset(&filepath);
    }

    GetProcBinary(&ppath, NULL);
    if(!alstr_empty(ppath))
    {
        alstr_append_cstr(&ppath, "\\alsoft.ini");
        TRACE("Loading config %s...\n", alstr_get_cstr(ppath));
        f = al_fopen(alstr_get_cstr(ppath), "r");
        if(f)
        {
            LoadConfigFromFile(f);
            fclose(f);
        }
    }

    if((str=_wgetenv(L"ALSOFT_CONF")) != NULL && *str)
    {
        al_string filepath = AL_STRING_INIT_STATIC();
        alstr_copy_wcstr(&filepath, str);

        TRACE("Loading config %s...\n", alstr_get_cstr(filepath));
        f = al_fopen(alstr_get_cstr(filepath), "rt");
        if(f)
        {
            LoadConfigFromFile(f);
            fclose(f);
        }
        alstr_reset(&filepath);
    }

    alstr_reset(&ppath);
}
#else
void ReadALConfig(void)
{
    al_string confpaths = AL_STRING_INIT_STATIC();
    al_string fname = AL_STRING_INIT_STATIC();
    const char *str;
    FILE *f;

    str = "/etc/openal/alsoft.conf";

    TRACE("Loading config %s...\n", str);
    f = al_fopen(str, "r");
    if(f)
    {
        LoadConfigFromFile(f);
        fclose(f);
    }

    if(!(str=getenv("XDG_CONFIG_DIRS")) || str[0] == 0)
        str = "/etc/xdg";
    alstr_copy_cstr(&confpaths, str);
    /* Go through the list in reverse, since "the order of base directories
     * denotes their importance; the first directory listed is the most
     * important". Ergo, we need to load the settings from the later dirs
     * first so that the settings in the earlier dirs override them.
     */
    while(!alstr_empty(confpaths))
    {
        char *next = strrchr(alstr_get_cstr(confpaths), ':');
        if(next)
        {
            size_t len = next - alstr_get_cstr(confpaths);
            alstr_copy_cstr(&fname, next+1);
            VECTOR_RESIZE(confpaths, len, len+1);
            VECTOR_ELEM(confpaths, len) = 0;
        }
        else
        {
            alstr_reset(&fname);
            fname = confpaths;
            AL_STRING_INIT(confpaths);
        }

        if(alstr_empty(fname) || VECTOR_FRONT(fname) != '/')
            WARN("Ignoring XDG config dir: %s\n", alstr_get_cstr(fname));
        else
        {
            if(VECTOR_BACK(fname) != '/') alstr_append_cstr(&fname, "/alsoft.conf");
            else alstr_append_cstr(&fname, "alsoft.conf");

            TRACE("Loading config %s...\n", alstr_get_cstr(fname));
            f = al_fopen(alstr_get_cstr(fname), "r");
            if(f)
            {
                LoadConfigFromFile(f);
                fclose(f);
            }
        }
        alstr_clear(&fname);
    }

    if((str=getenv("HOME")) != NULL && *str)
    {
        alstr_copy_cstr(&fname, str);
        if(VECTOR_BACK(fname) != '/') alstr_append_cstr(&fname, "/.alsoftrc");
        else alstr_append_cstr(&fname, ".alsoftrc");

        TRACE("Loading config %s...\n", alstr_get_cstr(fname));
        f = al_fopen(alstr_get_cstr(fname), "r");
        if(f)
        {
            LoadConfigFromFile(f);
            fclose(f);
        }
    }

    if((str=getenv("XDG_CONFIG_HOME")) != NULL && str[0] != 0)
    {
        alstr_copy_cstr(&fname, str);
        if(VECTOR_BACK(fname) != '/') alstr_append_cstr(&fname, "/alsoft.conf");
        else alstr_append_cstr(&fname, "alsoft.conf");
    }
    else
    {
        alstr_clear(&fname);
        if((str=getenv("HOME")) != NULL && str[0] != 0)
        {
            alstr_copy_cstr(&fname, str);
            if(VECTOR_BACK(fname) != '/') alstr_append_cstr(&fname, "/.config/alsoft.conf");
            else alstr_append_cstr(&fname, ".config/alsoft.conf");
        }
    }
    if(!alstr_empty(fname))
    {
        TRACE("Loading config %s...\n", alstr_get_cstr(fname));
        f = al_fopen(alstr_get_cstr(fname), "r");
        if(f)
        {
            LoadConfigFromFile(f);
            fclose(f);
        }
    }

    alstr_clear(&fname);
    GetProcBinary(&fname, NULL);
    if(!alstr_empty(fname))
    {
        if(VECTOR_BACK(fname) != '/') alstr_append_cstr(&fname, "/alsoft.conf");
        else alstr_append_cstr(&fname, "alsoft.conf");

        TRACE("Loading config %s...\n", alstr_get_cstr(fname));
        f = al_fopen(alstr_get_cstr(fname), "r");
        if(f)
        {
            LoadConfigFromFile(f);
            fclose(f);
        }
    }

    if((str=getenv("ALSOFT_CONF")) != NULL && *str)
    {
        TRACE("Loading config %s...\n", str);
        f = al_fopen(str, "r");
        if(f)
        {
            LoadConfigFromFile(f);
            fclose(f);
        }
    }

    alstr_reset(&fname);
    alstr_reset(&confpaths);
}
#endif

void FreeALConfig(void)
{
    unsigned int i;

    for(i = 0;i < cfgBlock.entryCount;i++)
    {
        free(cfgBlock.entries[i].key);
        free(cfgBlock.entries[i].value);
    }
    free(cfgBlock.entries);
}

const char *GetConfigValue(const char *devName, const char *blockName, const char *keyName, const char *def)
{
    unsigned int i;
    char key[256];

    if(!keyName)
        return def;

    if(blockName && strcasecmp(blockName, "general") != 0)
    {
        if(devName)
            snprintf(key, sizeof(key), "%s/%s/%s", blockName, devName, keyName);
        else
            snprintf(key, sizeof(key), "%s/%s", blockName, keyName);
    }
    else
    {
        if(devName)
            snprintf(key, sizeof(key), "%s/%s", devName, keyName);
        else
        {
            strncpy(key, keyName, sizeof(key)-1);
            key[sizeof(key)-1] = 0;
        }
    }

    for(i = 0;i < cfgBlock.entryCount;i++)
    {
        if(strcmp(cfgBlock.entries[i].key, key) == 0)
        {
            TRACE("Found %s = \"%s\"\n", key, cfgBlock.entries[i].value);
            if(cfgBlock.entries[i].value[0])
                return cfgBlock.entries[i].value;
            return def;
        }
    }

    if(!devName)
    {
        TRACE("Key %s not found\n", key);
        return def;
    }
    return GetConfigValue(NULL, blockName, keyName, def);
}

int ConfigValueExists(const char *devName, const char *blockName, const char *keyName)
{
    const char *val = GetConfigValue(devName, blockName, keyName, "");
    return !!val[0];
}

int ConfigValueStr(const char *devName, const char *blockName, const char *keyName, const char **ret)
{
    const char *val = GetConfigValue(devName, blockName, keyName, "");
    if(!val[0]) return 0;

    *ret = val;
    return 1;
}

int ConfigValueInt(const char *devName, const char *blockName, const char *keyName, int *ret)
{
    const char *val = GetConfigValue(devName, blockName, keyName, "");
    if(!val[0]) return 0;

    *ret = strtol(val, NULL, 0);
    return 1;
}

int ConfigValueUInt(const char *devName, const char *blockName, const char *keyName, unsigned int *ret)
{
    const char *val = GetConfigValue(devName, blockName, keyName, "");
    if(!val[0]) return 0;

    *ret = strtoul(val, NULL, 0);
    return 1;
}

int ConfigValueFloat(const char *devName, const char *blockName, const char *keyName, float *ret)
{
    const char *val = GetConfigValue(devName, blockName, keyName, "");
    if(!val[0]) return 0;

#ifdef HAVE_STRTOF
    *ret = strtof(val, NULL);
#else
    *ret = (float)strtod(val, NULL);
#endif
    return 1;
}

int ConfigValueBool(const char *devName, const char *blockName, const char *keyName, int *ret)
{
    const char *val = GetConfigValue(devName, blockName, keyName, "");
    if(!val[0]) return 0;

    *ret = (strcasecmp(val, "true") == 0 || strcasecmp(val, "yes") == 0 ||
            strcasecmp(val, "on") == 0 || atoi(val) != 0);
    return 1;
}

int GetConfigValueBool(const char *devName, const char *blockName, const char *keyName, int def)
{
    const char *val = GetConfigValue(devName, blockName, keyName, "");

    if(!val[0]) return !!def;
    return (strcasecmp(val, "true") == 0 || strcasecmp(val, "yes") == 0 ||
            strcasecmp(val, "on") == 0 || atoi(val) != 0);
}
