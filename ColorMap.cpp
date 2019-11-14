#include "ColorMap.h"

std::vector <unsigned char> colormap(int ewidth, int eheight) {
	std::vector<unsigned char> pixels = std::vector<unsigned char>(ewidth * eheight*4);

	for (int y = 0, p = 0; y < eheight; y++) {
		for (int x = 0; x < ewidth; x++) {
			float e = 2 * x / (float)ewidth - 1;
			float m = y / (float)eheight;

			unsigned char r, g, b;

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

			pixels[p++] = r;
			pixels[p++] = g;
			pixels[p++] = b;
			pixels[p++] = 255;
		}
	}
	return pixels;
}