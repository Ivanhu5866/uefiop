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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <byteswap.h>
#include <unistd.h>
#include <stdbool.h> 
#include <sys/stat.h>
#include <paths.h>
#include <sys/wait.h>

#include "uefiop.h"
#include "utils.h"
#include "uefiop_version.h"

static char *efi_dev_name = NULL;
static char *module_name = NULL;

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
	printf("Version %s, %s\n", UEFIOP_VERSION, UEFIOP_DATE);
}

int check_segment(const char *str, size_t len)
{
	int i;
	for(i = 0; i < len; i++) {
		if ((str[i] >= '0' && str[i] <= '9') || ((str[i] | 0x20) >= 'a' && (str[i] | 0x20) <= 'f'))
			continue;
		return -1;
	}
	return 0;
}

int string_to_guid(const char *str, efi_guid *guid)
{
	int i;
	char bytes_8[9] = "";
	char bytes_4[5] = "";
	char bytes_2[3] = "";
	size_t slen = strlen(str);
	size_t guidlen = strlen("12345678-1234-1234-1234-112233445566");

	if (slen == guidlen + 2) {
		if (str[0] != '{' || str[slen - 1] != '}') {
			return -1;
		}
		str++;
		slen -= 2;
	}

	if (slen != guidlen)
		return -1;

	if (str[8] != '-' || str[13] != '-' || str[18] != '-' ||
			str[23] != '-')
		return -1;

	strncpy(bytes_8, str, 8);
	if (check_segment(bytes_8, 8) < 0)
		return -1;
	guid->a = (uint32_t)strtoul(bytes_8, NULL, 16);

	strncpy(bytes_4, str + 9, 4);
	if (check_segment(bytes_4, 4) < 0)
		return -1;
	guid->b = (uint16_t)strtoul(bytes_4, NULL, 16);

	strncpy(bytes_4, str + 14, 4);
	if (check_segment(bytes_4, 4) < 0)
		return -1;
	guid->c = (uint16_t)strtoul(bytes_4, NULL, 16);

	strncpy(bytes_4, str + 19, 4);
	if (check_segment(bytes_4, 4) < 0)
		return -1;
	guid->d = bswap_16((uint16_t)strtoul(bytes_4, NULL, 16));

	for (i = 0 ; i < 6 ; i++) {
		strncpy(bytes_2, str + 24 + (2 * i), 2);
		if (check_segment(bytes_2, 2) < 0)
			return -1;
		guid->e[i] = (uint8_t)strtoul(bytes_2, NULL, 16);
	}
	return 0;
}

void str_to_ucs(uint16_t *des, const char *str, size_t len)
{
	size_t i;
	for (i = 0; i < len; i++)
		des[i] = (uint16_t)str[i];
	des[i] = 0;

	return;
}

void ucs_to_str(char *des, const uint16_t *str, size_t len)
{
	size_t i;
	for (i = 0; i < (len/2 - 1); i++)
		des[i] = (char)str[i];
	des[i] = 0;

	return;
}

static int check_device(char *devname)
{
	struct stat statbuf;

	if (stat(devname, &statbuf))
		return UEFIOP_ERROR;

	if (S_ISCHR(statbuf.st_mode)) {
		efi_dev_name = devname;
		return UEFIOP_OK;
	}
	return UEFIOP_ERROR;
}

static int check_module_loaded(
	char *module,
	bool *loaded)
{
	FILE *fp;

	*loaded = false;

	if ((fp = fopen("/proc/modules", "r")) != NULL) {
		char buffer[1024];

		while (fgets(buffer, sizeof(buffer), fp) != NULL) {
			if (strstr(buffer, module)) {
				*loaded = true;
				break;
			}
		}
		(void)fclose(fp);
		return UEFIOP_OK;
	}
	printf("Could not open /proc/modules to check if efi module '%s' is loaded.\n", module);

	return UEFIOP_ERROR;
}


static int check_module_loaded_no_dev(char *module)
{
	bool loaded;

	if (check_module_loaded(module, &loaded) != UEFIOP_OK)
		return UEFIOP_OK;
	if (loaded) {
		printf("Module '%s' is already loaded, but device not available.\n", module);
		return UEFIOP_ERROR;
	}
	return UEFIOP_OK;
}

