#include "usbd_core.h"
#include "hpm_usb_device.h"
#include "board.h"
#include "hpm_l1c_drv.h"

#ifndef USBD_IRQHandler
#define USBD_IRQHandler USBD_IRQHandler // use actual usb irq name instead
#endif

#ifndef USB_NUM_BIDIR_ENDPOINTS
#define USB_NUM_BIDIR_ENDPOINTS USB_SOC_DCD_MAX_ENDPOINT_COUNT
#endif

#define USB_ALIGN(size, align) (((size) + (align)-1) & ~((align)-1))

/* USBSTS, USBINTR */
enum {
    intr_usb = HPM_BITSMASK(1, 0),
    intr_error = HPM_BITSMASK(1, 1),
    intr_port_change = HPM_BITSMASK(1, 2),
    intr_reset = HPM_BITSMASK(1, 6),
    intr_sof = HPM_BITSMASK(1, 7),
    intr_suspend = HPM_BITSMASK(1, 8),
    intr_nak = HPM_BITSMASK(1, 16)
};

/* Endpoint state */
struct hpm_ep_state {
    uint16_t ep_mps;    /* Endpoint max packet size */
    uint8_t ep_type;    /* Endpoint type */
    uint8_t ep_stalled; /* Endpoint stall flag */
    uint8_t *xfer_buf;
    uint32_t xfer_len;
    uint32_t actual_xfer_len;
};

/* Driver state */
struct hpm_udc {
    usb_device_handle_t *handle;
    struct hpm_ep_state in_ep[USB_NUM_BIDIR_ENDPOINTS];  /*!< IN endpoint parameters*/
    struct hpm_ep_state out_ep[USB_NUM_BIDIR_ENDPOINTS]; /*!< OUT endpoint parameters */
} g_hpm_udc;

static ATTR_PLACE_AT_NONCACHEABLE_WITH_ALIGNMENT(USB_SOC_DCD_DATA_RAM_ADDRESS_ALIGNMENT) dcd_data_t _dcd_data;
static ATTR_PLACE_AT_NONCACHEABLE usb_device_handle_t usb_device_handle[USB_SOC_MAX_COUNT];

/* Index to bit position in register */
static inline uint8_t ep_idx2bit(uint8_t ep_idx)
{
    return ep_idx / 2 + ((ep_idx % 2) ? 16 : 0);
}

__WEAK void usb_dc_low_level_init(void)
{
}

__WEAK void usb_dc_low_level_deinit(void)
{
}

int usb_dc_init(void)
{
    memset(&g_hpm_udc, 0, sizeof(struct hpm_udc));

    usb_dc_low_level_init();

    g_hpm_udc.handle = &usb_device_handle[0];
    g_hpm_udc.handle->regs = (USB_Type *)HPM_USB0_BASE;
    g_hpm_udc.handle->dcd_data = &_dcd_data;

    uint32_t int_mask;
    int_mask = (USB_USBINTR_UE_MASK | USB_USBINTR_UEE_MASK |
                USB_USBINTR_PCE_MASK | USB_USBINTR_URE_MASK);

    usb_device_init(g_hpm_udc.handle, int_mask);

    intc_m_enable_irq(IRQn_USB0);
    return 0;
}

int usb_dc_deinit(void)
{
    return 0;
}

int usbd_set_address(const uint8_t addr)
{
    usb_device_handle_t *handle = g_hpm_udc.handle;
    usb_dcd_set_address(handle->regs, addr);
    return 0;
}

int usbd_ep_open(const struct usbd_endpoint_cfg *ep_cfg)
{
    usb_endpoint_config_t tmp_ep_cfg;
    usb_device_handle_t *handle = g_hpm_udc.handle;

    uint8_t ep_idx = USB_EP_GET_IDX(ep_cfg->ep_addr);

    if (USB_EP_DIR_IS_OUT(ep_cfg->ep_addr)) {
        g_hpm_udc.out_ep[ep_idx].ep_mps = ep_cfg->ep_mps;
        g_hpm_udc.out_ep[ep_idx].ep_type = ep_cfg->ep_type;
    } else {
        g_hpm_udc.in_ep[ep_idx].ep_mps = ep_cfg->ep_mps;
        g_hpm_udc.in_ep[ep_idx].ep_type = ep_cfg->ep_type;
    }

    tmp_ep_cfg.xfer = ep_cfg->ep_type;
    tmp_ep_cfg.ep_addr = ep_cfg->ep_addr;
    tmp_ep_cfg.max_packet_size = ep_cfg->ep_mps;

    usb_device_edpt_open(handle, &tmp_ep_cfg);
    return 0;
}

int usbd_ep_close(const uint8_t ep)
{
    return 0;
}

int usbd_ep_set_stall(const uint8_t ep)
{
    usb_device_handle_t *handle = g_hpm_udc.handle;

    usb_device_edpt_stall(handle, ep);
    return 0;
}

int usbd_ep_clear_stall(const uint8_t ep)
{
    usb_device_handle_t *handle = g_hpm_udc.handle;

    usb_device_edpt_clear_stall(handle, ep);
    return 0;
}

