#ifndef UM_UTILS_H
#define UM_UTILS_H

/* Little endian or big endian system */
int isLittleEndian();

/* Check if file exists */
int fileExists(const char *file);

/* Delete file */
int deleteFile(const char *file);

/* Copy file */
int copyFile(const char *src, const char *dest);

#endif
