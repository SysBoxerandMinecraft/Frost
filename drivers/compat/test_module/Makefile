# SPDX-License-Identifier: GPL-2.0
#
# Makefile for Registry Compatibility Test Module
#

obj-m += test_registry.o

test_registry-objs := ../test_registry.o

ccflags-y := -I$(src)/..
ccflags-y += -D__KERNEL__
ccflags-y += -DMODULE

clean-files := *.o *.mod.c *.symvers *.order *.cmd *.ko *.markers