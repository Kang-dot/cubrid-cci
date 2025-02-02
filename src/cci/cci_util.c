/*
 * Copyright (C) 2008 Search Solution Corporation. 
 * Copyright (c) 2016 CUBRID Corporation.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors
 *   may be used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */


/*
 * cci_util.c -
 */

#ident "$Id$"

/************************************************************************
 * IMPORTED SYSTEM HEADER FILES						*
 ************************************************************************/
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#ifdef WINDOWS
#include <winsock2.h>
#include <windows.h>
#endif
#include <sys/types.h>
#include <regex>
#include  <math.h>

/************************************************************************
 * OTHER IMPORTED HEADER FILES						*
 ************************************************************************/

#include "cci_common.h"
#include "cas_cci.h"
#include "cci_util.h"
#include "cci_handle_mng.h"
#include "cci_t_lob.h"

/************************************************************************
 * PRIVATE DEFINITIONS							*
 ************************************************************************/
#define MAX_DOUBLE_STRING            (512)
#define MAX_FLOAT_STRING             (128)

/************************************************************************
 * PRIVATE TYPE DEFINITIONS						*
 ************************************************************************/

/************************************************************************
 * PRIVATE FUNCTION PROTOTYPES						*
 ************************************************************************/

static char is_float_str (char *str);
static int skip_ampm_chars (char *str);
static int get_pm_offset (char *str, int hh);

/************************************************************************
 * INTERFACE VARIABLES							*
 ************************************************************************/

/************************************************************************
 * PUBLIC VARIABLES							*
 ************************************************************************/

/************************************************************************
 * PRIVATE VARIABLES							*
 ************************************************************************/

/************************************************************************
 * IMPLEMENTATION OF INTERFACE FUNCTIONS 				*
 ************************************************************************/

/************************************************************************
 * IMPLEMENTATION OF PUBLIC FUNCTIONS	 				*
 ************************************************************************/

int
ut_str_to_bigint (char *str, INT64 * value)
{
  int error = 0;
  INT64 bi_val = 0;
  char *end_p = NULL;

  assert (value != NULL);

  *value = 0;

  error = str_to_int64 (&bi_val, &end_p, str, 10);
  if (error < 0)
    {
      return (CCI_ER_TYPE_CONVERSION);
    }

  if (*end_p == 0 || *end_p == '.' || isspace ((int) *end_p))
    {
      *value = bi_val;
      return 0;
    }

  return (CCI_ER_TYPE_CONVERSION);
}

int
ut_str_to_ubigint (char *str, UINT64 * value)
{
  int error = 0;
  UINT64 ubi_val = 0;
  char *end_p = NULL;

  assert (value != NULL);

  *value = 0;

  error = str_to_uint64 (&ubi_val, &end_p, str, 10);
  if (error < 0)
    {
      return (CCI_ER_TYPE_CONVERSION);
    }

  if (*end_p == 0 || *end_p == '.' || isspace ((int) *end_p))
    {
      *value = ubi_val;
      return 0;
    }

  return (CCI_ER_TYPE_CONVERSION);
}

int
ut_str_to_int (char *str, int *value)
{
  int error = 0;
  int i_val = 0;
  char *end_p = NULL;

  assert (value != NULL);

  *value = 0;

  error = str_to_int32 (&i_val, &end_p, str, 10);
  if (error < 0)
    {
      return (CCI_ER_TYPE_CONVERSION);
    }

  if (*end_p == 0 || *end_p == '.' || isspace ((int) *end_p))
    {
      *value = i_val;
      return 0;
    }

  return (CCI_ER_TYPE_CONVERSION);
}

int
ut_str_to_uint (char *str, unsigned int *value)
{
  int error = 0;
  unsigned int ui_val = 0;
  char *end_p = NULL;

  assert (value != NULL);

  *value = 0;

  error = str_to_uint32 (&ui_val, &end_p, str, 10);
  if (error < 0)
    {
      return (CCI_ER_TYPE_CONVERSION);
    }

  if (*end_p == 0 || *end_p == '.' || isspace ((int) *end_p))
    {
      *value = ui_val;
      return 0;
    }

  return (CCI_ER_TYPE_CONVERSION);
}

