/**
 * With exception of certain parts that are prominently marked as being
 * in the Public Domain, BSD, or LGPL this Software is provided under the
 * Zope Public License (ZPL) Version 2.1.
 * 
 * Copyright (c) 2009 - 2013 by the mingw-w64 project
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int asprintf(char** ret, const char* format, ...) {
  va_list ap;
  int len;
  va_start(ap,format);
  /* Get Length */
  len = _vsnprintf(NULL,0,format,ap);
  if (len < 0) goto _end;
  /* +1 for \0 terminator. */
  *ret = malloc(len + 1);
  /* Check malloc fail*/
  if (!*ret) {
    len = -1;
    goto _end;
  }
  /* Write String */
  _vsnprintf(*ret,len+1,format,ap);
  /* Terminate explicitly */
  (*ret)[len] = '\0';
  _end:
  va_end(ap);
  return len;
}
