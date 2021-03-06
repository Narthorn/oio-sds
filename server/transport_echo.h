/*
OpenIO SDS server
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

#ifndef OIO_SDS__server__transport_echo_h
# define OIO_SDS__server__transport_echo_h 1

/**
 * @defgroup server_transecho ECHO transport
 * @ingroup server
 * @brief
 * @details
 * 
 * @{
 */

struct network_client_s;

/**
 * @param u
 * @param clt
 */
void transport_echo_factory(gpointer u, struct network_client_s *clt);

/** @} */

#endif /*OIO_SDS__server__transport_echo_h*/