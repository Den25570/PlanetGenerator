#include "ColorMap.h"

std::vector<glm::vec4> colormap() {
	std::vector<glm::vec4> pixels = std::vector<glm::vec4>(ewidth * eheight * 4);

	for (int y = 0, p = 0; y < eheight; y++) {
		for (int x = 0; x < ewidth; x++) {
			int e = 2 * x / ewidth - 1,
				m = y / eheight;

			int r, g, b;

			if (x == ewidth / 2 - 1) {
				r = 48;
				g = 120;
				b = 160;
			}
			else
				if (x == ewidth / 2 - 2) {
					r = 48;
					g = 100;
					b = 150;
				}
				else if (x == ewidth / 2 - 3) {
					r = 48;
					g = 80;
					b = 140;
				}
				else
					if (e < 0.0) {
						r = 48 + 48 * e;
						g = 64 + 64 * e;
						b = 127 + 127 * e;
					}
					else { // adapted from terrain-from-noise article
						m = m * (1 - e); // higher elevation holds less moisture; TODO: should be based on slope, not elevation

						r = 210 - 100 * m;
						g = 185 - 45 * m;
						b = 139 - 45 * m;
						r = 255 * e + r * (1 - e);
						g = 255 * e + g * (1 - e);
						b = 255 * e + b * (1 - e);
					}

			pixels[p].x = r / 255.0f;
			pixels[p].y = g / 255.0f;
			pixels[p].z = b / 255.0f;
			pixels[p++].a = 255 / 255.0f;
		}
	}
	return pixels;
}