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
#include <sys/ioctl.h>
#include <getopt.h>
#include <string.h>

#include <efi_runtime.h>
#include "utils.h"

static int fd;

static struct option options[] = {
	{ "gettime", no_argument, NULL, 'g' },
	{ "settime", required_argument, NULL, 's' },
	{ "getwakeup", no_argument, NULL, 'G' },
	{ "setwakeup", required_argument, NULL, 'S' },
	{ "help", no_argument, NULL, 'h' },
	{ "version", no_argument, NULL, 'V' },
	{ NULL, 0, NULL, 0 },
};

static void usage(void)
{
	printf("Usage: %s [options] --get --set <time>\n"
		"This application helps to get and set current time date information with runtime services.\n\n"
		"Options:\n"
		"\t--gettime -g		get current time and date information\n"
		"\t	ex. uefitime -g \n"
		"\t--settime -s		set current time and date information\n"
		"\t	<time>  \"Year:Month:Day:Hour:Minute:Second:Pad1:Nanosecond:TimeZone:Daylight:Pad2\"\n"
		"\t	ex. uefitime -s \"2016:10:01:02:10:20:0:0:8:1:0\"\n"
		"\t--getwakeup -G	get current wakeup alarm clock setting\n"
		"\t	ex. uefitime -G \n"
		"\t--Setwakeup -S	set current wakeup alarm clock setting\n"
		"\t	uefitime -S <enable>,<time>\n"
		"\t	ex. uefitime -S \"True,2016:10:01:02:10:20:0:0:8:1:0\"\n"
		"\t	ex. uefitime -S \"False\"\n"
		"\t--version -V		show version\n"
		"\t--help -h		show this menu\n",
		"uefitime");
}

static void print_time_info(EFI_TIME *time, EFI_TIME_CAPABILITIES *cap)
{
	if (time) {
		printf ("TIME\n");
		printf ("  [%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d]\n", time->Year,
			time->Month, time->Day, time->Hour, time->Minute,
			time->Second, time->Pad1, time->Nanosecond,
			time->TimeZone, time->Daylight, time->Pad2);
 		printf ("  Year:       %d\n", time->Year);
 		printf ("  Month:      %d\n", time->Month);
 		printf ("  Day:        %d\n", time->Day);
		printf ("  Hour:       %d\n", time->Hour);
		printf ("  Minute:     %d\n", time->Minute);
		printf ("  Second:     %d\n", time->Second);
		printf ("  Pad1:       %d\n", time->Pad1);
		printf ("  Nanosecond: %d\n", time->Nanosecond);
		printf ("  TimeZone:   %d\n", time->TimeZone);
		printf ("  Daylight:   %d\n", time->Daylight);
		printf ("  Pad2:       %d\n", time->Pad2);
	}
	if (cap) {
		printf ("CAPABILITIES\n");
		printf ("  Resolution: %d\n", cap->Resolution);
		printf ("  Accuracy:   %d\n", cap->Accuracy);
		printf ("  SetsToZero: %s\n", cap->SetsToZero == 1
							? "TRUE" : "FALSE");
	}
}

static void parse_time(char *str, EFI_TIME **time, bool *enable)
{
	char *pch;
	char *saveptr1;

	memset(*time, 0, sizeof(EFI_TIME));
	if (enable) {
		pch = strtok_r(str, ",", &saveptr1);
		if (pch != NULL) {
			if (strstr(pch, "TRUE") || strstr(pch, "true")
				|| strstr(pch, "True"))
			*enable = true;
			pch = strtok_r(NULL, ":", &saveptr1);
		}
	} else 
		pch = strtok_r(str, ":", &saveptr1);

	if (!pch) {
		*time = NULL;
		return;
	}

	if (pch != NULL) {
		(*time)->Year = strtol(pch, NULL, 10);
		pch = strtok_r(NULL, ":", &saveptr1);
	}

	if (pch != NULL) {
		(*time)->Month = strtol(pch, NULL, 10);
		pch = strtok_r(NULL, ":", &saveptr1);
	}

	if (pch != NULL) {
		(*time)->Day = strtol(pch, NULL, 10);
		pch = strtok_r(NULL, ":", &saveptr1);
	}

	if (pch != NULL) {
		(*time)->Hour = strtol(pch, NULL, 10);
		pch = strtok_r(NULL,":", &saveptr1);
	}
	if (pch != NULL) {
		(*time)->Minute = strtol(pch, NULL, 10);
		pch = strtok_r(NULL, ":", &saveptr1);
	}
	if (pch != NULL) {
		(*time)->Second = strtol(pch, NULL, 10);
		pch = strtok_r(NULL, ":", &saveptr1);
	}
	if (pch != NULL) {
		(*time)->Pad1 = strtol(pch, NULL, 10);
		pch = strtok_r(NULL, ":", &saveptr1);
	}
	if (pch != NULL) {
		(*time)->Nanosecond = strtol(pch, NULL, 10);
		pch = strtok_r(NULL, ":", &saveptr1);
	}
	if (pch != NULL) {
		(*time)->TimeZone = strtol(pch, NULL, 10);
		pch = strtok_r(NULL, ":", &saveptr1);
	}
	if (pch != NULL) {
		(*time)->Daylight = strtol(pch, NULL, 10);
		pch = strtok_r(NULL, ":", &saveptr1);
	}
	if (pch != NULL) {
		(*time)->Pad2 = strtol(pch, NULL, 10);
	}

	return;
}

