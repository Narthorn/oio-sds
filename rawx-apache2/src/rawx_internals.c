/*
OpenIO SDS rawx-apache2
Copyright (C) 2014 Worldine, original work as part of Redcurrant
Copyright (C) 2015 OpenIO, modified as part of OpenIO Software Defined Storage

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include <rawx-lib/src/rawx.h>

#include "rawx_internals.h"
#include "rawx_repo_core.h"
#include "rawx_config.h"

/******************** RESOURCE UTILY FUNCTIONS *******************/

dav_rawx_server_conf*
resource_get_server_config(const dav_resource *resource)
{
	return request_get_server_config(resource->info->request);
}

apr_pool_t *
resource_get_pool(const dav_resource *resource)
{
	return resource->info->pool;
}
const char *
resource_get_pathname(const dav_resource *resource)
{
	return resource->info->fullpath;
}

/******************** REQUEST UTILITY FUNCTIONS ******************/

dav_rawx_server_conf*
request_get_server_config(const request_rec *r)
{
	dav_rawx_server_conf *conf;

	conf = ap_get_module_config(r->server->module_config, &dav_rawx_module);
	return conf;
}

apr_uint64_t
request_get_duration(const request_rec *req)
{
	apr_uint64_t t = 0;
	apr_time_t d = apr_time_now() - req->request_time;
	t = d;
	return t;
}

/*************** OTHER *********************/

void
str_replace_by_pooled_str(apr_pool_t *pool, char ** pstr)
{
	char *str_orig = *pstr;
	*pstr = NULL;
	if (str_orig) {
		*pstr = apr_pstrdup(pool, str_orig);
		g_free(str_orig);
	}
}

/* Note: picked up from ap_gm_timestr_822() */
/* NOTE: buf must be at least DAV_TIMEBUF_SIZE chars in size */
void
dav_format_time(int style, apr_time_t sec, char *buf)
{
	apr_time_exp_t tms;

	/* ### what to do if fails? */
	(void) apr_time_exp_gmt(&tms, sec);

	if (style == DAV_STYLE_ISO8601) {
		/* ### should we use "-00:00" instead of "Z" ?? */

		/* 20 chars plus null term */
		sprintf(buf, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2dZ",
				tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday,
				tms.tm_hour, tms.tm_min, tms.tm_sec);
		return;
	}

	/* RFC 822 date format; as strftime '%a, %d %b %Y %T GMT' */

	/* 29 chars plus null term */
	sprintf(buf,
			"%s, %.2d %s %d %.2d:%.2d:%.2d GMT",
			apr_day_snames[tms.tm_wday],
			tms.tm_mday, apr_month_snames[tms.tm_mon],
			tms.tm_year + 1900,
			tms.tm_hour, tms.tm_min, tms.tm_sec);
}
