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
                strncpy(curSection, section, sizeof(curSection)-1);
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
    WCHAR buffer[PATH_MAX];
    const WCHAR *str;
    al_string ppath;
    FILE *f;

    if(SHGetSpecialFolderPathW(NULL, buffer, CSIDL_APPDATA, FALSE) != FALSE)
    {
        al_string filepath = AL_STRING_INIT_STATIC();
        al_string_copy_wcstr(&filepath, buffer);
        al_string_append_cstr(&filepath, "\\alsoft.ini");

        TRACE("Loading config %s...\n", al_string_get_cstr(filepath));
        f = al_fopen(al_string_get_cstr(filepath), "rt");
        if(f)
        {
            LoadConfigFromFile(f);
            fclose(f);
        }
        al_string_deinit(&filepath);
    }

    ppath = GetProcPath();
    if(!al_string_empty(ppath))
    {
        al_string_append_cstr(&ppath, "\\alsoft.ini");
        TRACE("Loading config %s...\n", al_string_get_cstr(ppath));
        f = al_fopen(al_string_get_cstr(ppath), "r");
        if(f)
        {
            LoadConfigFromFile(f);
            fclose(f);
        }
    }

    if((str=_wgetenv(L"ALSOFT_CONF")) != NULL && *str)
    {
        al_string filepath = AL_STRING_INIT_STATIC();
        al_string_copy_wcstr(&filepath, str);

        TRACE("Loading config %s...\n", al_string_get_cstr(filepath));
        f = al_fopen(al_string_get_cstr(filepath), "rt");
        if(f)
        {
            LoadConfigFromFile(f);
            fclose(f);
        }
        al_string_deinit(&filepath);
    }

    al_string_deinit(&ppath);
}
#else
void ReadALConfig(void)
{
    char buffer[PATH_MAX];
    const char *str;
    al_string ppath;
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
    strncpy(buffer, str, sizeof(buffer)-1);
    buffer[sizeof(buffer)-1] = 0;
    /* Go through the list in reverse, since "the order of base directories
     * denotes their importance; the first directory listed is the most
     * important". Ergo, we need to load the settings from the later dirs
     * first so that the settings in the earlier dirs override them.
     */
    while(1)
    {
        char *next = strrchr(buffer, ':');
        if(next) *(next++) = 0;
        else next = buffer;

        if(next[0] != '/')
            WARN("Ignoring XDG config dir: %s\n", next);
        else
        {
            size_t len = strlen(next);
            strncpy(next+len, "/alsoft.conf", buffer+sizeof(buffer)-next-len);
            buffer[sizeof(buffer)-1] = 0;

            TRACE("Loading config %s...\n", next);
            f = al_fopen(next, "r");
            if(f)
            {
                LoadConfigFromFile(f);
                fclose(f);
            }
        }
        if(next == buffer)
            break;
    }

    if((str=getenv("HOME")) != NULL && *str)
    {
        snprintf(buffer, sizeof(buffer), "%s/.alsoftrc", str);

        TRACE("Loading config %s...\n", buffer);
        f = al_fopen(buffer, "r");
        if(f)
        {
            LoadConfigFromFile(f);
            fclose(f);
        }
    }

    if((str=getenv("XDG_CONFIG_HOME")) != NULL && str[0] != 0)
        snprintf(buffer, sizeof(buffer), "%s/%s", str, "alsoft.conf");
    else
    {
        buffer[0] = 0;
        if((str=getenv("HOME")) != NULL && str[0] != 0)
            snprintf(buffer, sizeof(buffer), "%s/.config/%s", str, "alsoft.conf");
    }
    if(buffer[0] != 0)
    {
        TRACE("Loading config %s...\n", buffer);
        f = al_fopen(buffer, "r");
        if(f)
        {
            LoadConfigFromFile(f);
            fclose(f);
        }
    }

    ppath = GetProcPath();
    if(!al_string_empty(ppath))
    {
        al_string_append_cstr(&ppath, "/alsoft.conf");
        TRACE("Loading config %s...\n", al_string_get_cstr(ppath));
        f = al_fopen(al_string_get_cstr(ppath), "r");
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

    al_string_deinit(&ppath);
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
