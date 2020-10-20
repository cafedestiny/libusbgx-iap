/*
 * Copyright (C) 2014 Samsung Electronics
 *
 * Krzysztof Opasiak <k.opasiak@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <errno.h>
#include <stdio.h>
#include <linux/usb/ch9.h>
#include <usbg/usbg.h>

#define VENDOR          0x0525
#define PRODUCT         0x0104

int main(void)
{
	usbg_state *s;
	usbg_gadget *g;
	usbg_config *c;
	usbg_function *f_ffs, *f_acm;
	int ret = -EINVAL;
	int usbg_ret;

	struct usbg_gadget_attrs g_attrs = {
		.bcdUSB = 0x0200,
		.bDeviceClass =	0x00,
		.bDeviceSubClass = 0x00,
		.bDeviceProtocol = 0x00,
		.bMaxPacketSize0 = 64, /* Max allowed ep0 packet size */
		.idVendor = VENDOR,
		.idProduct = PRODUCT,
		.bcdDevice = 0x0001, /* Version of device */
	};

	struct usbg_gadget_strs g_strs = {
		.serial = "0123456789", /* Serial number */
		.manufacturer = "Linux Foundation", /* Manufacturer */
		.product = "Infotainment Unit" /* Product string */
	};

	struct usbg_config_strs c_strs = {
		.configuration = "iAP2 / NCM Interface"
	};

	usbg_ret = usbg_init("/sys/kernel/config", &s);
	if (usbg_ret != USBG_SUCCESS) {
		fprintf(stderr, "Error on USB gadget init\n");
		fprintf(stderr, "Error: %s : %s\n", usbg_error_name(usbg_ret),
				usbg_strerror(usbg_ret));
		goto out1;
	}

	usbg_ret = usbg_create_gadget(s, "g1", &g_attrs, &g_strs, &g);
	if (usbg_ret != USBG_SUCCESS) {
		fprintf(stderr, "Error on create gadget\n");
		fprintf(stderr, "Error: %s : %s\n", usbg_error_name(usbg_ret),
				usbg_strerror(usbg_ret));
		goto out2;
	}

	usbg_ret = usbg_create_function(g, USBG_F_FFS, "usb0", NULL, &f_ffs);
	if (usbg_ret != USBG_SUCCESS) {
		fprintf(stderr, "Error creating ffs function\n");
		fprintf(stderr, "Error: %s : %s\n", usbg_error_name(usbg_ret),
				usbg_strerror(usbg_ret));
		goto out2;
	}

	usbg_ret = usbg_create_function(g, USBG_F_NCM, "usb0", NULL, &f_acm);
	if (usbg_ret != USBG_SUCCESS) {
		fprintf(stderr, "Error creating ncm function\n");
		fprintf(stderr, "Error: %s : %s\n", usbg_error_name(usbg_ret),
				usbg_strerror(usbg_ret));
		goto out2;
	}

	/* NULL can be passed to use kernel defaults */
	usbg_ret = usbg_create_config(g, 1, NULL, NULL, &c_strs, &c);
	if (usbg_ret != USBG_SUCCESS) {
		fprintf(stderr, "Error creating config\n");
		fprintf(stderr, "Error: %s : %s\n", usbg_error_name(usbg_ret),
				usbg_strerror(usbg_ret));
		goto out2;
	}

	usbg_ret = usbg_add_config_function(c, "ffs.usb0", f_ffs);
	if (usbg_ret != USBG_SUCCESS) {
		fprintf(stderr, "Error adding ffs\n");
		fprintf(stderr, "Error: %s : %s\n", usbg_error_name(usbg_ret),
				usbg_strerror(usbg_ret));
		goto out2;
	}

	usbg_ret = usbg_add_config_function(c, "ncm.usb0", f_acm);
	if (usbg_ret != USBG_SUCCESS) {
		fprintf(stderr, "Error adding ncm \n");
		fprintf(stderr, "Error: %s : %s\n", usbg_error_name(usbg_ret),
				usbg_strerror(usbg_ret));
		goto out2;
	}

	fprintf(stdout, "FFS gadget has been created.\n"
		"Enable it after preparing your functions.\n");

	/*
	 * Here we end up with two created ffs instances but they are not
	 * fully operational. Now we have to do step by step:
	 * 1) Mount both instances:
	 *    $ mount my_dev_name -t functionfs /path/to/mount/dir1
	 *    $ mount my_awesome_dev_name -t functionfs /path/to/mount/dir2
	 *
	 * 2) Run ffs daemons for both instances:
	 *    $ my-ffs-daemon /path/to/mount/dir1
	 *    $ my-ffs-daemon /path/to/mount/dir2
	 *
	 * 3) Enable your gadget:
	 *    $ echo "my_udc_name" > /sys/kernel/config/usb_gadget/g1/UDC
	 */

	ret = 0;

out2:
	usbg_cleanup(s);

out1:
	return ret;
}
