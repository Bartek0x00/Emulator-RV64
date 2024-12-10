#pragma once

#include <array>
#include <vector>
#include "device.hpp"
#include "common.hpp"

namespace Emulator {	
	class Virtio : public Device {
	private:
		enum : uint64_t {
			VIRTIO_BASE 	= 0x10001000ULL,
			VIRTIO_SIZE 	= 0x1000ULL,
			VIRTIO_IRQN 	= 0x1,
			MAGIC_VALUE 	= 0x0,
			VERSION 		= 0x4,
			DEVICE_ID 		= 0x8,
			VENDOR_ID		= 0xc,
			DEVICE_FEAT		= 0x10,
			DEVICE_FEAT_SEL	= 0x14,
			DRIVER_FEAT		= 0x20,
			DRIVER_FEAT_SEL = 0x24,
			GUEST_PAGE_SIZE = 0x28,
			QUEUE_SEL		= 0x30,
			QUEUE_NOTIFY_RESET = (~0ULL) >> 32,
			QUEUE_NUM_MAX	= 0x34,
			QUEUE_NUM		= 0x38,
			QUEUE_ALIGN		= 0x3c,
			QUEUE_PFN 		= 0x40,
			QUEUE_NOTIFY 	= 0x50,
			INTERRUPT_STATUS = 0x60,
			INTERRUPT_ACK 	= 0x64,
			STATUS 			= 0x70,
			CONFIG 			= 0x100,
		};

		enum : uint32_t {
			MAGIC 			= 0x74726976,
			VERSION_LEGACY 	= 0x1,
			VENDOR 			= 0x554d4551,
			BLK_DEV 		= 0x2,
			DISK_DELAY 		= 0x1f4,
			SECTOR_SIZE 	= 0x200
		};

		enum : uint16_t {
			VQUEUE_MAX_SIZE = 0x400,
			VQUEUE_ALIGN 	= 0x1000,
			DESC_F_NEXT 	= 0x1,
			DESC_F_WRITE 	= 0x2
		};

		enum : uint8_t {
			BLK_T_IN  		= 0x0,
			BLK_T_OUT 		= 0x1,
			BLK_S_OK  		= BLK_T_IN
		};
	
		struct VRingAvail {
			uint16_t flags;
			uint16_t idx;
			uint16_t ring[];
		};

		struct VirtqDesc {
			uint64_t addr;
			uint32_t len;
			uint16_t flags;
			uint16_t next;
		};

		struct Virtq {
			uint64_t desc;
			uint64_t avail;
			uint64_t used;

			uint32_t num;
			uint32_t align;
		};

		Virtq vq = {0};
		std::vector<uint8_t> rfsimg;
		std::array<uint32_t, 2> host_feat = {0};
		std::array<uint32_t, 2> guest_feat = {0};
		std::array<uint8_t, 8> config = {0};

		uint64_t clock = 0;
		uint64_t notify_clock = 0;

		uint32_t host_feat_sel = 0;
		uint32_t guest_feat_sel = 0;
		uint32_t guest_page_size = 0;
		uint32_t queue_pfn = 0;
		uint32_t queue_notify = 0;

		uint16_t id = 0;
		uint16_t queue_sel = 0;

		uint8_t isr = 0;
		uint8_t status = 0;

	public:
		explicit Virtio(std::vector<uint8_t> data) :
			rfsimg(std::move(data)),
			base(VIRTIO_BASE), 
			size(VIRTIO_SIZE), 
			name("VIRTIO") {};

		uint64_t load(uint64_t addr) override;
		void store(uint64_t addr, uint64_t value) override;
		void dump(void) const override;
		void tick(void) override;

		inline uint32_t *interrupting(void)
		{
			if (isr & 1)
				return VIRTIO_IRQN;

			return nullptr;
		}

		inline void reset(void)
		{
			id = 0;
			isr = 0;
		}
		
		void update(void);
		VirtqDesc load_desc(uint64_t addr);
		void access_disk(void);
	};
};