int usbd_ep_is_stalled(const uint8_t ep, uint8_t *stalled)
{
    return 0;
}

int usbd_ep_start_write(const uint8_t ep, const uint8_t *data, uint32_t data_len)
{
    uint8_t ep_idx = USB_EP_GET_IDX(ep);
    usb_device_handle_t *handle = g_hpm_udc.handle;

    if (!data && data_len) {
        return -1;
    }

    g_hpm_udc.in_ep[ep_idx].xfer_buf = (uint8_t *)data;
    g_hpm_udc.in_ep[ep_idx].xfer_len = data_len;
    g_hpm_udc.in_ep[ep_idx].actual_xfer_len = 0;

#ifdef CONFIG_USB_DCACHE_ENABLE
    if (data_len != 0) {
        uint32_t align_len = USB_ALIGN(data_len, CONFIG_DCACHE_LINE_SIZE);
        l1c_dc_flush((uint32_t)data, align_len);
    }
#endif
    usb_device_edpt_xfer(handle, ep, data, data_len);

    return 0;
}

int usbd_ep_start_read(const uint8_t ep, uint8_t *data, uint32_t data_len)
{
    uint8_t ep_idx = USB_EP_GET_IDX(ep);
    usb_device_handle_t *handle = g_hpm_udc.handle;

    if (!data && data_len) {
        return -1;
    }

    g_hpm_udc.out_ep[ep_idx].xfer_buf = (uint8_t *)data;
    g_hpm_udc.out_ep[ep_idx].xfer_len = data_len;
    g_hpm_udc.out_ep[ep_idx].actual_xfer_len = 0;

    usb_device_edpt_xfer(handle, ep, data, data_len);

    return 0;
}

void USBD_IRQHandler(void)
{
    uint32_t int_status;
    uint32_t speed;
    usb_device_handle_t *handle = g_hpm_udc.handle;
    uint32_t transfer_len;

    /* Acknowledge handled interrupt */
    int_status = usb_device_status_flags(handle);
    int_status &= usb_device_interrupts(handle);
    usb_device_clear_status_flags(handle, int_status);

    /* disabled interrupt sources */
    if (int_status == 0) {
        return;
    }

    if (int_status & intr_reset) {
        speed = usb_device_get_port_speed(handle);
        usbd_event_reset_handler();
        usb_device_bus_reset(handle, 64);
    }

    if (int_status & intr_suspend) {
        if (usb_device_get_suspend_status(handle)) {
            /* Note: Host may delay more than 3 ms before and/or after bus reset
       * before doing enumeration. */
            if (usb_device_get_address(handle)) {
            }
        }
    }

    if (int_status & intr_port_change) {
        if (!usb_device_get_port_ccs(handle)) {
        } else {
            if (usb_device_get_port_reset_status(handle) == 0) {
                uint32_t speed = usb_device_get_port_speed(handle);
            }
        }
    }

    if (int_status & intr_usb) {
        uint32_t const edpt_complete = usb_device_get_edpt_complete_status(handle);
        usb_device_clear_edpt_complete_status(handle, edpt_complete);
        uint32_t edpt_setup_status = usb_device_get_setup_status(handle);

        if (edpt_setup_status) {
            /*------------- Set up Received -------------*/
            usb_device_clear_setup_status(handle, edpt_setup_status);
            dcd_qhd_t *qhd0 = usb_device_qhd_get(handle, 0);
            usbd_event_ep0_setup_complete_handler((uint8_t *)&qhd0->setup_request);
        }

        if (edpt_complete) {
            for (uint8_t ep_idx = 0; ep_idx < USB_SOS_DCD_MAX_QHD_COUNT; ep_idx++) {
                if (edpt_complete & (1 << ep_idx2bit(ep_idx))) {
                    /* Failed QTD also get ENDPTCOMPLETE set */
                    dcd_qtd_t *p_qtd = usb_device_qtd_get(handle, ep_idx);

                    if (p_qtd->halted || p_qtd->xact_err || p_qtd->buffer_err) {
                    } else {
                        /* only number of bytes in the IOC qtd */
                        uint8_t const ep_addr = (ep_idx / 2) | ((ep_idx & 0x01) ? 0x80 : 0);

                        transfer_len = p_qtd->expected_bytes - p_qtd->total_bytes;

                        if (ep_addr & 0x80) {
                            usbd_event_ep_in_complete_handler(ep_addr, transfer_len);
                        } else {
#ifdef CONFIG_USB_DCACHE_ENABLE
                            if (transfer_len) {
                                uint32_t align_len = USB_ALIGN(transfer_len, CONFIG_DCACHE_LINE_SIZE);
                                l1c_dc_invalidate((uint32_t)g_hpm_udc.out_ep[ep_idx].xfer_buf, align_len);
                            }
#endif
                            usbd_event_ep_out_complete_handler(ep_addr, transfer_len);
                        }
                    }
                }
            }
        }
    }
}

ATTR_RAMFUNC void isr_usb0(void)
{
    USBD_IRQHandler();
}
SDK_DECLARE_EXT_ISR_M(IRQn_USB0, isr_usb0)
