/*
 * Copyright (C) 2017 Ivan Hu <ivan.hu@canonical.com>
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
	{ "size", required_argument, NULL, 's' },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	{ NULL, 0, NULL, 0 },
};

static void usage(void)
{
	printf("Usage: %s [options] --size <size> --guid <guid> --name <varname> "
			"--file <varfile>\n"
		"This application helps to enumerates the current variable names with runtime services.\n\n"
		"Options:\n"
		"\t--size -s <size>	The size of the VariableName buffer\n"
		"\t	ex. uefigetnextvarname -s 512\n"
		"\t--version -V		show version\n"
		"\t--help -h		show this menu\n",
		"uefigetnextvarname");
}

static int uefigetnextvarname(
	int fd,
	uint64_t *size,
	uint16_t *varname,
	efi_guid *guid,
	uint64_t *status)
{
	int ioret;
	struct efi_getnextvariablename getnextvariablename;

	getnextvariablename.VariableNameSize = size;
	getnextvariablename.VariableName = varname;
	getnextvariablename.VendorGuid = (EFI_GUID *)guid;
	getnextvariablename.status = status;
	ioret = ioctl(fd, EFI_RUNTIME_GET_NEXTVARIABLENAME, &getnextvariablename);

	return ioret;

}

int main(int argc, char **argv)
{

	int c;
	efi_guid guid;
	uint16_t *varnamebuffer = NULL;
	uint64_t varnamesize = 0;
	uint64_t bufffersize = 0;
	uint64_t status;
	bool got_size = false;
	char *str = NULL;

	for (;;) {
		int idx;
		c = getopt_long(argc, argv, "s:Vh", options, &idx);
		if (c == -1)
			break;

		switch (c) {
		case 's':
			bufffersize = strtoul(optarg, NULL, 10);
			got_size = true;
			break;
		case 'V':
			version();
			return EXIT_SUCCESS;
		case 'h':
			usage();
			return EXIT_SUCCESS;
		}
	}

	fd = init_driver();
	if (fd == -1) {
		printf ("Cannot open efi_runtime driver. Aborted.\n");
		goto error;
	}

	if (!got_size) {
		printf ("Need to input the variable name buffer szie\n");
		goto error;
	}

	if (bufffersize < 2) {
		printf ("Variable name buffer size should lager or equal to 2 "
			"bytes for Null-terminated string of UCS\n");
		goto error;
	}

	varnamebuffer = malloc(bufffersize);
	if (!varnamebuffer) {
		printf ("error: cannot alloc memory for variable name buffer\n");
		goto error;
	}

	str = malloc(bufffersize/2);
	if (!str) {
		printf ("error: cannot alloc memory\n");
		goto error;
	}

	/* stare search */
	varnamebuffer[0] = '\0';
	while (true) {

		varnamesize = bufffersize;
		uefigetnextvarname(fd, &varnamesize, varnamebuffer, &guid, &status);

		if (status != EFI_SUCCESS) {

			/* no next variable was found*/
			if (status == EFI_NOT_FOUND) {
				printf ("Searching completed.\n");
				break;
			}

			/*
			 * The buffer we provided is too small for the name 
			 */
			if (status == EFI_BUFFER_TOO_SMALL) {
				printf ("The buffer is too small, need %ld bytes. "
					"Can specify more buffer and try again.\n",
					varnamesize);
				break;
			}

			/* other errors */
			print_status_info(status);
			break;
		}

		ucs_to_str(str, varnamebuffer, varnamesize);
		printf ("VariableName: %s\n", str);
		printf ("VendorGuid: %08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x\n",
			guid.a, guid.b, guid.c, guid.d, guid.e[0], guid.e[1],
			guid.e[2], guid.e[3], guid.e[4], guid.e[5]);
	}

	if (str)
		free(str);	

	if (varnamebuffer)
		free(varnamebuffer);

	deinit_driver(fd);

	return EXIT_SUCCESS;

error:

	if (varnamebuffer)
		free(varnamebuffer);

	if (str)
		free(str);

	deinit_driver(fd);

	return EXIT_FAILURE;
}

