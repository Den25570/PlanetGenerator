#pragma once

#include <vector>

struct Borders {
	std::vector<std::size_t> mountain_region;
	std::vector<std::size_t> coastline_region;
	std::vector<std::size_t> ocean_region;

	Borders() {};
	Borders(std::vector<std::size_t> mountain_region,
		std::vector<std::size_t> coastline_region,
		std::vector<std::size_t> ocean_region)
	{
		this->mountain_region = mountain_region;
		this->coastline_region = coastline_region;
		this->ocean_region = ocean_region;
	}
};
