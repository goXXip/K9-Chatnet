/* Database file handling routines.
 *
 * (C) 2003 Anope Team
 * Contact us at info@anope.org
 *
 * Please read COPYING and README for furhter details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church. 
 * 
 * $Id: datafiles.c 5 2004-03-29 01:29:50Z dane $ 
 *
 */

#include "services.h"
#include "datafiles.h"
#include <fcntl.h>

static int curday = 0;

/*************************************************************************/
/*************************************************************************/

/* Return the version number on the file.  Return 0 if there is no version
 * number or the number doesn't make sense (i.e. less than 1 or greater
 * than FILE_VERSION).
 */

int get_file_version(dbFILE * f)
{
    FILE *fp = f->fp;
    int version =
        fgetc(fp) << 24 | fgetc(fp) << 16 | fgetc(fp) << 8 | fgetc(fp);
    if (ferror(fp)) {
#ifndef NOT_MAIN
        log_perror("Error reading version number on %s", f->filename);
#endif
        return 0;
    } else if (feof(fp)) {
#ifndef NOT_MAIN
        alog("Error reading version number on %s: End of file detected",
             f->filename);
#endif
        return 0;
    } else if (version < 1) {
#ifndef NOT_MAIN
        alog("Invalid version number (%d) on %s", version, f->filename);
#endif
        return 0;
    }
    return version;
}

/*************************************************************************/

/* Write the current version number to the file.  Return 0 on error, 1 on
 * success.
 */

int write_file_version(dbFILE * f, uint32 version)
{
    FILE *fp = f->fp;
    if (fputc(version >> 24 & 0xFF, fp) < 0 ||
        fputc(version >> 16 & 0xFF, fp) < 0 ||
        fputc(version >> 8 & 0xFF, fp) < 0 ||
        fputc(version & 0xFF, fp) < 0) {
#ifndef NOT_MAIN
        log_perror("Error writing version number on %s", f->filename);
#endif
        return 0;
    }
    return 1;
}

/*************************************************************************/
/*************************************************************************/

static dbFILE *open_db_read(const char *service, const char *filename)
{
    dbFILE *f;
    FILE *fp;

    f = scalloc(sizeof(*f), 1);
    if (!f) {
#ifndef NOT_MAIN
        log_perror("Can't read %s database %s", service, filename);
#endif
        return NULL;
    }
    strscpy(f->filename, filename, sizeof(f->filename));
    f->mode = 'r';
    fp = fopen(f->filename, "rb");
    if (!fp) {
        int errno_save = errno;
#ifndef NOT_MAIN
        if (errno != ENOENT)
            log_perror("Can't read %s database %s", service, f->filename);
#endif
        free(f);
        errno = errno_save;
        return NULL;
    }
    f->fp = fp;
    f->backupfp = NULL;
    return f;
}

/*************************************************************************/

static dbFILE *open_db_write(const char *service, const char *filename,
                             uint32 version)
{
    dbFILE *f;
    int fd;

    f = scalloc(sizeof(*f), 1);
    if (!f) {
#ifndef NOT_MAIN
        log_perror("Can't read %s database %s", service, filename);
#endif
        return NULL;
    }
    strscpy(f->filename, filename, sizeof(f->filename));
    filename = f->filename;
    f->mode = 'w';

    *f->backupname = 0;
    snprintf(f->backupname, sizeof(f->backupname), "%s.save", filename);
    if (!*f->backupname || strcmp(f->backupname, filename) == 0) {
        int errno_save = errno;
#ifndef NOT_MAIN
        alog("Opening %s database %s for write: Filename too long",
             service, filename);
#endif
        free(f);
        errno = errno_save;
        return NULL;
    }
    unlink(f->backupname);
    f->backupfp = fopen(filename, "rb");
    if (rename(filename, f->backupname) < 0 && errno != ENOENT) {
        int errno_save = errno;
#ifndef NOT_MAIN
        static int walloped = 0;
        if (!walloped) {
            walloped++;
            wallops(NULL, "Can't back up %s database %s", service,
                    filename);
        }
        errno = errno_save;
        log_perror("Can't back up %s database %s", service, filename);
        if (!NoBackupOkay) {
#endif
            if (f->backupfp)
                fclose(f->backupfp);
            free(f);
            errno = errno_save;
            return NULL;
#ifndef NOT_MAIN
        }
#endif
        *f->backupname = 0;
    }
    unlink(filename);
    /* Use open() to avoid people sneaking a new file in under us */
    fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0666);
    f->fp = fdopen(fd, "wb");   /* will fail and return NULL if fd < 0 */
    if (!f->fp || !write_file_version(f, version)) {
        int errno_save = errno;
#ifndef NOT_MAIN
        static int walloped = 0;
        if (!walloped) {
            walloped++;
            wallops(NULL, "Can't write to %s database %s", service,
                    filename);
        }
        errno = errno_save;
        log_perror("Can't write to %s database %s", service, filename);
#endif
        if (f->fp) {
            fclose(f->fp);
            unlink(filename);
        }
        if (*f->backupname && rename(f->backupname, filename) < 0)
#ifndef NOT_MAIN
            log_perror("Cannot restore backup copy of %s", filename);
#else
            ;
#endif
        errno = errno_save;
        return NULL;
    }
    return f;
}

