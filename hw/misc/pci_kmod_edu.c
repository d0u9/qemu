#include "qemu/osdep.h"
#include "qemu/module.h"
#include "hw/pci/pci.h"
#include "hw/pci/pcie.h"
#if 9
#include "qemu/units.h"
#include "hw/hw.h"
#include "hw/pci/msi.h"
#include "qemu/timer.h"
#include "qemu/main-loop.h" /* iothread mutex */
#include "qapi/visitor.h"
#endif

#define TYPE_PCI_KMOD_EDU	"pci-kmod-edu"
#define PCI_KMOD_EDU(obj) \
	OBJECT_CHECK(PCIKmodEduState, (obj), TYPE_PCI_KMOD_EDU)

typedef struct PCIKmodEduState {
	PCIDevice parent_obj;

	MemoryRegion mmio;
} PCIKmodEduState;

static uint64_t kmod_edu_mmio_read(void *opaque, hwaddr addr, unsigned size)
{

	return 0;
}

static void kmod_edu_mmio_write(void *opaque, hwaddr addr, uint64_t val,
			   unsigned size)
{

}

static const MemoryRegionOps kmod_edu_mmio_ops = {
    .read = kmod_edu_mmio_read,
    .write = kmod_edu_mmio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 8,
    },
    .impl = {
        .min_access_size = 4,
        .max_access_size = 8,
    },
};

static void pci_kmod_edu_realize(PCIDevice *pdev, Error **errp)
{
	PCIKmodEduState *edu = PCI_KMOD_EDU(pdev);
	uint8_t *pci_conf = pdev->config;
	pci_config_set_interrupt_pin(pci_conf, 2);

	memory_region_init_io(&edu->mmio, OBJECT(edu), &kmod_edu_mmio_ops,
			      edu, "edu-mmio", 8 *MiB);
	pci_register_bar(pdev, 0, PCI_BASE_ADDRESS_SPACE_MEMORY, &edu->mmio);

	if (pcie_endpoint_cap_init(pdev, 0x0E0) < 0) {
		hw_error("Failed to initialize PCIe capability");
	}
}

static void pci_kmod_edu_uninit(PCIDevice *pdev)
{
    pcie_cap_exit(pdev);
}

static void pci_kmod_edu_class_initfn(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    PCIDeviceClass *k = PCI_DEVICE_CLASS(klass);

    k->realize = pci_kmod_edu_realize;
    k->exit = pci_kmod_edu_uninit;
    k->vendor_id = PCI_VENDOR_ID_QEMU;
    k->device_id = 0x7863;
    k->revision = 0x10;
    k->class_id = PCI_CLASS_OTHERS;
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
}

static InterfaceInfo interfaces[] = {
	// { INTERFACE_CONVENTIONAL_PCI_DEVICE },
	{ INTERFACE_PCIE_DEVICE },
	{ },
};

static const TypeInfo pci_kmod_edu_info = {
	.name		= TYPE_PCI_KMOD_EDU,
	.parent		= TYPE_PCI_DEVICE,
	.instance_size	= sizeof(PCIKmodEduState),
	.class_init	= pci_kmod_edu_class_initfn,
        .interfaces = interfaces,
};

static void pci_kmod_edu_register_types(void)
{
	type_register_static(&pci_kmod_edu_info);
}

type_init(pci_kmod_edu_register_types)

