#pragma once

#include <memory>
#include <cstdint>
#include <optional>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "common.hpp"
#include "device.hpp"
#include "terminal.hpp"

namespace Emulator {
	class Gpu : public Device {
	private:
		constexpr uint64_t UART_BASE = 0x10000000ULL;
		constexpr uint64_t UART_SIZE = 0x100;

		enum : uint64_t {
			RHR = UART_BASE,
			THR = RHR,
			DLL = RHR,
			IER = UART_BASE + 1,
			DLM = IER,
			ISR = UART_BASE + 2,
			FCR = ISR,
			LCR = UART_BASE + 3,
			MCR = UART_BASE + 4,
			LSR = UART_BASE + 5,
			MSR = UART_BASE + 6,
			SCR = UART_BASE + 7
		};

		enum : uint8_t {
			LSR_DR 		= 0x01,
			LSR_THRE 	= 0x20,
			LSR_TEMT 	= 0x40,
			IER_RDI 	= 0x01,
			IER_THRI 	= 0x02,
			LCR_DLAB 	= 0x80,
			ISR_NO_INT 	= 0x01,
			ISR_THRI 	= 0x02,
			ISR_RDI 	= 0x04
		};
		
		enum : uint64_t {
			FB_RENDER = UART_BASE - 0x700000,
			FB_DIMENSIONS = FB_RENDER + 4,
			TERM_DIMENSIONS = FB_DIMENSIONS + 4,
			FB_CHANNELS = TERM_DIMENSIONS + 4,
			FB_START = FB_CHANNELS + 4
		};

		uint64_t fb_size = 0;

		uint8_t dll = 0, dlm = 0, isr = 0, ier = 0,
				fcr = 0, lcr = 0, mcr = 0, lsr = 0,
				msr = 0, scr = 0, val = 0;

		SDL_Renderer *renderer = nullptr;
		SDL_Window *window = nullptr;
		SDL_Texture *texture = nullptr;
		TTF_Font *font = nullptr;

		uint32_t width, height, channels;
		int32_t term_rows = 32, 
		int32_t term_cols = 120;
	
		uint64_t last_tick = 0;
		uint64_t last_text = ~0ULL;
		
		std::unique_ptr<uint8_t[]> framebuffer;
		std::unique_ptr<Terminal> terminal;
	
		void uart_putchar(uint8_t ch);
		void render_textbuffer(void);
		void resize_screen(uint16_t width, uint16_t height);
		void render_framebuffer(void);
		void dispatch(void);

	public:
		constexpr uint64_t UART_IRQN = 10;
		bool is_uart_interrupting = false;
		
		explicit Gpu(uint32_t _width, uint32_t _height) :
			width(_width), height(_height), 
			channels(3), name("GPU") {};

		inline uint32_t *interrupting(void)
		{
			if (is_uart_interrupting) {
				is_uart_interrupting = false;
				return UART_IRQN;
			}

			return nullptr;
		}
		
		uint64_t load(uint64_t addr) override;
		void store(uint64_t addr, uint64_t value) override;
		void dump(void) const override;
		void tick(void) override;
	};
};