/*************************************************************************/

/* Open a database file for reading (*mode == 'r') or writing (*mode == 'w').
 * Return the stream pointer, or NULL on error.  When opening for write, it
 * is an error for rename() to return an error (when backing up the original
 * file) other than ENOENT, if NO_BACKUP_OKAY is not defined; it is an error
 * if the version number cannot be written to the file; and it is a fatal
 * error if opening the file for write fails and the backup was successfully
 * made but cannot be restored.
 */

dbFILE *open_db(const char *service, const char *filename,
                const char *mode, uint32 version)
{
    if (*mode == 'r') {
        return open_db_read(service, filename);
    } else if (*mode == 'w') {
        return open_db_write(service, filename, version);
    } else {
        errno = EINVAL;
        return NULL;
    }
}

/*************************************************************************/

/* Restore the database file to its condition before open_db().  This is
 * identical to close_db() for files open for reading; however, for files
 * open for writing, we first attempt to restore any backup file before
 * closing files.
 */

void restore_db(dbFILE * f)
{
    int errno_save = errno;

    if (f->mode == 'w') {
        int ok = 0;             /* Did we manage to restore the old file? */
        errno = errno_save = 0;
        if (*f->backupname && strcmp(f->backupname, f->filename) != 0) {
            if (rename(f->backupname, f->filename) == 0)
                ok = 1;
        }
        if (!ok && f->backupfp) {
            char buf[1024];
            int i;
            ok = 1;
            if (fseek(f->fp, 0, SEEK_SET) < 0)
                ok = 0;
            while (ok && (i = fread(buf, 1, sizeof(buf), f->backupfp)) > 0) {
                if (fwrite(buf, 1, i, f->fp) != i)
                    ok = 0;
            }
            if (ok) {
                fflush(f->fp);
                ftruncate(fileno(f->fp), ftell(f->fp));
            }
        }
#ifndef NOT_MAIN
        if (!ok && errno > 0)
            log_perror("Unable to restore backup of %s", f->filename);
#endif
        errno_save = errno;
        if (f->backupfp)
            fclose(f->backupfp);
        if (*f->backupname)
            unlink(f->backupname);
    }
    fclose(f->fp);
    if (!errno_save)
        errno_save = errno;
    free(f);
    errno = errno_save;
}

/*************************************************************************/

/* Close a database file.  If the file was opened for write, remove the
 * backup we (may have) created earlier.
 */

void close_db(dbFILE * f)
{
    if (f->mode == 'w' && *f->backupname
        && strcmp(f->backupname, f->filename) != 0) {
        if (f->backupfp)
            fclose(f->backupfp);
        unlink(f->backupname);
    }
    fclose(f->fp);
    free(f);
}

/*************************************************************************/
/*************************************************************************/

/* Read and write 2- and 4-byte quantities, pointers, and strings.  All
 * multibyte values are stored in big-endian order (most significant byte
 * first).  A pointer is stored as a byte, either 0 if NULL or 1 if not,
 * and read pointers are returned as either (void *)0 or (void *)1.  A
 * string is stored with a 2-byte unsigned length (including the trailing
 * \0) first; a length of 0 indicates that the string pointer is NULL.
 * Written strings are truncated silently at 65534 bytes, and are always
 * null-terminated.
 *
 * All routines return -1 on error, 0 otherwise.
 */


int read_int16(uint16 * ret, dbFILE * f)
{
    int c1, c2;

    c1 = fgetc(f->fp);
    c2 = fgetc(f->fp);
    if (c1 == EOF || c2 == EOF)
        return -1;
    *ret = c1 << 8 | c2;
    return 0;
}

int write_int16(uint16 val, dbFILE * f)
{
    if (fputc((val >> 8) & 0xFF, f->fp) == EOF
        || fputc(val & 0xFF, f->fp) == EOF)
        return -1;
    return 0;
}


