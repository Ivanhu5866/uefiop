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
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <byteswap.h>

#include <efi_runtime.h>

#include "uefivarset.h"
#include "utils.h"

static uint32_t attributes =
	EFI_VARIABLE_NON_VOLATILE |
	EFI_VARIABLE_BOOTSERVICE_ACCESS |
	EFI_VARIABLE_RUNTIME_ACCESS;

typedef struct {
	uint32_t	a;
	uint16_t	b;
	uint16_t	c;
	uint16_t	d;
	uint8_t		e[6];
} efi_guid;

static struct option options[] = {
	{ "guid", required_argument, NULL, 'g' },
	{ "data", required_argument, NULL, 'd' },
	{ "name", required_argument, NULL, 'n' },
	{ "file", required_argument, NULL, 'f' },
	{ "delete", required_argument, NULL, 'D' },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	{ NULL, 0, NULL, 0 },
};

static void usage(void)
{
	printf("Usage: %s [options] --guid <guid> --name <varname> --data <data> "
			"--file <varfile>\n"
		"This application helps to set UEFI variable with runtime services.\n\n"
		"Options:\n"
		"\t--guid -g <guid>	the guid of the variable\n"
		"\t	ex. uefivarset -g 12345678-1234-1234-1234-112233445566\n"
		"\t--name -n <varname>	the name of the variable\n"
		"\t	ex. uefivarset -n Test\n" 		
		"\t--data -d <data>	the date in hex of the variable\n"
		"\t	ex. uefivarset -d \"11 22 33 ff\"\n"
		"\t--file -f <file>	the date of the variable\n"
		"\t	ex. uefivarset -f test.dat\n"
		"\t	if data and file exist at the same time, the data will be set\n"
		"\t--delete -D <file>	delete the variable\n"
		"\t	ex. uefivarset -g 12345678-1234-1234-1234-112233445566 -name Test -D\n"
		"\t--version -V		show version\n"
		"\t--help -h		show this menu\n",
		"uefivarset");
}

static int init_driver(void)
{
	int fd;

	fd = open("/dev/efi_runtime", O_WRONLY | O_RDWR);
	return fd;
}

static int check_segment(const char *str, size_t len)
{
	int i;
	for(i = 0; i < len; i++) {
		if ((str[i] >= '0' && str[i] <= '9') || ((str[i] | 0x20) >= 'a' && (str[i] | 0x20) <= 'f'))
			continue;
		return -1;
	}
	return 0;
}

static int string_to_guid(const char *str, efi_guid *guid)
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

static void str_to_ucs(uint16_t *des, const char *str, size_t len)
{
	size_t i;
	for (i = 0; i < len; i++)
		des[i] = (uint16_t)str[i];
	des[i] = 0;

	return;
}

static size_t get_data_len(char *str)
{
	int i = 0;
	char *pch;
        char *saveptr1;
	pch = strtok_r(str," ,", &saveptr1);
	while (pch != NULL) {
		if (strlen(pch) != 2 || check_segment(pch, 2)) {
			printf ("Data error!\n");
			return -1;
		}
		i++;
		pch = strtok_r(NULL," ,", &saveptr1);
	}

	return i;
}

static void get_data(char *str, uint8_t *data)
{
	int i = 0;
	char *pch;
        char *saveptr1;
	pch = strtok_r(str," ,", &saveptr1);
	while (pch != NULL) {
		if (strlen(pch) != 2 || check_segment(pch, 2)) {
			printf ("Data error!\n");
			return;
		}
		data[i] = strtol(pch, NULL, 16);
		i++;
		pch = strtok_r(NULL," ,", &saveptr1);
	}

	return;
}

