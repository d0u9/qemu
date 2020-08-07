#include "qemu/osdep.h"
#include "hw/isa/isa.h"
#include "hw/qdev-properties.h"
#include "chardev/char-fe.h"
#include "qapi/error.h"
#include "sysemu/reset.h"
#include "hw/irq.h"
#if 0
#include "qemu/module.h"
#include "chardev/char-parallel.h"
#include "chardev/char-fe.h"
#include "hw/isa/isa.h"
#include "migration/vmstate.h"
#include "hw/char/parallel.h"
#include "sysemu/reset.h"
#include "sysemu/sysemu.h"
#include "trace.h"
#endif

#define TYPE_ISA_KMOD_EDU	"isa-kmod-edu"
#define ISA_KMOD_EDU(obj) \
	OBJECT_CHECK(ISAKmodEduState, (obj), TYPE_ISA_KMOD_EDU)

typedef struct ISAKmodEduState {
	ISADevice parent_obj;

	uint32_t iobase;
	uint32_t isairq;

	CharBackend chr;
	qemu_irq irq;
	PortioList portio_list;
} ISAKmodEduState;

static void isa_kmod_edu_reset(void *opaque)
{
}

static int isa_kmod_edu_can_receive(void *opaque)
{
	return 1;
}

static uint32_t isa_kmod_edu_ioport_read(void *opaque, uint32_t addr)
{
	static int cursor = 0;
	const int len = 13;
	const char str[] = "Hello World!\n";
	char ret;

	ret = str[cursor++];

	if (cursor % len == 0)
		cursor = 0;

	return (uint32_t)ret;
}

static void
isa_kmod_edu_ioport_write(void *opaque, uint32_t addr, uint32_t val)
{
	ISAKmodEduState *edu = opaque;
	uint8_t byte = (uint8_t)val;
	qemu_chr_fe_write_all(&edu->chr, &byte, 1);

	// send interrupt if user write a '\a' char.
	if (byte == '\n')
		qemu_irq_pulse(edu->irq);
}

static const MemoryRegionPortio isa_kmod_edu_portio_list[] = {
    { 0, 8, 1,
      .read = isa_kmod_edu_ioport_read,
      .write = isa_kmod_edu_ioport_write },
    PORTIO_END_OF_LIST(),
};

static void isa_kmod_edu_realizefn(DeviceState *dev, Error **errp)
{
	static bool registered = false;
	ISADevice *isadev = ISA_DEVICE(dev);
	ISAKmodEduState *edu = ISA_KMOD_EDU(dev);
	int base;

	if (registered) {
		error_setg(errp, "Only one kmod_edu device can be registered");
		return;
	}

	if (!qemu_chr_fe_backend_connected(&edu->chr)) {
		error_setg(errp, "Can't create kmod_edu device, empty char device");
		return;
	}
	
	if (edu->iobase == -1) {
		edu->iobase = 0x200;
	}
	base = edu->iobase;

	isa_init_irq(isadev, &edu->irq, edu->isairq);
	qemu_register_reset(isa_kmod_edu_reset, edu);

	qemu_chr_fe_set_handlers(&edu->chr, isa_kmod_edu_can_receive, NULL,
				 NULL, NULL, edu, NULL, true);
	isa_register_portio_list(isadev, &edu->portio_list, base,
				 isa_kmod_edu_portio_list,
				 edu, TYPE_ISA_KMOD_EDU);
}

static Property isa_kmod_edu_properties[] = {
    DEFINE_PROP_UINT32("iobase", ISAKmodEduState, iobase,  -1),
    DEFINE_PROP_UINT32("irq",   ISAKmodEduState, isairq,  6),
    DEFINE_PROP_CHR("chardev",  ISAKmodEduState, chr),
    DEFINE_PROP_END_OF_LIST(),
};

static void isa_kmod_edu_class_initfn(ObjectClass *klass, void *data)
{
	DeviceClass *dc = DEVICE_CLASS(klass);

	dc->realize = isa_kmod_edu_realizefn;
	device_class_set_props(dc, isa_kmod_edu_properties);
	set_bit(DEVICE_CATEGORY_MISC, dc->categories);
}

static const TypeInfo isa_kmod_edu_info = {
	.name		= TYPE_ISA_KMOD_EDU,
	.parent		= TYPE_ISA_DEVICE,
	.instance_size	= sizeof(ISAKmodEduState),
	.class_init	= isa_kmod_edu_class_initfn,
};

static void isa_kmod_edu_register_types(void)
{
	type_register_static(&isa_kmod_edu_info);
}

type_init(isa_kmod_edu_register_types)