int
ut_str_to_float (char *str, float *value)
{
  int error = 0;
  float f_val = 0;
  char *end_p = NULL;

  assert (value != NULL);

  *value = 0;

  if (!is_float_str (str))
    {
      return (CCI_ER_TYPE_CONVERSION);
    }

  error = str_to_float (&f_val, &end_p, str);
  if (error < 0)
    {
      return (CCI_ER_TYPE_CONVERSION);
    }

  if (*end_p == 0 || isspace ((int) *end_p))
    {
      *value = f_val;
      return 0;
    }

  return (CCI_ER_TYPE_CONVERSION);
}

int
ut_str_to_double (char *str, double *value)
{
  int error = 0;
  double d_val = 0;
  char *end_p = NULL;

  assert (value != NULL);

  *value = 0;

  if (!is_float_str (str))
    {
      return (CCI_ER_TYPE_CONVERSION);
    }

  error = str_to_double (&d_val, &end_p, str);
  if (error < 0)
    {
      return (CCI_ER_TYPE_CONVERSION);
    }

  if (*end_p == 0 || isspace ((int) *end_p))
    {
      *value = d_val;
      return 0;
    }

  return (CCI_ER_TYPE_CONVERSION);
}

int
ut_str_to_date (char *str, T_CCI_DATE * value)
{
  int error = 0;
  int yr = 0, mon = 0, day = 0;
  char *p = NULL;
  char *end_p = NULL;
  char delimiter = '\0';

  assert (value != NULL);

  if (str == NULL)
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  p = str;

  error = str_to_int32 (&yr, &end_p, p, 10);
  if (error < 0)
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  delimiter = *end_p;
  if (delimiter != '-' && delimiter != '/')
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  p = end_p + 1;

  error = str_to_int32 (&mon, &end_p, p, 10);
  if (error < 0)
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  if (*end_p != delimiter)
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  p = end_p + 1;

  error = str_to_int32 (&day, &end_p, p, 10);
  if (error < 0)
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  if (*end_p != 0 && !isspace ((int) *end_p))
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  memset (value, 0, sizeof (T_CCI_DATE));
  value->yr = yr;
  value->mon = mon;
  value->day = day;
  return 0;
}

int
ut_str_to_time (char *str, T_CCI_DATE * value)
{
  int error = 0, offset = 0;
  int hh = 0, mm = 0, ss = 0;
  char *p = NULL;
  char *end_p = NULL;

  assert (value != NULL);

  if (str == NULL)
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  p = str;

  error = str_to_int32 (&hh, &end_p, p, 10);
  if (error < 0)
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  if (*end_p != ':')
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  p = end_p + 1;

  error = str_to_int32 (&mm, &end_p, p, 10);
  if (error < 0)
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  if (*end_p != ':')
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  p = end_p + 1;

  error = str_to_int32 (&ss, &end_p, p, 10);
  if (error < 0)
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  if (*end_p != '\0' && !isspace ((int) *end_p))
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  if (*end_p != '\0')
    {
      p = end_p + 1;
      offset = get_pm_offset (p, hh);
      hh += offset;
    }

  memset (value, 0, sizeof (T_CCI_DATE));
  value->hh = hh;
  value->mm = mm;
  value->ss = ss;
  return 0;
}

int
ut_str_to_timetz (char *str, T_CCI_DATE_TZ * value)
{
  char *p, *q;
  int hh, mm, ss;

  if (str == NULL)
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  p = str;
  q = strchr (p, ':');
  if (q == NULL)
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  hh = atoi (p);
  p = q + 1;

  q = strchr (p, ':');
  if (q == NULL)
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  mm = atoi (p);
  p = q + 1;
  ss = atoi (p);
  q = strchr (p, ' ');

  if (q != NULL)
    {
      strncpy (value->tz, q + 1, sizeof (value->tz) - 1);
    }
  else
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  memset (value, 0, sizeof (T_CCI_DATE));
  value->hh = hh;
  value->mm = mm;
  value->ss = ss;

  return 0;
}