static int variableset(
	int fd,
	const uint64_t datasize,
	uint16_t *varname,
	uint8_t *data,
	efi_guid *gtestguid,
	uint64_t *status)
{
	int ioret;
	struct efi_setvariable setvariable;

	setvariable.VariableName = varname;
	setvariable.VendorGuid = (EFI_GUID *)gtestguid;
	setvariable.Attributes = attributes;
	setvariable.DataSize = datasize;
	setvariable.Data = data;
	setvariable.status = status;
	ioret = ioctl(fd, EFI_RUNTIME_SET_VARIABLE, &setvariable);

	printf ("status 0x%lx\n", *status);

	return ioret;

}

int main(int argc, char **argv)
{

	int c, fd;
	int rc;
	efi_guid guid;
	uint16_t *varname;
	size_t varlen = 0;
	uint64_t datalen = 0;
	uint8_t *data = NULL;
	uint8_t *fdata = NULL;
	uint64_t status;
	char *str;
	bool got_guid = false;
	FILE *fp;
	uint64_t flen = 0, frlen = 0;
	bool del_var = false;

	fd = init_driver();
	if (fd == -1) {
		printf ("Cannot open efi_runtime driver. Aborted.\n");
		return EXIT_FAILURE;
	}

	for (;;) {
		int idx;
		c = getopt_long(argc, argv, "g:n:d:f:VhD", options, &idx);
		if (c == -1)
			break;

		switch (c) {
		case 'g':
			rc = string_to_guid(optarg, &guid);
			if (rc != 0) {
				printf ("Invalid guid:  \"%s\"\n", optarg);
				goto error;
			}
			got_guid = true;
			break;
		case 'n':
			varlen = strlen(optarg);
			varname = malloc((varlen + 1) * 2);
			if (!varname) {
				printf ("error: cannot alloc memory\n");
				goto error;
			}
			str_to_ucs(varname, optarg, varlen);
			break;
		case 'd':
			str = strdup(optarg);
			if (!str) {
				printf ("error: insufficient memory was available\n");
				goto error;
			}
			datalen = get_data_len(optarg);
			if (datalen < 0)
				goto error;
			if (datalen != 0) {
				data = (uint8_t *)malloc(datalen + 1);
				if (!data) {
					printf ("error: cannot alloc memory\n");
					goto error;
				}
			}
			get_data(str, data);
			free(str);
			break;
		case 'f':
			fp = fopen(optarg, "rb");
    			if (!fp) {
        			printf("error: cannot open file\n");
				goto error;
			}
			fseek(fp, 0L, SEEK_END);
			flen = ftell(fp);
			rewind(fp);
			if (flen != 0) {
				fdata = (uint8_t *)malloc(flen + 1);
				if (!fdata) {
					printf ("error: cannot alloc memory\n");
					goto error;
				}
				frlen = fread(fdata, flen, 1, fp);
				if (!frlen) {
					printf ("error: cannot read file\n");
					goto error;
				}
			}
			fclose(fp);
			break;
		case 'D':
			del_var = true;
			break;
		case 'V':
			version();
			return EXIT_SUCCESS;
		case 'h':
			usage();
			return EXIT_SUCCESS;
		}
	}

	if (varlen == 0) {
		printf ("need to input the variable name\n");
		goto error;
	}
	if (!got_guid) {
		printf ("need to input the GUID for varialbe\n");
		goto error;
	}

	if (datalen == 0 && flen != 0)	{
		datalen = flen;
		if (!data)
			free(data);
		data = fdata;
		fdata = NULL;
	}

	if (del_var)
		datalen = 0;

	variableset(fd, datalen, varname, data, &guid, &status);
	if (status != 0)
		printf ("setvariable error! status: 0x%lx\n", status);

	if (!varname)
		free(varname);

	if (!data)
		free(fdata);

	if (!fdata)
		free(data);

	close(fd);

	return EXIT_SUCCESS;

error:
	if (!varname)
		free(varname);

	if (!data)
		free(data);

	if (!str)
		free(str);

	if (!fdata)
		free(fdata);

	close(fd);

	return EXIT_FAILURE;
}

