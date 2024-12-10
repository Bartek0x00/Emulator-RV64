#include "virtio.hpp"

using namespace Emulator;

Virtio::Virtio(std::vector<uint8_t> data)
{
	queue_notify = QUEUE_NOTIFY_RESET;

	vq.align = VQUEUE_ALIGN;

	config[1] = 0x20;
	config[2] = 0x03;

	host_feat[1] = 1 << 3;

	reset();
}

void Virtio::update(void)
{
	vq.desc = queue_pfn * guest_page_size;
	vq.avail = vq.desc + (vq.num * sizeof(VirtqDesc));
	vq.used = align_up(
		vq.avail + \
		offsetof(VRingAvail, ring) + \
		vq.num * sizeof(VRingAvail::ring[0]),
		vq.align
	);
}

VirtqDesc Virtio::load_desc(uint64_t addr)
{
	return {	
		.addr = bus.load(addr);
		.len = bus.load(addr + 8);
		.flags = bus.load(addr + 12);
		.next = bus.load(addr + 14);
	};
}

void Virtio::access_disk(void)
{
	uint16_t idx = bus.load(
		vq.avail + offsetof(VRingAvail, idx)
	);
	
	uint16_t desc_off = bus.load(
		vq.avail + 4 + ((idx & vq.num) * 2)
	);

	VirtqDesc desc0 = load_desc(vq.desc + (sizeof(VirtqDesc) * desc_off);
	VirtqDesc desc1 = load_desc(vq.desc + (sizeof(VirtqDesc) * desc0.next);
	VirtqDesc desc2 = load_desc(vq.desc + (sizeof(VirtqDesc) * desc1.next);

	uint32_t blk_req_type = bus.load(desc0.addr);
	uint64_t blk_req_sector = bus.load(desc0.addr + 8);

	if (blk_req_type == BLK_T_OUT)
		for (uint32_t i = 0; i < desc1.len; i++)
			rfsimg[blk_req_sector * SECTOR_SIZE + i] = 
				bus.load(desc1.addr + 1);
	else
		for (uint32_t i = 0; i < desc1.len; i++)
			bus.store(
				desc1.addr + i,
				rfsimg[blk_req_sector * SECTOR_SIZE + i]
			);
	
	bus.store(desc2.addr, BLK_S_OK);
	bus.store(vq.used + 4 + ((id % vq.num) * 8), desc_off);
	bus.store(vq.used + 2, ++id);
}

uint64_t Virtio::load(uint64_t addr)
{
	addr -= base;

	if (addr >= CONFIG)
		return config[addr - CONFIG];

	switch (addr) {
	case MAGIC_VALUE: return MAGIC;
	case VERSION: return VERSION_LEGACY;
	case DEVICE_ID: return BLK_DEV;
	case VENDOR_ID: return VENDOR;
	case DEVICE_FEAT: return host_feat[host_feat_sel];
	case QUEUE_NUM_MAX: return VQUEUE_MAX_SIZE;
	case QUEUE_PFN: return queue_pfn;
	case INTERRUPT_STATUS: return isr;
	case STATUS: return status;
	default: return 0;
	};
}

void Virtio::store(uint64_t addr. uint64_t value)
{
	addr -= base;

	if (addr >= CONFIG) {
		uint64_t idx = addr - CONFIG;
		config[idx] = (value >> (idx * 8)) & 0xFF;
		return;
	}

	switch (addr) {
	case DEVICE_FEAT_SEL:
		host_feat_sel = value;
		break;
	case DRIVER_FEAT:
		guest_feat[guest_feat_sel] = value;
		break;
	case DRIVER_FEAT_SEL:
		guest_feat_sel = value;
		break;
	case GUEST_PAGE_SIZE:
		guest_page_size = value;
		break;
	case QUEUE_NUM:
		vq.num = value;
		break;
	case QUEUE_ALIGN:
		vq.align = value;
		break;
	case QUEUE_PFN:
		queue_pfn = value;
		update();
		break;
	case QUEUE_NOTIFY:
		queue_notify = value;
		notify_clock = clock;
		break;
	case INTERRUPT_ACK:
		isr = ~value;
		break;
	case STATUS:
		status = value & 0xFF;

		if (status == 0)
			reset();
		else if (status & 0x4)
			update();
		break;
	default:
		break;
	}
}

void Virtio::dump(void) const
{
	error<INFO>(
		"################################\n"
		"#  Device: VIRTIO              #\n"
		"################################"
		"\n# base: ", base,
		"\n# size: ", size,
		"\n# status: ", status,
		"\n################################\n"
	);
}

void Virtio::tick(void)
{
	if (queue_notify != QUEUE_NOTIFY_RESET &&
		clock == notify_clock + DISK_DELAY)
	{
		isr |= 0x1;

		access_disk();

		queue_notify = QUEUE_NOTIFY_RESET;
	}

	++clock;
}