int
ut_str_to_mtime (char *str, T_CCI_DATE * value)
{
  int error = 0, offset = 0;
  int hh = 0, mm = 0, ss = 0, ms = 0;
  char *p = NULL;
  char *end_p = NULL;
  double ms_tmp = 0;

  assert (value != NULL);

  if (str == NULL)
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  p = str;

  error = str_to_int32 (&hh, &end_p, p, 10);
  if (error < 0)
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  if (*end_p != ':')
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  p = end_p + 1;

  error = str_to_int32 (&mm, &end_p, p, 10);
  if (error < 0)
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  if (*end_p != ':')
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  p = end_p + 1;

  error = str_to_int32 (&ss, &end_p, p, 10);
  if (error < 0)
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  if (*end_p == '.')
    {
      p = end_p;
      ms_tmp = 0;

      error = str_to_double (&ms_tmp, &end_p, p);
      if (error < 0)
	{
	  return CCI_ER_TYPE_CONVERSION;
	}
      ms = (int) (ms_tmp * 1000 + 0.5);
    }
  else
    {
      ms = 0;
    }

  if (*end_p != 0 && !isspace ((int) *end_p))
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  if (*end_p != '\0')
    {
      p = end_p + 1;
      offset = get_pm_offset (p, hh);
      hh += offset;
    }

  memset (value, 0, sizeof (T_CCI_DATE));
  value->hh = hh;
  value->mm = mm;
  value->ss = ss;
  value->ms = ms;

  return 0;
}

int
ut_str_to_timestamp (char *str, T_CCI_DATE * value)
{
  T_CCI_DATE date;
  T_CCI_DATE time;
  char *p = NULL;
  int err_code = 0;

  p = strchr (str, ' ');

  if ((err_code = ut_str_to_date (str, &date)) < 0)
    {
      return err_code;
    }
  if ((err_code = ut_str_to_time (p, &time)) < 0)
    {
      return err_code;
    }

  memset (value, 0, sizeof (T_CCI_DATE));
  value->yr = date.yr;
  value->mon = date.mon;
  value->day = date.day;
  value->hh = time.hh;
  value->mm = time.mm;
  value->ss = time.ss;

  return 0;
}

int
ut_str_to_timestamptz (char *str, T_CCI_DATE_TZ * value)
{
  T_CCI_DATE date;
  T_CCI_DATE time;
  char *p;
  int err_code, ampm_skipped_chars = 0;

  p = strchr (str, ' ');

  if ((err_code = ut_str_to_date (str, &date)) < 0)
    {
      return err_code;
    }
  if ((err_code = ut_str_to_time (p, &time)) < 0)
    {
      return err_code;
    }

  p = p + 1;
  p = strchr (p, ' ');

  if (p != NULL)
    {
      ampm_skipped_chars = skip_ampm_chars (p);
      p += ampm_skipped_chars;
    }

  if (p != NULL)
    {
      strncpy (value->tz, p, sizeof (value->tz) - 1 - ampm_skipped_chars);
    }
  else
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  value->yr = date.yr;
  value->mon = date.mon;
  value->day = date.day;
  value->hh = time.hh;
  value->mm = time.mm;
  value->ss = time.ss;

  return 0;
}

int
ut_str_to_datetime (char *str, T_CCI_DATE * value)
{
  T_CCI_DATE date;
  T_CCI_DATE mtime;
  char *p = NULL;
  int err_code = 0;

  p = strchr (str, ' ');

  if ((err_code = ut_str_to_date (str, &date)) < 0)
    {
      return err_code;
    }
  if ((err_code = ut_str_to_mtime (p, &mtime)) < 0)
    {
      return err_code;
    }

  memset (value, 0, sizeof (T_CCI_DATE));
  value->yr = date.yr;
  value->mon = date.mon;
  value->day = date.day;
  value->hh = mtime.hh;
  value->mm = mtime.mm;
  value->ss = mtime.ss;
  value->ms = mtime.ms;

  return 0;
}