int main(int argc, char **argv)
{
	int c;
	struct efi_gettime gettime;
	struct efi_settime settime;
	struct efi_getwakeuptime getwakeuptime;
	struct efi_setwakeuptime setwakeuptime;
	EFI_TIME efi_time;
	EFI_TIME *p_time = NULL;
	EFI_TIME_CAPABILITIES efi_time_cap;
	bool get = false;
	bool set = false;
	bool getwakeup = false;
	bool setwakeup = false;
	uint64_t status;
	uint8_t enabled, pending;
	bool enable = false;

	for (;;) {
		int idx;
		c = getopt_long(argc, argv, "gs:GS:Vh", options, &idx);
		if (c == -1)
			break;

		switch (c) {
		case 'g':
			get = true;
			break;
		case 'G':
			getwakeup = true;
			break;
		case 's':
			set = true;
			p_time = &efi_time;
			parse_time(optarg, &p_time, NULL);
			break;
		case 'S':
			setwakeup = true;
			p_time = &efi_time;
			parse_time(optarg, &p_time, &enable);
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

	if (!get && !set && !getwakeup && !setwakeup) {
		printf ("Need to specify set, get, getwakup or sewakup time.\n");
		return EXIT_FAILURE;
	}

	if (get && set) {
		printf ("Both set and get time specified.\n");
		return EXIT_FAILURE;
	}

	if (getwakeup && setwakeup) {
		printf ("Both set and get wakeup time specified.\n");
		return EXIT_FAILURE;
	}

	if (get) {
		gettime.Capabilities = &efi_time_cap;
		gettime.Time = &efi_time;
		gettime.status = &status;

		ioctl(fd, EFI_RUNTIME_GET_TIME, &gettime);

		if (status == EFI_SUCCESS)
			print_time_info(gettime.Time, gettime.Capabilities);

		print_status_info(status);
	}

	if (set) {
		settime.Time = &efi_time;
		settime.status = &status;

		ioctl(fd, EFI_RUNTIME_SET_TIME, &settime);

		print_status_info(status);
	}

	if (getwakeup) {
		getwakeuptime.Enabled = &enabled;
		getwakeuptime.Pending = &pending;
		getwakeuptime.Time = &efi_time;
		getwakeuptime.status = &status;

		ioctl(fd, EFI_RUNTIME_GET_WAKETIME, &getwakeuptime);

		if (status == EFI_SUCCESS) {
			printf ("Enable: %s\n", *getwakeuptime.Enabled == 1
							? "TRUE" : "FALSE");
			printf ("Pending: %s\n", *getwakeuptime.Pending == 1
							? "TRUE" : "FALSE");
			print_time_info(getwakeuptime.Time, NULL);
		}
		print_status_info(status);
	}

	if (setwakeup) {
		if (enable && ((void *)p_time == NULL)) {
			printf ("Enable the wakeup alarm, time parameter should not be NULL.\n");
			return EXIT_FAILURE;
		}
		
		setwakeuptime.Enabled = enable;
		setwakeuptime.Time = (EFI_TIME *)p_time;
		setwakeuptime.status = &status;

		ioctl(fd, EFI_RUNTIME_SET_WAKETIME, &setwakeuptime);

		print_status_info(status);
	}
	deinit_driver(fd);

	return EXIT_SUCCESS;

}

