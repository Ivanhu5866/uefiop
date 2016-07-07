/*
 * Copyright (C) 2016 Ivan Hu <ivan.hu@canonical.com>
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

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include "utils.h"

typedef struct {
	const uint64_t statusvalue;
	const char *mnemonic ;
	const char *description;
} uefistatus_info;

static uefistatus_info uefistatus_info_table[] = {
	{ EFI_SUCCESS,			"EFI_SUCCESS",			"The operation completed successfully." },
	{ EFI_LOAD_ERROR,		"EFI_LOAD_ERROR",		"The image failed to load." },
	{ EFI_INVALID_PARAMETER,	"EFI_INVALID_PARAMETER",	"A parameter was incorrect." },
	{ EFI_UNSUPPORTED,		"EFI_UNSUPPORTED", 		"The operation is not supported." },
	{ EFI_BAD_BUFFER_SIZE,		"EFI_BAD_BUFFER_SIZE",		"The buffer was not the proper size for the request." },
	{ EFI_BUFFER_TOO_SMALL,		"EFI_BUFFER_TOO_SMALL",		"The buffer is not large enough to hold the requested data. The required buffer size is returned in the appropriate parameter when this error occurs." },
	{ EFI_NOT_READY,		"EFI_NOT_READY", 		"There is no data pending upon return." },
	{ EFI_DEVICE_ERROR,		"EFI_DEVICE_ERROR", 		"The physical device reported an error while attempting the operation." },
	{ EFI_WRITE_PROTECTED,		"EFI_WRITE_PROTECTED", 		"The device cannot be written to." },
	{ EFI_OUT_OF_RESOURCES,		"EFI_OUT_OF_RESOURCES", 	"A resource has run out." },
	{ EFI_VOLUME_CORRUPTED,		"EFI_VOLUME_CORRUPTED", 	"An inconstancy was detected on the file system causing the operating to fail." },
	{ EFI_VOLUME_FULL,		"EFI_VOLUME_FULL",		"There is no more space on the file system." },
	{ EFI_NO_MEDIA,			"EFI_NO_MEDIA",			"The device does not contain any medium to perform the operation." },
	{ EFI_MEDIA_CHANGED,		"EFI_MEDIA_CHANGED",		"The medium in the device has changed since the last access." },
	{ EFI_NOT_FOUND,		"EFI_NOT_FOUND",		"The item was not found." },
	{ EFI_ACCESS_DENIED,		"EFI_ACCESS_DENIED",		"Access was denied." },
	{ EFI_NO_RESPONSE,		"EFI_NO_RESPONSE",		"The server was not found or did not respond to the request." },
	{ EFI_NO_MAPPING,		"EFI_NO_MAPPING",		"A mapping to a device does not exist." },
	{ EFI_TIMEOUT,			"EFI_TIMEOUT",			"The timeout time expired." },
	{ EFI_NOT_STARTED,		"EFI_NOT_STARTED",		"The protocol has not been started." },
	{ EFI_ALREADY_STARTED,		"EFI_ALREADY_STARTED",		"The protocol has already been started." },
	{ EFI_ABORTED,			"EFI_ABORTED",			"The operation was aborted." },
	{ EFI_ICMP_ERROR,		"EFI_ICMP_ERROR",		"An ICMP error occurred during the network operation." },
	{ EFI_TFTP_ERROR,		"EFI_TFTP_ERROR",		"A TFTP error occurred during the network operation." },
	{ EFI_PROTOCOL_ERROR,		"EFI_PROTOCOL_ERROR",		"A protocol error occurred during the network operation." },
	{ EFI_INCOMPATIBLE_VERSION,	"EFI_INCOMPATIBLE_VERSION",	"The function encountered an internal version that was incompatible with a version requested by the caller." },
	{ EFI_SECURITY_VIOLATION,	"EFI_SECURITY_VIOLATION",	"The function was not performed due to a security violation." },
	{ EFI_CRC_ERROR,		"EFI_CRC_ERROR",		"A CRC error was detected." },
	{ EFI_END_OF_MEDIA,		"EFI_END_OF_MEDIA",		"Beginning or end of media was reached." },
	{ EFI_END_OF_FILE,		"EFI_END_OF_FILE",		"The end of the file was reached." },
	{ EFI_INVALID_LANGUAGE,		"EFI_INVALID_LANGUAGE",		"The language specified was invalid." },
	{ EFI_COMPROMISED_DATA,		"EFI_COMPROMISED_DATA",		"The security status of the data is unknown or compromised and the data must be updated or replaced to restore a valid security status." },
	{ EFI_IP_ADDRESS_CONFLICT,	"EFI_IP_ADDRESS_CONFLICT",	"There is an address conflict address allocation." },
	{ EFI_HTTP_ERROR,		"EFI_HTTP_ERROR",		"A HTTP error occurred during the network operation." },
	{ ~0, NULL, NULL }
};

void print_status_info(const uint64_t status)
{
	uefistatus_info *info;

	for (info = uefistatus_info_table; info->mnemonic != NULL; info++) {
		if (status == info->statusvalue) {
			printf("Return status: %s. %s\n", info->mnemonic, info->description);
			return;
		}
	}
	printf("Cannot find the return status information, value = 0x%lx\n.", status);
}

void version(void)
{
	printf("version 0.1.0\n");
}

int init_driver(void)
{
	return open("/dev/efi_runtime", O_WRONLY | O_RDWR);
}

void deinit_driver(int fd)
{
	close(fd);
}