int
ut_str_to_datetimetz (char *str, T_CCI_DATE_TZ * value)
{
  T_CCI_DATE date;
  T_CCI_DATE mtime;
  char *p;
  int err_code, ampm_skipped_chars = 0;

  p = strchr (str, ' ');

  if ((err_code = ut_str_to_date (str, &date)) < 0)
    {
      return err_code;
    }
  if ((err_code = ut_str_to_mtime (p, &mtime)) < 0)
    {
      return err_code;
    }

  p = p + 1;
  p = strchr (p, ' ');

  if (p != NULL)
    {
      ampm_skipped_chars = skip_ampm_chars (p);
      p += ampm_skipped_chars;
    }

  if (p != NULL)
    {
      strncpy (value->tz, p, sizeof (value->tz) - 1 - ampm_skipped_chars);
    }
  else
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  value->yr = date.yr;
  value->mon = date.mon;
  value->day = date.day;
  value->hh = mtime.hh;
  value->mm = mtime.mm;
  value->ss = mtime.ss;
  value->ms = mtime.ms;

  return 0;
}

int
ut_str_to_oid (char *str, T_OBJECT * value)
{
  int error = 0;
  int id = 0;
  char *p = str;
  char *end_p = NULL;

  memset (value, 0, sizeof (T_OBJECT));

  if (p == NULL)
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  if (*p != '@')
    {
      return CCI_ER_TYPE_CONVERSION;
    }

  p++;
  error = str_to_int32 (&id, &end_p, p, 10);	/* page id */
  if (error < 0)
    {
      return CCI_ER_TYPE_CONVERSION;
    }
  if (*end_p != '|')
    {
      return CCI_ER_TYPE_CONVERSION;
    }
  value->pageid = id;

  p = end_p + 1;
  error = str_to_int32 (&id, &end_p, p, 10);	/* slot id */
  if (error < 0)
    {
      return CCI_ER_TYPE_CONVERSION;
    }
  if (*end_p != '|')
    {
      return CCI_ER_TYPE_CONVERSION;
    }
  value->slotid = id;

  p = end_p + 1;
  error = str_to_int32 (&id, &end_p, p, 10);	/* vol id */
  if (error < 0)
    {
      return CCI_ER_TYPE_CONVERSION;
    }
  if (*end_p != 0)
    {
      return CCI_ER_TYPE_CONVERSION;
    }
  value->volid = id;

  /* Set virtual oid, "0|0|0", as an error, requested by ctshim */
  if (value->pageid == 0 && value->slotid == 0 && value->volid == 0)
    {
      return CCI_ER_OBJECT;
    }

  return 0;
}

void
ut_int_to_str (INT64 value, char *str, int size)
{
  snprintf (str, size, "%lld", (long long) value);
}

void
ut_uint_to_str (UINT64 value, char *str, int size)
{
  snprintf (str, size, "%llu", (unsigned long long) value);
}

void
ut_float_to_str (float value, char *str, int size)
{
  snprintf (str, size, "%f", value);
}

void
ut_double_to_str (double value, char *str, int size)
{
  snprintf (str, size, "%.16f", value);
}

