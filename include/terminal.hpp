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
			data(std::make_unique<T[]>(rows * cols) {};

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

		inline T *operator[](uint32_t row)
		{
			return data.get() + (row * cols);
		}
	};

	class Terminal {
		
	};
};
