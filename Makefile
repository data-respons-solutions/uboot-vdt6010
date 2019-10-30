#
# Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
#
# (C) Copyright 2011 Freescale Semiconductor, Inc.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
else
obj-y += vdt6010.o
obj-y += ../common/dev/pfuze100.o
endif
