#ifndef WIN_MAIN_UTF8_H
#define WIN_MAIN_UTF8_H

/* For Windows systems this provides a way to get UTF-8 encoded argv strings,
 * and also overrides fopen to accept UTF-8 filenames. Working with wmain
 * directly complicates cross-platform compatibility, while normal main() in
 * Windows uses the current codepage (which has limited availability of
 * characters).
 *
 * For MinGW, you must link with -municode
 */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

static FILE *my_fopen(const char *fname, const char *mode)
{
    WCHAR *wname=NULL, *wmode=NULL;
    int namelen, modelen;
    FILE *file = NULL;
    errno_t err;

    namelen = MultiByteToWideChar(CP_UTF8, 0, fname, -1, NULL, 0);
    modelen = MultiByteToWideChar(CP_UTF8, 0, mode, -1, NULL, 0);

    if(namelen <= 0 || modelen <= 0)
    {
        fprintf(stderr, "Failed to convert UTF-8 fname \"%s\", mode \"%s\"\n", fname, mode);
        return NULL;
    }

    wname = calloc(sizeof(WCHAR), namelen+modelen);
    wmode = wname + namelen;
    MultiByteToWideChar(CP_UTF8, 0, fname, -1, wname, namelen);
    MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, modelen);

    err = _wfopen_s(&file, wname, wmode);
    if(err)
    {
        errno = err;
        file = NULL;
    }

    free(wname);

    return file;
}
#define fopen my_fopen


static char **arglist;
static void cleanup_arglist(void)
{
    free(arglist);
}

static void GetUnicodeArgs(int *argc, char ***argv)
{
    size_t total;
    wchar_t **args;
    int nargs, i;

    args = CommandLineToArgvW(GetCommandLineW(), &nargs);
    if(!args)
    {
        fprintf(stderr, "Failed to get command line args: %ld\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    total = sizeof(**argv) * nargs;
    for(i = 0;i < nargs;i++)
        total += WideCharToMultiByte(CP_UTF8, 0, args[i], -1, NULL, 0, NULL, NULL);

    atexit(cleanup_arglist);
    arglist = *argv = calloc(1, total);
    (*argv)[0] = (char*)(*argv + nargs);
    for(i = 0;i < nargs-1;i++)
    {
        int len = WideCharToMultiByte(CP_UTF8, 0, args[i], -1, (*argv)[i], 65535, NULL, NULL);
        (*argv)[i+1] = (*argv)[i] + len;
    }
    WideCharToMultiByte(CP_UTF8, 0, args[i], -1, (*argv)[i], 65535, NULL, NULL);
    *argc = nargs;

    LocalFree(args);
}
#define GET_UNICODE_ARGS(argc, argv) GetUnicodeArgs(argc, argv)

#else

/* Do nothing. */
#define GET_UNICODE_ARGS(argc, argv)

#endif

#endif /* WIN_MAIN_UTF8_H */
