#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <vterm.h>
#include <vector>

#define U_SHOW_CPLUSPLUS_API 1

#include <unicode/normlzr.h>
#include <unicode/unistr.h>

namespace Emulator {
	template<typename T>
	class Matrix {
	private:
		std::unique_ptr<T[]> data;
	
	public:
		uint32_t rows;
		uint32_t cols;

		inline Matrix(uint32_t _rows, uint32_t _cols) : 
			rows(_rows), cols(_cols),
			data(std::make_unique<T[]>(rows * cols)) {};

		Matrix<T>& operator=(const Matrix& other)
		{
			if (this == &other) return *this;

			if (rows != other.rows ||
				cols != other.cols)
			{
				data = std::make_unique<T[]>(
					other.rows * other.cols
				);

				rows = other.rows;
				cols = other.cols;
			}

			std::memcpy(
				data.get(),
				other.data.get(),
				rows * cols * sizeof(T)
			);

			return *this;
		}

		Matrix<T>& operator=(Matrix&& other) noexcept
		{
			if (this == &other) return *this;

			data = std::move(other.data);
			rows = other.rows;
			cols = other.cols;

			other.rows = 0;
			other.cols = 0;

			return *this;
		}

		inline void fill(const T& value)
		{
			std::memset(data, value, rows * cols * sizeof(T));
		}

		inline T& operator()(uint32_t row, uint32_t col)
		{
			return data.get()[(row * cols) + col];
		}
	};

	class Terminal {
	private:
		const VTermScreenCallbacks screen_callbacks = {
			.damage = damage, 
			.moverect = moverect, 
			.movecursor = movecursor, 
			.settermprop = settermprop,
			.bell = bell, 
			.resize = resize, 
			.sb_pushline = sb_pushline, 
			.sb_popline = sb_popline
		};

		Matrix<unsigned char> matrix;
		VTermPos cursor_pos;

		VTerm *vterm = nullptr;
		VTermScreen *screen = nullptr;
		SDL_Surface *surface = nullptr;
		SDL_Texture *texture = nullptr;
		TTF_Font *font = nullptr;

		uint32_t font_width = 0;
		uint32_t font_height = 0;

		bool is_ringing = false;
	
	public:
		inline invalidate_texture(void)
		{
			if (!texture)
				return;

			SDL_FreeTexture(texture);
			texture = nullptr;
		}

		explicit inline Terminal(uint32_t rows, uint32_t cols, TTF_Font *font)
		{
			reset(rows, cols, font);
		}

		inline ~Terminal(void)
		{
			vterm_free(vterm);
			invalidate_texture();
			SDL_FreeSurface(surface);
		}

		static int damage(VTermRect rect, void *user);
		static int moverrect(VTermRect dest, VTermRect src, void *user);
		static int movecursor(VTermPos pos, VTermPos old_pos, int visible, void *user);
		static int settermprop(VTermProp prop, VTermValue *value, void *user);
		static int bell(void *user);
		static int resize(int rows, int cols, void *user);
		static int sb_pushline(int cols, const VTermScreenCell *cells, void *user);
		static int sb_popline(int cols, VTermScreenCell *cells, void *user);

		void reset(uint32_t cols, uint32_t rows, TTF_Font *font);
		void render(SDL_Renderer *renderer, const SDL_Rect& window_rect);
		void process_event(const SDL_Event& event);
	};
};
