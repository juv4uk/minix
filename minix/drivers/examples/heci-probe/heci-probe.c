#include <minix/drivers.h>
#include <minix/syslib.h>
#include <stdio.h>

static int sef_cb_init(int type, sef_init_info_t *info)
{
    int r, devind;
    u16_t vid, did;
    u32_t bar0_base, bar0_size;
    int ioflag;

    printf("heci-probe: Starting PCI Discovery...\n");

    pci_init();

    r = pci_first_dev(&devind, &vid, &did);
    
    while (r > 0) {
        if (vid == 0x8086 && did == 0xa13a) {
            printf("heci-probe: FOUND HECI! Vendor: 0x%04x, Device: 0x%04x\n", vid, did);
            
            if (pci_get_bar(devind, 0x10, &bar0_base, &bar0_size, &ioflag) == OK) {
                printf("heci-probe: BAR0 Address: 0x%08x, Size: %d bytes\n", bar0_base, bar0_size);
            }
            
            u8_t irq = pci_attr_r8(devind, 0x3C); /* Interrupt Line */
            printf("heci-probe: IRQ Line: %d\n", irq);
            
            break;
        }
        r = pci_next_dev(&devind, &vid, &did);
    }

    if (r <= 0) {
        printf("heci-probe: HECI device not found on PCI bus.\n");
    }

    return OK;
}

static void sef_local_startup(void)
{
    sef_setcb_init_fresh(sef_cb_init);
    sef_startup();
}

int main(void)
{
    sef_local_startup();
    
    /* We just want to probe once and hang around or exit.
       Since we are not a block/char driver, we just sleep. */
    while (1) {
        sefs_ping(); /* Handle SEF pings to keep RS happy */
    }
    return 0;
}
