/*
OpenIO SDS metautils
Copyright (C) 2014 Worldine, original work as part of Redcurrant
Copyright (C) 2015 OpenIO, modified as part of OpenIO Software Defined Storage

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.
*/

#include "metautils.h"

gint
l4v_fill(void *src, gsize srcSize, void *dst, gsize dstSize, GError ** error)
{
	if (NULL == src || srcSize < 1 || NULL == dst || dstSize < 4) {
		GSETERROR(error, "Invalid parameter");
		return 0;
	}

	if (dstSize < srcSize - 4) {
		GSETERROR(error, "Buffer to small");
		return 0;
	}

	l4v_prepend_size(dst, dstSize);
	memcpy(((guint8 *) dst) + 4, src, srcSize);
	return 1;
}

GByteArray *
l4v_read_2to(int fd, gint ms1, gint msAll, GError ** err)
{
	gint rc = 0;
	int nbRecv = 0;
	GByteArray *gba = NULL;
	guint8 recvBuf[4096];
	gsize msgSize = 0;

	/* the size */
	rc = sock_to_read_size(fd, ms1, recvBuf, 4, err);
	if (rc < 4) {
		GSETERROR(err, "Failed to read %d bytes on socket", 4);
		return NULL;
	}

	msgSize = l4v_get_size(recvBuf);
	gba = g_byte_array_sized_new(MIN(msgSize + 4 + 4, 16384));

	if (NULL == gba) {
		GSETERROR(err, "Cannot create a pre-allocated buffer");
		return NULL;
	}

	gba = g_byte_array_append(gba, recvBuf, 4);

	/* the remaining */
	while (gba->len < msgSize + 4) {
		nbRecv = sock_to_read(fd, msAll, recvBuf,
				MIN(sizeof(recvBuf), msgSize + 4 - gba->len), err);
		if (nbRecv <= 0) {
			GSETERROR(err, "Read failed after %i bytes", gba->len);
			g_byte_array_free(gba, TRUE);
			return NULL;
		}
		else {
			if (!g_byte_array_append(gba, recvBuf, nbRecv)) {
				GSETERROR(err, "Memory allocation failure");
				g_byte_array_free(gba, TRUE);
				return NULL;
			}
		}
	}

	return gba;
}

GByteArray *
l4v_read(int fd, gint ms, GError ** err)
{
	return l4v_read_2to(fd, ms, ms, err);
}

