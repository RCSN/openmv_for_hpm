# Copyright 2021 hpmicro
# SPDX-License-Identifier: BSD-3-Clause

sdk_inc(common)
sdk_inc(core)
sdk_inc(class/cdc)
sdk_inc(class/hid)
sdk_inc(class/msc)

sdk_src(core/usbd_core.c)
sdk_src(class/cdc/usbd_cdc.c)
sdk_src(class/hid/usbd_hid.c)
sdk_src(class/msc/usbd_msc.c)

sdk_src(demo/cdc_acm_msc_template.c)
sdk_src(port/hpm/usb_dc_hpm.c)
