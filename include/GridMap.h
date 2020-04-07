/*
 * GridMap.h
 *
 *  Created on: Apr 7, 2020
 *      Author: mad
 */

#ifndef INCLUDE_GRIDMAP_H_
#define INCLUDE_GRIDMAP_H_

#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdexcept>


template<typename T>
class GridMap {
public:
	GridMap(int size_, float scale_)
		:	m_size(size_),
			m_scale(scale_)
	{
		T = new T[int64_t(size_) * size_];
	}

	GridMap(const GridMap& other)
		:	GridMap(other.m_size, other.m_scale)
	{
		*this = other;
	}

	~GridMap()
	{
		delete [] m_map;
		m_map = 0;
	}

	GridMap& operator=(const GridMap& other)
	{
		if(m_size != other.m_size) {
			throw std::logic_error("grid size mismatch");
		}
		m_scale = other.m_scale;
		::memcpy(m_map, other.m_map, m_size * m_size);
		return *this;
	}

	int size() const {
		return m_size;
	}

	float scale() const {
		return m_scale;
	}

	int64_t num_cells() const {
		return int64_t(m_size) * m_size;
	}

	T& operator()(int x, int y)
	{
		return m_map[int64_t(y) * m_size + x];
	}

	const T& operator()(int x, int y) const
	{
		return m_map[int64_t(y) * m_size + x];
	}

	float bilinear_lookup(float x, float y) const
	{
		const float a = x - floorf(x);
		const float b = y - floorf(y);

		return bilinear_lookup_ex(x, y, a, b);
	}

	float bilinear_lookup_ex(int x, int y, float a, float b) const
	{
		const int x0 = std::min(std::max(x, 0), m_size - 1);
		const int x1 = std::min(x0 + 1, m_size - 1);
		const int y0 = std::min(std::max(y, 0), m_size - 1);
		const int y1 = std::min(y0 + 1, m_size - 1);

		return		(*this)(x0, y0) * ((1.f - a) * (1.f - b))
				+	(*this)(x1, y0) * (a * (1.f - b))
				+	(*this)(x0, y1) * ((1.f - a) * b)
				+	(*this)(x1, y1) * (a * b);
	}

	void calc_gradient(float x, float y, float& dx, float& dy) const
	{
		static const float coeff_33_dxy[3][3] = {
				{-0.139505, 0, 0.139505},
				{-0.220989, 0, 0.220989},
				{-0.139505, 0, 0.139505}
		};

		const int x0 = x;
		const int y0 = y;

		const float a = x - floorf(x);
		const float b = y - floorf(y);

		dx = 0;
		dy = 0;

		for(int j = -1; j <= 1; ++j) {
			for(int i = -1; i <= 1; ++i) {
				const float value = bilinear_lookup_ex(x0 + i, y0 + j, a, b);
				dx += coeff_33_dxy[j+1][i+1] * value;
				dy += coeff_33_dxy[i+1][j+1] * value;
			}
		}
	}

	void calc_gradient2(float x, float y, float& ddx, float& ddy) const
	{
		static const float coeff_33_ddxy[3][3] = {
				{0.069752, -0.139504, 0.069752},
				{0.110494, -0.220989, 0.110494},
				{0.069752, -0.139504, 0.069752}
		};

		const int x0 = x;
		const int y0 = y;

		const float a = x - floorf(x);
		const float b = y - floorf(y);

		ddx = 0;
		ddy = 0;

		for(int j = -1; j <= 1; ++j) {
			for(int i = -1; i <= 1; ++i) {
				const float value = bilinear_lookup_ex(x0 + i, y0 + j, a, b);
				ddx += coeff_33_ddxy[j+1][i+1] * value;
				ddy += coeff_33_ddxy[i+1][j+1] * value;
			}
		}
	}

	void smooth_33()
	{
		static const float coeff_33_1[3][3] = {
				{0.077847, 0.123317, 0.077847},
				{0.123317, 0.195346, 0.123317},
				{0.077847, 0.123317, 0.077847}
		};

		GridMap tmp(m_size);

		for(int y = 0; y < m_size; ++y) {
			for(int x = 0; x < m_size; ++x) {
				float sum = 0;
				for(int j = -1; j <= 1; ++j) {
					const int y_ = std::min(std::max(y + j, 0), m_size - 1);
					for(int i = -1; i <= 1; ++i) {
						const int x_ = std::min(std::max(x + i, 0), m_size - 1);
						sum += coeff_33_1[j+1][i+1] * (*this)(x_, y_);
					}
				}
				tmp(x, y) = sum;
			}
		}
		*this = tmp;
	}

private:
	int m_size = 0;
	float m_scale = 0;

	T* m_map = 0;

};


#endif /* INCLUDE_GRIDMAP_H_ */