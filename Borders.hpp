#pragma once

#include <vector>

struct Borders {
	std::vector<std::size_t> mountain_r;
	std::vector<std::size_t>	coastline_r;
	std::vector<std::size_t>	ocean_r;

	Borders() {};
	Borders(std::vector<std::size_t> mountain_r,
		std::vector<std::size_t>	coastline_r,
		std::vector<std::size_t>	ocean_r)
	{
		this->mountain_r = mountain_r;
		this->coastline_r = coastline_r;
		this->ocean_r = ocean_r;
	}
};