void
ut_double_to_str_with_remove_trailingzeros (double value, char *str, int size)
{
  char double_str[MAX_DOUBLE_STRING];
  char return_str[MAX_DOUBLE_STRING] = { '-', '0', '.', '\0', };
  int exp_num, offset, p;

  char *dot, *exp, *sp = return_str + 1;

  snprintf (double_str, sizeof (double_str), "%.16g", fabs (value));
  dot = strchr (double_str, '.');
  exp = strchr (double_str, 'e');

  if (!exp)
    {
      snprintf (str, size, "%s%s", (value < 0) ? "-" : "", double_str);
      return;
    }

  exp_num = atoi (exp + 1);

  if (exp_num > 0)
    {
      if (dot)
	{
	  memcpy (sp, double_str, offset = (int) (dot - double_str));
	  memcpy (sp + offset, dot + 1, p = (int) (exp - dot) - 1);
	  memset (sp + offset + p, '0', exp_num - (int) (exp - dot) + 1);
	}
      else
	{
	  memcpy (sp, double_str, p = (int) (exp - double_str));
	  memset (sp + p, '0', exp_num);
	}
    }
  else
    {
      exp_num = -exp_num;
      memset (sp + 2, '0', offset = exp_num - 1);

      if (dot)
	{
	  memcpy (sp + 2 + offset, double_str, p = (int) (dot - double_str));
	  memcpy (sp + 2 + offset + p, dot + 1, (int) (exp - dot) - 1);
	}
      else
	{
	  memcpy (sp + 2 + offset, double_str, (int) (exp - double_str));
	}
    }

  (value < 0) ? strncpy (str, return_str, size) : strncpy (str, return_str + 1, size);
}

void
ut_float_to_str_with_remove_trailingzeros (float value, char *str, int size)
{
  char float_str[MAX_FLOAT_STRING];
  char return_str[MAX_FLOAT_STRING] = { '-', '0', '.', '\0', };
  int exp_num, offset, p;

  char *dot, *exp, *sp = return_str + 1;

  snprintf (float_str, sizeof (float_str), "%.7g", fabsf (value));
  dot = strchr (float_str, '.');
  exp = strchr (float_str, 'e');

  if (!exp)
    {
      snprintf (str, size, "%s%s", (value < 0) ? "-" : "", float_str);
      return;
    }

  exp_num = atoi (exp + 1);

  if (exp_num > 0)
    {
      if (dot)
	{
	  memcpy (sp, float_str, offset = (int) (dot - float_str));
	  memcpy (sp + offset, dot + 1, p = (int) (exp - dot) - 1);
	  memset (sp + offset + p, '0', exp_num - (int) (exp - dot) + 1);
	}
      else
	{
	  memcpy (sp, float_str, p = (int) (exp - float_str));
	  memset (sp + p, '0', exp_num);
	}
    }
  else
    {
      exp_num = -exp_num;
      memset (sp + 2, '0', offset = exp_num - 1);

      if (dot)
	{
	  memcpy (sp + 2 + offset, float_str, p = (int) (dot - float_str));
	  memcpy (sp + 2 + offset + p, dot + 1, (int) (exp - dot) - 1);
	}
      else
	{
	  memcpy (sp + 2 + offset, float_str, (int) (exp - float_str));
	}
    }

  (value < 0) ? strncpy (str, return_str, size) : strncpy (str, return_str + 1, size);
}

void
ut_date_to_str (T_CCI_DATE * value, T_CCI_U_TYPE u_type, char *str, int size)
{
  if (u_type == CCI_U_TYPE_DATE)
    {
      snprintf (str, size, "%04d-%02d-%02d", value->yr, value->mon, value->day);
    }
  else if (u_type == CCI_U_TYPE_TIME)
    {
      snprintf (str, size, "%02d:%02d:%02d", value->hh, value->mm, value->ss);
    }
  else if (u_type == CCI_U_TYPE_TIMESTAMP)
    {
      snprintf (str, size, "%04d-%02d-%02d %02d:%02d:%02d", value->yr, value->mon, value->day, value->hh, value->mm,
		value->ss);
    }
  else
    {				/* u_type == CCI_U_TYPE_DATETIME */
      snprintf (str, size, "%04d-%02d-%02d %02d:%02d:%02d.%03d", value->yr, value->mon, value->day, value->hh,
		value->mm, value->ss, value->ms);
    }
}

