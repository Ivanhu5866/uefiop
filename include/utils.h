/*
 * Copyright (C) 2016-2017 Ivan Hu <ivan.hu@canonical.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the OpenSSL
 * library under certain conditions as described in each individual source file,
 * and distribute linked combinations including the two.
 *
 * You must obey the GNU General Public License in all respects for all
 * of the code used other than OpenSSL. If you modify file(s) with this
 * exception, you may extend this exception to your version of the
 * file(s), but you are not obligated to do so. If you do not wish to do
 * so, delete this exception statement from your version. If you delete
 * this exception statement from all source files in the program, then
 * also delete it here.
 */

#ifndef _UEFIOP_UTILS_
#define _UEFIOP_UTILS_

#define BITS_PER_LONG	(sizeof(long) * 8)

#define HIGH_BIT_SET	(1UL << (BITS_PER_LONG-1))

#define EFI_SUCCESS			0
#define EFI_LOAD_ERROR			(1 | HIGH_BIT_SET)
#define EFI_INVALID_PARAMETER		(2 | HIGH_BIT_SET)
#define EFI_UNSUPPORTED			(3 | HIGH_BIT_SET)
#define EFI_BAD_BUFFER_SIZE 		(4 | HIGH_BIT_SET)
#define EFI_BUFFER_TOO_SMALL		(5 | HIGH_BIT_SET)
#define EFI_NOT_READY			(6 | HIGH_BIT_SET)
#define EFI_DEVICE_ERROR		(7 | HIGH_BIT_SET)
#define EFI_WRITE_PROTECTED		(8 | HIGH_BIT_SET)
#define EFI_OUT_OF_RESOURCES		(9 | HIGH_BIT_SET)
#define EFI_VOLUME_CORRUPTED		(10 | HIGH_BIT_SET)
#define EFI_VOLUME_FULL			(11 | HIGH_BIT_SET)
#define EFI_NO_MEDIA			(12 | HIGH_BIT_SET)
#define EFI_MEDIA_CHANGED		(13 | HIGH_BIT_SET)
#define EFI_NOT_FOUND			(14 | HIGH_BIT_SET)
#define EFI_ACCESS_DENIED		(15 | HIGH_BIT_SET)
#define EFI_NO_RESPONSE			(16 | HIGH_BIT_SET)
#define EFI_NO_MAPPING			(17 | HIGH_BIT_SET)
#define EFI_TIMEOUT			(18 | HIGH_BIT_SET)
#define EFI_NOT_STARTED			(19 | HIGH_BIT_SET)
#define EFI_ALREADY_STARTED		(20 | HIGH_BIT_SET)
#define EFI_ABORTED			(21 | HIGH_BIT_SET)
#define EFI_ICMP_ERROR			(22 | HIGH_BIT_SET)
#define EFI_TFTP_ERROR			(23 | HIGH_BIT_SET)
#define EFI_PROTOCOL_ERROR		(24 | HIGH_BIT_SET)
#define EFI_INCOMPATIBLE_VERSION	(25 | HIGH_BIT_SET)
#define EFI_SECURITY_VIOLATION		(26 | HIGH_BIT_SET)
#define EFI_CRC_ERROR			(27 | HIGH_BIT_SET)
#define EFI_END_OF_MEDIA		(28 | HIGH_BIT_SET)
#define EFI_END_OF_FILE			(31 | HIGH_BIT_SET)
#define EFI_INVALID_LANGUAGE		(32 | HIGH_BIT_SET)
#define EFI_COMPROMISED_DATA		(33 | HIGH_BIT_SET)
#define EFI_IP_ADDRESS_CONFLICT		(34 | HIGH_BIT_SET)
#define EFI_HTTP_ERROR			(35 | HIGH_BIT_SET)

typedef struct {
	uint32_t	a;
	uint16_t	b;
	uint16_t	c;
	uint16_t	d;
	uint8_t		e[6];
} efi_guid;

void print_status_info(const uint64_t status);
void version(void);
int init_driver(void);
void deinit_driver(int fd);
int check_segment(const char *str, size_t len);
int string_to_guid(const char *str, efi_guid *guid);
void str_to_ucs(uint16_t *des, const char *str, size_t len);
void ucs_to_str(char *des, const uint16_t *str, size_t len);

#endif /* _UEFIOP_UTILS_ */

