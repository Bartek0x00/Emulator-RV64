#include "errors.hpp"
#include "gpu.hpp"

using namespace Emulator;

Gpu::Gpu(uint32_t _width, uint32_t _height) :
	width(_width), height(_height), 
	channels(3), name("GPU")
{
	if (SDL_Init(SDL_INIT_VIDEO))
		error<FAIL>(
			name,
			": Cannot init SDL System: ",
			SDL_GetError()
		);

	window = SDL_CreateWindow(
		"I15", 
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height,
		SDL_WINDOW_SHOWN
	);
	if (!window)
		error<FAIL>(
			name,
			": Cannot create SDL window: ",
			SDL_GetError()
		);
	
	renderer = SDL_CreateRenderer(
		window, -1, 
		SDL_RENDERER_ACCELERATED
	);
	if (!renderer)
		error<FAIL>(
			name,
			": Cannot create SDL renderer: ",
			SDL_GetError()
		);

	texture = SDL_CreateTexture(
		renderer, 
		SDL_PIXELFORMAT_RGB24, 
		SDL_TEXTUREACCESS_TARGET, 
		width, height
	);
	if (!texture)
		error<FAIL>(
			name,
			": Cannot create SDL texture: ",
			SDL_GetError()
		);
	
	if (TTF_Init())
		error<FAIL>(
			name,
			": Cannot init TTF subsystem: ",
			SDL_GetError()
		);

	SDL_RWops *font_rw = SDL_RWFromMem(
		font_ttf, font_ttf_len
	);
	if (!font_rw)
		error<FAIL>(
			name,
			": Cannot create SDL_RWops: ",
			SDL_GetError();
		);
	
	font = TTF_OpenFontRW(font_rw, 0, 16);
	SDL_RWclose(font_rw);
	if (!font)
		error<FAIL>(
			name,
			": Cannot load font from RW: ",
			SDL_GetError()
		);
	
	framebuffer = std::make_unique<uint8_t[]>(
		width * height * channels
	);

	fb_size = width * height * channels;
	
	terminal = std::make_unique<Terminal>(
		term_rows, term_cols, font
	);

	lsr = LSR_TEMT | LSR_THRE;
}

