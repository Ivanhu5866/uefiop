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
#include <sys/ioctl.h>
#include <getopt.h>

#include <efi_runtime.h>
#include "utils.h"

static int fd;

static struct option options[] = {
	{ "gettime", no_argument, NULL, 'g' },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	{ NULL, 0, NULL, 0 },
};

static void usage(void)
{
	printf("Usage: %s [options] --get \n"
		"This application helps to get and set current time date information with runtime services.\n\n"
		"Options:\n"
		"\t--gettime -g		get current time and date information\n"
		"\t	ex. uefitime -g \n"
		"\t--version -V		show version\n"
		"\t--help -h		show this menu\n",
		"uefitime");
}

int main(int argc, char **argv)
{
	int c;
	struct efi_gettime gettime;
	EFI_TIME efi_time;
	EFI_TIME_CAPABILITIES efi_time_cap;
	bool get = false;
	uint64_t status;

	for (;;) {
		int idx;
		c = getopt_long(argc, argv, "gVh", options, &idx);
		if (c == -1)
			break;

		switch (c) {
		case 'g':
			get = true;
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
		return EXIT_FAILURE;
	}

	if (get) {
		gettime.Capabilities = &efi_time_cap;
		gettime.Time = &efi_time;
		gettime.status = &status;

		ioctl(fd, EFI_RUNTIME_GET_TIME, &gettime);

		if (status == EFI_SUCCESS) {
			printf ("Year:       %d\n", gettime.Time->Year);
			printf ("Month:      %d\n", gettime.Time->Month);
			printf ("Day:        %d\n", gettime.Time->Day);
			printf ("Hour:       %d\n", gettime.Time->Hour);
			printf ("Minute:     %d\n", gettime.Time->Minute);
			printf ("Second:     %d\n", gettime.Time->Second);
			printf ("Pad1:       %d\n", gettime.Time->Pad1);
			printf ("Nanosecond: %d\n", gettime.Time->Nanosecond);
			printf ("TimeZone:   %d\n", gettime.Time->TimeZone);
			printf ("Daylight:   %d\n", gettime.Time->Daylight);
			printf ("Pad2:       %d\n", gettime.Time->Pad2);
		}
		print_status_info(status);
	}

	deinit_driver(fd);

	return EXIT_SUCCESS;

}