static int uefiop_exec(const char *command)
{
	pid_t pid;

	pid = fork();
	switch (pid) {
	case -1:
		/* Ooops */
		return UEFIOP_ERROR;
	case 0:
		/* Child */
		execl(_PATH_BSHELL, "sh", "-c", command, NULL);
		_exit(UEFIOP_ERROR);
	default:
		/* Parent */
		wait(NULL);
		return UEFIOP_OK;
	}
}

static int load_module(
	char *module,
	char *devname)
{
	char cmd[80];
	bool loaded;

	snprintf(cmd, sizeof(cmd), "modprobe %s", module);

	if (uefiop_exec(cmd) != UEFIOP_OK)
		return UEFIOP_ERROR;

	if (check_module_loaded(module, &loaded) != UEFIOP_OK)
		return UEFIOP_ERROR;

	if (!loaded)
		return UEFIOP_ERROR;

	if (check_device(devname) != UEFIOP_OK)
		return UEFIOP_ERROR;

	module_name = module;

	return UEFIOP_OK;
}


static int lib_load_module()
{
	efi_dev_name = NULL;
	module_name = NULL;

	/* Check if dev is already available */
	if (check_device("/dev/efi_runtime") == UEFIOP_OK)
		return UEFIOP_OK;
	if (check_device("/dev/efi_test") == UEFIOP_OK)
		return UEFIOP_OK;

	/* Since the devices can't be found, the module should be not loaded */
	if (check_module_loaded_no_dev("efi_runtime") != UEFIOP_OK)
		return UEFIOP_ERROR;
	if (check_module_loaded_no_dev("efi_test") != UEFIOP_OK)
		return UEFIOP_ERROR;

	/* Now try to load the module */

	if (load_module("efi_runtime", "/dev/efi_runtime") == UEFIOP_OK)
		return UEFIOP_OK;
	if (load_module("efi_test", "/dev/efi_test") == UEFIOP_OK)
		return UEFIOP_OK;

	printf("Failed to load efi runtime module.\n");
	return UEFIOP_ERROR;
}

static int lib_unload_module()
{
	bool loaded;
	int status;
	char cmd[80], *tmp_name = module_name;

	efi_dev_name = NULL;

	/* No module, not much to do */
	if (!module_name)
		return UEFIOP_OK;

	module_name = NULL;

	/* If it is not loaded, no need to unload it */
	if (check_module_loaded(tmp_name, &loaded) != UEFIOP_OK)
		return UEFIOP_ERROR;
	if (!loaded)
		return UEFIOP_OK;

	snprintf(cmd, sizeof(cmd), "modprobe -r %s", tmp_name);
	if (uefiop_exec(cmd) != UEFIOP_OK) {
		printf("Failed to unload module '%s'.\n", tmp_name);
		return UEFIOP_ERROR;
	}

	/* Module should not be loaded at this point */
	if (check_module_loaded(tmp_name, &loaded) != UEFIOP_OK)
		return UEFIOP_ERROR;
	if (loaded) {
		printf("Failed to unload module '%s'.\n", tmp_name);
		return UEFIOP_ERROR;
	}

	return UEFIOP_OK;
}

static int lib_efi_runtime_open(void)
{

	if (!efi_dev_name)
		return -1;

	return open(efi_dev_name, O_WRONLY | O_RDWR);
}

static int lib_efi_runtime_close(int fd)
{
	return close(fd);
}

int init_driver(void)
{

	int fd;

	if (lib_load_module() != UEFIOP_OK) {
		printf("Cannot load efi runtime module. Aborted.\n");
		return UEFIOP_ERROR;
	}

	fd = lib_efi_runtime_open();
	if (fd == -1) {
		printf("Cannot open efi runtime driver. Aborted.\n");
		return UEFIOP_ERROR;
	}

	return fd;
}

void deinit_driver(int fd)
{
	if (fd != -1)
		lib_efi_runtime_close(fd);
	lib_unload_module();
}