Gpu::~Gpu(void)
{
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

uint64_t Gpu::load(uint64_t addr)
{
	if (addr == FB_CHANNELS)
		return channels;
	
	if (addr == FB_DIMENSIONS)
		return (height << 16) | width;
	
	if (addr == TERM_DIMENSIONS)
		return (term_rows << 16) | term_cols;
	
	if (addr >= FB_START && addr <= FB_START + fb_size)
		return framebuffer[addr - FB_START];
	
	if (addr >= UART_BASE && 
		addr <= UART_BASE + UART_SIZE) 
	{
		switch(addr) {
		case THR:
			if (lsr & LSR_DR) {
				lsr &= ~LSR_DR;
				dispatch();
			}

			return val;

		case IER: return ier;
		case ISR: return isr;
		case LCR: return lcr;
		case MCR: return mcr;
		case LSR: return lsr;
		case MSR: return msr;
		case SCR: return scr;
		default: break;
		}
	}

	return 0;
}

void Gpu::store(uint64_t addr, uint64_t value)
{
	switch (addr) {
	case FB_RENDER:
		if (value == 1)
			render_framebuffer();
		break;
	
	case FB_DIMENSIONS:
	{
		uint16_t new_width = value & 0xFFFF;
		uint16_t new_height = (value >> 16) & 0xFFFF;

		resize_screen(new_width, new_height);
		break;
	}

	case TERM_DIMENSIONS:
	{
		uint16_t new_cols = value & 0xFFFF;
		uint16_t new_rows = (value >> 16) & 0xFFFF;

		terminal->reset(new_rows, new_cols, font);

		term_cols = new_cols;
		term_rows = new_rows;
		
		break;
	}

	default:
		if (addr >= FB_START &&
			addr <= FB_START + fb_size)
		{
			framebuffer[addr - FB_START] = value;
			break;
		}
	
		if (addr >= UART_BASE &&
			addr <= UART_BASE + UART_SIZE)
		{
			switch (addr) {
			case THR:
			{
				char c = static_cast<char>(value);
				terminal->input_write(&c, sizeof(c));
				last_text = get_milliseconds();
				
				if (c == '\n') {
					c = '\r';
					terminal->input_write(&c, sizeof(c));
					render_textbuffer();
				}

				break;
			}

			case IER:
				ier = value;
				dispatch();
				break;

			case FCR: 
				fcr = value; 
				break;

			case LCR:
				lcr = value;
				break;

			case MCR:
				mcr = value;
				break;

			case SCR:
				scr = value;
				break;

			default:
				break;
			}
		}
	}
}

void Gpu::uart_putchar(uint8_t ch)
{
	val = ch;
	lsr |= LSR_DR;
	dispatch();
}

void Gpu::dispatch(void)
{
	isr |= 0xc0;

	if ((ier & IER_RDI) && (lsr & LSR_DR)) {
		is_uart_interrupting = true;
		return;
	}
	
	if ((ier & IER_THRI) && (lsr & LSR_TEMT)) {
		is_uart_interrupting = false;
		return;
	}
}

void Gpu::tick(void)
{
	uint64_t current_tick = get_milliseconds();
	
	auto _render = [](void) -> void {
		if ((last_text != ~0ULL) && 
			(get_milliseconds() - last_text > 10))
		{
			render_textbuffer();
		}
	};

	if (current_tick - last_tick <= 10) {
		_render();
		return;
	}

	last_tick = current_tick;
	SDL_Event event;

	if (!SDL_PollEvent(&event)) {
		_render();
		return;
	}

	switch(event.type) {
	case SDL_QUIT:
		error<FAIL>("Exiting...");
		break;
	
	case SDL_KEYDOWN:
	{
		if (SDL_GetModState() & KMOD_CTRL) {
			if (event.key.keysym.sym == SDLK_c)
				uart_putchar(0x3);

			if (event.key.keysym.sym == SDLK_d)
				uart_putchar(0x4);
			
			_render();
			return;
		}

		const uint8_t *state = SDL_GetKeyboardState(NULL);
		SDL_Keycode keycode = event.key.keysym.sym;
		
		bool is_shift = state[SDL_SCANCODE_LSHIFT] || 
			state[SDL_SCANCODE_RSHIFT];
		bool is_caps = SDL_GetModState() & KMOD_CAPS;
		bool is_letter = (keycode >= SDLK_a && keycode <= SDLK_z);
		
		if (!is_letter)
			uart_putchar((char)keycode);
		else if (is_shift || is_caps)
			uart_putchar(
				(char)SDL_toupper((char)keycode)
			);
		else
			uart_putchar(
				(char)SDL_tolower((char)keycode)
			);

		break;
	}

	case SDL_KEYUP:
		val = 0;
		break;
	
	default:
		break;
	}

	_render();
}

void Gpu::render_framebuffer(void)
{
	SDL_UpdateTexture(
		texture, NULL, 
		framebuffer.get(), 
		width * channels
	);

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void Gpu::render_textbuffer(void)
{
	last_text = ~0ULL;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_Rect rect = {.w = width, .h = height};

	terminal->render(renderer, rect);
	SDL_RenderPresent(renderer);
}

void Gpu::resize_screen(uint16_t _width, uint16_t _height)
{
	width = _width;
	height = _height;
	SDL_SetWindowSize(window, width, height);

	fb_size = width * height * channels;
	framebuffer = make_unique<uint8_t[]>(fb_size);

	SDL_DestroyTexture(texture);
	texture = SDL_CreateTexture(
		renderer, SDL_PIXELFORMAT_RGB24,
		SDL_TEXTUREACCESS_TARGET,
		width, height
	);
}