void
ut_date_tz_to_str (T_CCI_DATE_TZ * value, T_CCI_U_TYPE u_type, char *str, int size)
{
  int len;

  if (u_type == CCI_U_TYPE_DATETIMETZ || u_type == CCI_U_TYPE_DATETIMELTZ || u_type == CCI_U_TYPE_TIMESTAMPTZ
      || u_type == CCI_U_TYPE_TIMESTAMPLTZ)
    {
      int remain_size;
      ut_date_to_str ((T_CCI_DATE *) value, u_type, str, size);

      len = (int) strlen (str);
      remain_size = size - len;
      if (remain_size > 1)
	{
	  str += len;
	  *str++ = ' ';
	  *str = '\0';
	  strncat (str, value->tz, remain_size - 1);
	}
    }
}

void
ut_oid_to_str (T_OBJECT * oid, char *str)
{
  sprintf (str, "@%d|%d|%d", oid->pageid, oid->slotid, oid->volid);
}

void
ut_lob_to_str (T_LOB * lob, char *str, int size)
{
#if 0
  sprintf (str, "%s:%s", (lob->type == CCI_U_TYPE_BLOB ? "BLOB" : (lob->type == CCI_U_TYPE_CLOB ? "CLOB" : "????")),
	   lob->handle + 16);
#else
  strncpy (str, lob->handle + 16, size);
#endif
}

void
ut_bit_to_str (char *bit_str, int bit_size, char *str, int str_size)
{
  char ch, c;
  int i;

  for (i = 0; i < bit_size; i++)
    {
      if (2 * i + 1 >= str_size)
	{
	  break;
	}

      ch = bit_str[i];

      c = (ch >> 4) & 0x0f;
      if (c <= 9)
	{
	  str[2 * i] = c + '0';
	}
      else
	{
	  str[2 * i] = c - 10 + 'A';
	}

      c = ch & 0x0f;
      if (c <= 9)
	{
	  str[2 * i + 1] = c + '0';
	}
      else
	{
	  str[2 * i + 1] = c - 10 + 'A';
	}
    }
  str[2 * i] = 0;
}

int
ut_is_deleted_oid (T_OBJECT * oid)
{
  T_OBJECT del_oid;

  memset (&del_oid, 0xff, sizeof (del_oid));

  if (oid->pageid == del_oid.pageid && oid->slotid == del_oid.slotid && oid->volid == del_oid.volid)
    {
      return CCI_ER_DELETED_TUPLE;
    }

  return 0;
}


/************************************************************************
 * IMPLEMENTATION OF PRIVATE FUNCTIONS	 				*
 ************************************************************************/

static char
is_float_str (char *str)
{
  char *p;
  char ch;
  int state = 0;

  for (p = str; *p && state >= 0; p++)
    {
      ch = *p;
      switch (state)
	{
	case 0:
	  if (ch == '+' || ch == '-')
	    state = 1;
	  else if (ch == '.')
	    state = 3;
	  else if (ch >= '0' && ch <= '9')
	    state = 2;
	  else
	    state = -1;
	  break;
	case 1:
	  if (ch >= '0' && ch <= '9')
	    state = 2;
	  else
	    state = -1;
	  break;
	case 2:
	  if (ch == '.')
	    state = 3;
	  else if (ch == 'e' || ch == 'E')
	    state = 4;
	  else if (ch >= '0' && ch <= '9')
	    state = 2;
	  else
	    state = -1;
	  break;
	case 3:
	  if (ch >= '0' && ch <= '9')
	    state = 5;
	  else
	    state = -1;
	  break;
	case 4:
	  if (ch == '+' || ch == '-' || (ch >= '0' && ch <= '9'))
	    state = 6;
	  else
	    state = -1;
	  break;
	case 5:
	  if (ch == 'e' || ch == 'E')
	    state = 4;
	  else if (ch >= '0' && ch <= '9')
	    state = 5;
	  else
	    state = -1;
	  break;
	case 6:
	  if (ch >= '0' && ch <= '9')
	    state = 6;
	  else
	    state = -1;
	default:
	  break;
	}
    }

  if (state == 2 || state == 5 || state == 6)
    return 1;

  return 0;
}

