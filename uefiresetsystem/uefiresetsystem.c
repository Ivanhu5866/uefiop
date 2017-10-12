/*
 * Copyright (C)2017 Ivan Hu <ivan.hu@canonical.com>
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
	{ "type", required_argument, NULL, 't' },
	{ "status", required_argument, NULL, 's' },
	{ "size", required_argument, NULL, 'z' },
	{ "data", required_argument, NULL, 'd' },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	{ NULL, 0, NULL, 0 },
};

static void usage(void)
{
	printf("Usage: %s [options] --type <guid> --name <varname> --data <data> "
			"--file <varfile>\n"
		"This application helps to set UEFI variable with runtime services.\n\n"
		"Options:\n"
		"\t--type -t <ResetType>	the type of reset to perform\n"
		"\t--status -s <ResetStatus>	the status code for the reset\n"
		"\t--size -z <DataSize>		the size, in bytes, of ResetData\n"
		"\t--data -d <data>		the date buffer\n"
		"\t	ex. uefiresetsystem -t 0 -s 0 -z 0\n"
		"\t	ex. uefiresetsystem -t 0 -s 0 -z 5 -d \"01 02 10 12 33\"\n"
		"\t--version -V		show version\n"
		"\t--help -h		show this menu\n",
		"uefiresetsystem");
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

int main(int argc, char **argv)
{

	int c;
	int type = 0;
	uint64_t data_size = 0;
	uint8_t *data = NULL;
	uint64_t status = 0;
	char *str = NULL;
	uint64_t datalen = 0;
	struct efi_resetsystem resetsystem;

	for (;;) {
		int idx;
		c = getopt_long(argc, argv, "t:s:z:d:Vh", options, &idx);
		if (c == -1)
			break;

		switch (c) {
		case 't':
			type = strtol(optarg, NULL, 16);
			break;
		case 's':
			status = strtoul(optarg, NULL, 16);
			break;
		case 'z':
			data_size = strtoul(optarg, NULL, 10);
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
		printf ("Cannot open efi_test or efi_runtime driver. Aborted.\n");
		goto error;
	}

	resetsystem.reset_type = type;
	resetsystem.status = status;
	resetsystem.data_size = data_size;
	resetsystem.data = (uint16_t *)data;

	ioctl(fd, EFI_RUNTIME_RESET_SYSTEM, &resetsystem);

	if (data)
		free(data);

	deinit_driver(fd);

	return EXIT_SUCCESS;

error:
	if (data)
		free(data);

	if (str)
		free(str);

	deinit_driver(fd);

	return EXIT_FAILURE;
}

