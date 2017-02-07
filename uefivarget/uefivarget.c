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
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <getopt.h>

#include <efi_runtime.h>

#include "uefiop.h"
#include "utils.h"

static int fd = -1;

static struct option options[] = {
	{ "guid", required_argument, NULL, 'g' },
	{ "name", required_argument, NULL, 'n' },
	{ "file", required_argument, NULL, 'f' },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	{ NULL, 0, NULL, 0 },
};

static void usage(void)
{
	printf("Usage: %s [options] --guid <guid> --name <varname> --data <data> "
			"--file <varfile>\n"
		"This application helps to get UEFI variable with runtime services.\n\n"
		"Options:\n"
		"\t--guid -g <guid>	the guid of the variable\n"
		"\t	ex. uefivarget -g 12345678-1234-1234-1234-112233445566\n"
		"\t--name -n <varname>	the name of the variable\n"
		"\t	ex. uefivarget -n Test\n" 		
		"\t--file -f <file>	store the date of the variable to the file\n"
		"\t	ex. uefivarget -f test.dat\n"
		"\t--version -V		show version\n"
		"\t--help -h		show this menu\n",
		"uefivarget");
}

static int variableget(
	int fd,
	uint64_t *datasize,
	uint16_t *varname,
	uint8_t *data,
	efi_guid *guid,
	uint32_t *attr,
	uint64_t *status)
{
	int ioret;
	struct efi_getvariable getvariable;

	getvariable.VariableName = varname;
	getvariable.VendorGuid = (EFI_GUID *)guid;
	getvariable.Attributes = attr;
	getvariable.DataSize = datasize;
	getvariable.Data = data;
	getvariable.status = status;
	ioret = ioctl(fd, EFI_RUNTIME_GET_VARIABLE, &getvariable);

	return ioret;

}

int main(int argc, char **argv)
{

	int c;
	int rc;
	efi_guid guid;
	uint16_t *varname = NULL;
	size_t varlen = 0;
	uint64_t datalen = 1024;
	uint8_t *data = NULL;
	uint64_t status;
	bool got_guid = false;
	uint32_t attributes;
	int i;
	FILE *fp = NULL;
	size_t iwrite;

	for (;;) {
		int idx;
		c = getopt_long(argc, argv, "g:n:f:Vh", options, &idx);
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
		case 'f':
			fp = fopen(optarg, "wb");
			if (!fp) {
			printf("error: cannot open file\n");
				goto error;
			}
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

	data = malloc(datalen);
	if (!data) {
		printf ("error: cannot alloc memory for data\n");
		goto error;
	}

	fd = init_driver();
	if (fd == -1) {
		printf ("Cannot open efi_runtime driver. Aborted.\n");
		goto error;
	}

	variableget(fd, &datalen, varname, data, &guid, &attributes, &status);

	if (status == EFI_BUFFER_TOO_SMALL) {
		data = realloc(data, datalen);
		if (!data) {
			printf ("error: cannot realloc memory for data\n");
			goto error;
		}
		variableget(fd, &datalen, varname, data, &guid, &attributes,
				&status);
	}

	if (status == EFI_SUCCESS) {
		if (fp) {
			iwrite = fwrite(data, 1, datalen, fp);
			if (iwrite < datalen) {
				printf ("error: fail to write data to file\n");
				goto error;
			}
			fclose(fp);
		} else  {
			printf ("Data: \n");
			for (i = 0; i < datalen; i++)
				printf("%2.2x", data[i]);
			printf ("\n");
		}
	}
	print_status_info(status);

	if (varname)
		free(varname);

	if (data)
		free(data);

	deinit_driver(fd);

	return EXIT_SUCCESS;

error:
	if (varname)
		free(varname);

	if (data)
		free(data);

	if (fp)
		fclose(fp);

	deinit_driver(fd);

	return EXIT_FAILURE;
}