int read_int32(uint32 * ret, dbFILE * f)
{
    int c1, c2, c3, c4;

    c1 = fgetc(f->fp);
    c2 = fgetc(f->fp);
    c3 = fgetc(f->fp);
    c4 = fgetc(f->fp);
    if (c1 == EOF || c2 == EOF || c3 == EOF || c4 == EOF)
        return -1;
    *ret = c1 << 24 | c2 << 16 | c3 << 8 | c4;
    return 0;
}

int write_int32(uint32 val, dbFILE * f)
{
    if (fputc((val >> 24) & 0xFF, f->fp) == EOF)
        return -1;
    if (fputc((val >> 16) & 0xFF, f->fp) == EOF)
        return -1;
    if (fputc((val >> 8) & 0xFF, f->fp) == EOF)
        return -1;
    if (fputc((val) & 0xFF, f->fp) == EOF)
        return -1;
    return 0;
}


int read_ptr(void **ret, dbFILE * f)
{
    int c;

    c = fgetc(f->fp);
    if (c == EOF)
        return -1;
    *ret = (c ? (void *) 1 : (void *) 0);
    return 0;
}

int write_ptr(const void *ptr, dbFILE * f)
{
    if (fputc(ptr ? 1 : 0, f->fp) == EOF)
        return -1;
    return 0;
}


int read_string(char **ret, dbFILE * f)
{
    char *s;
    uint16 len;

    if (read_int16(&len, f) < 0)
        return -1;
    if (len == 0) {
        *ret = NULL;
        return 0;
    }
    s = scalloc(len, 1);
    if (len != fread(s, 1, len, f->fp)) {
        free(s);
        return -1;
    }
    *ret = s;
    return 0;
}

int write_string(const char *s, dbFILE * f)
{
    uint32 len;

    if (!s)
        return write_int16(0, f);
    len = strlen(s);
    if (len > 65534)
        len = 65534;
    if (write_int16((uint16) (len + 1), f) < 0)
        return -1;
    if (len > 0 && fwrite(s, 1, len, f->fp) != len)
        return -1;
    if (fputc(0, f->fp) == EOF)
        return -1;
    return 0;
}

/*************************************************************************/
/*************************************************************************/

/* Renames a database */

static void rename_database(char *name, char *ext)
{

    char destpath[PATH_MAX];

    snprintf(destpath, sizeof(destpath), "backups/%s.%s", name, ext);
    if (rename(name, destpath) != 0) {
        alog("Backup of %s failed.", name);
        wallops(s_OperServ, "WARNING! Backup of %s failed.", name);
    }
}

/*************************************************************************/

/* Removes old databases */

static void remove_backups(void)
{

    char ext[9];
    char path[PATH_MAX];

    time_t t;
    struct tm tm;

    time(&t);
    t -= (60 * 60 * 24 * KeepBackups);
    tm = *localtime(&t);
    strftime(ext, sizeof(ext), "%Y%m%d", &tm);

    snprintf(path, sizeof(path), "backups/%s.%s", NickDBName, ext);
    unlink(path);
    snprintf(path, sizeof(path), "backups/%s.%s", BotDBName, ext);
    unlink(path);
    snprintf(path, sizeof(path), "backups/%s.%s", ChanDBName, ext);
    unlink(path);
    snprintf(path, sizeof(path), "backups/%s.%s", OperDBName, ext);
    unlink(path);
    snprintf(path, sizeof(path), "backups/%s.%s", NewsDBName, ext);
    unlink(path);
    snprintf(path, sizeof(path), "backups/%s.%s", ExceptionDBName, ext);
    unlink(path);
    snprintf(path, sizeof(path), "backups/%s.%s", HostDBName, ext);
    unlink(path);
}

/*************************************************************************/

/* Handles database backups. */

void backup_databases(void)
{

    time_t t;
    struct tm tm;

    if (!KeepBackups)
        return;

    time(&t);
    tm = *localtime(&t);

    if (!curday) {
        curday = tm.tm_yday;
        return;
    }

    if (curday != tm.tm_yday) {

        char ext[9];

        alog("Backing up databases");

        remove_backups();

        curday = tm.tm_yday;
        strftime(ext, sizeof(ext), "%Y%m%d", &tm);

        if (!skeleton) {
            rename_database(NickDBName, ext);
            if (s_BotServ)
                rename_database(BotDBName, ext);
            rename_database(ChanDBName, ext);
            if (s_HostServ)
                rename_database(HostDBName, ext);
        }

        rename_database(OperDBName, ext);
        rename_database(NewsDBName, ext);
        rename_database(ExceptionDBName, ext);
    }
}
