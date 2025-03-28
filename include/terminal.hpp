#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vterm.h>
#include <vector>

#define U_SHOW_CPLUSPLUS_API 1

#include <unicode/normlzr.h>
#include <unicode/unistr.h>

namespace Emulator {
	template<typename T>
	class Matrix {
	private:
		T *data = nullptr;
	
	public:
		int32_t rows = 0;
		int32_t cols = 0;
		
		constexpr inline Matrix() = default;

		inline Matrix(int32_t _rows, int32_t _cols) : 
			rows(_rows), cols(_cols)
		{
			data = new T[cols * rows];
		}

		inline ~Matrix(void)
		{
			if (data)
				delete[] data;
		}

		Matrix<T>& operator=(const Matrix& other)
		{
			if (data)
				delete[] data;

			rows = other.rows;
			cols = other.cols;

			data = new T[cols * rows];

			std::memcpy(data, other.data, rows * cols);
			
			return *this;
		}

		Matrix<T>& operator=(Matrix&& other) noexcept
		{
			if (data)
				delete[] data;

			rows = other.rows;
			cols = other.cols;

			data = other.data;
			other.data = nullptr;

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
			return data[(row * cols) + col];
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

		VTermScreen *screen = nullptr;
		SDL_Surface *surface = nullptr;
		SDL_Texture *texture = nullptr;
		TTF_Font *font = nullptr;

		int32_t font_width = 0;
		int32_t font_height = 0;

		bool is_ringing = false;
	
	public:
		VTerm *vterm = nullptr;

		inline void invalidate_texture(void)
		{
			if (!texture)
				return;

			SDL_DestroyTexture(texture);
			texture = nullptr;
		}

		explicit inline Terminal(int32_t rows, int32_t cols, TTF_Font *font)
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
		static int moverect(VTermRect dest, VTermRect src, void *user);
		static int movecursor(VTermPos pos, VTermPos old_pos, int visible, void *user);
		static int settermprop(VTermProp prop, VTermValue *value, void *user);
		static int bell(void *user);
		static int resize(int rows, int cols, void *user);
		static int sb_pushline(int cols, const VTermScreenCell *cells, void *user);
		static int sb_popline(int cols, VTermScreenCell *cells, void *user);

		void reset(int32_t _rows, int32_t _cols, TTF_Font *_font);
		void render(SDL_Renderer *renderer, const SDL_Rect& window_rect);
		void process_event(const SDL_Event& event);
	};
};