int
cci_url_match (const char *src, char *token[])
{
  // *INDENT-OFF*
  using namespace std::regex_constants;
  // *INDENT-ON*
  static const char *pattern =
    "cci:cubrid(-oracle|-mysql)?:([a-zA-Z_0-9\\.-]*):([0-9]*):([^:]+):([^:]*):([^:]*):(\\?[a-zA-Z_0-9]+=[^&=?]+(&[a-zA-Z_0-9]+=[^&=?]+)*)?";
  static const std::regex reg (pattern, ECMAScript | icase | optimize);
  static int match_idx[] = { 2, 3, 4, 5, 6, 7, -1 };

  int error = CCI_ER_NO_ERROR;
  for (int i = 0; match_idx[i] != -1; i++)
    {
      token[i] = NULL;
    }

  // *INDENT-OFF*
  try
    {
      std::cmatch match;
      bool is_matched = std::regex_match (src, match, reg);
      if (is_matched)
	{
	  int num_matches = match.size ();
	  for (int i = 0; match_idx[i] != -1 && match_idx[i] < num_matches; i++)
	    {
	      std::csub_match sub_by_idx = match[match_idx[i]];
	      size_t n = sub_by_idx.length ();
	      token[i] = (char *) MALLOC (n + 1);
	      if (token[i] == NULL)
		{
		  error = CCI_ER_NO_MORE_MEMORY;
		  break;
		}
	      std::string t = sub_by_idx.str ();
	      strncpy (token[i], t.c_str (), n);
	      token[i][n] = '\0';
	    }
	}
      else
	{
	  error = CCI_ER_INVALID_URL;
	}
    }
  catch (std::regex_error &e)
    {
      // regex exception
      if (e.code () == error_stack)
	{
	  error = CCI_ER_NO_MORE_MEMORY;
	}
      else
	{
	  error = CCI_ER_INVALID_URL;
	}
      fprintf (stderr, "regex_error : %s\n", e.what ());
    }
  // *INDENT-ON*

  if (error != CCI_ER_NO_ERROR)
    {
      /* free allocated memory when error was CCI_ER_NO_MORE_MEMORY */
      for (int i = 0; match_idx[i] != -1; i++)
	{
	  FREE_MEM (token[i]);
	}
    }

  return error;
}

long
ut_timeval_diff_msec (struct timeval *start, struct timeval *end)
{
  long diff_msec;
  assert (start != NULL && end != NULL);

  diff_msec = (end->tv_sec - start->tv_sec) * 1000;
  diff_msec += ((end->tv_usec - start->tv_usec) / 1000);

  return diff_msec;
}

static int
get_pm_offset (char *str, int hh)
{
  int len;

  while ((*str) == ' ')
    {
      str++;
    }

  len = (int) strlen (str);

  if ((((len > 2) && (*(str + 2) == ' ')) || (len == 2))
      && ((((*str) == 'p') || ((*str) == 'P')) && ((*(str + 1) == 'm') || (*(str + 1) == 'M'))) && (hh < 12))
    {
      return 12;
    }

  return 0;
}

static int
skip_ampm_chars (char *str)
{
  int ampm_skipped_chars = 0, len = 0;

  while ((*str) == ' ')
    {
      str++;
      ampm_skipped_chars++;
    }

  len = (int) strlen (str);
  if ((len > 2 && (*(str + 2) == ' ')) || (len == 2))
    {
      if (((*str == 'a') || (*str == 'A') || (*str == 'p') || (*str == 'P'))
	  && (((*(str + 1) == 'm') || (*(str + 1) == 'M')) && (*(str + 2) == ' ')))
	{
	  str += 2;
	  ampm_skipped_chars += 2;
	}
    }

  return ampm_skipped_chars;
}
