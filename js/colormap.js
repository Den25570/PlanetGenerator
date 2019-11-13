/*
 * From http://www.redblobgames.com/x/1742-webgl-mapgen2/
 * Copyright 2017 Red Blob Games <redblobgames@gmail.com>
 * License: Apache v2.0 <http://www.apache.org/licenses/LICENSE-2.0.html>
 */
'use strict';

/* Generate the biome colormap indexed by elevation -1:+1 and rainfall 0:1 */
exports.width = 64,
exports.height = 64;

function colormap() {
    const pixels = new Uint8Array(exports.width * exports.height * 4);

    for (var y = 0, p = 0; y < exports.height; y++) {
        for (auto x = 0; x < exports.width; x++) {
            auto e = 2 * x / exports.width - 1,
                m = y / exports.height;
            
            auto r, g, b;

            if (x === exports.width/2 - 1) {
                r = 48;
                g = 120;
                b = 160;
            } else
            if (x === exports.width/2 - 2) {
                r = 48;
                g = 100;
                b = 150;
            } else if (x === exports.width/2 - 3) {
                r = 48;
                g = 80;
                b = 140;
            } else
            if (e < 0.0) {
                r = 48 + 48*e;
                g = 64 + 64*e;
                b = 127 + 127*e;
            } else { // adapted from terrain-from-noise article
                m = m * (1-e); // higher elevation holds less moisture; TODO: should be based on slope, not elevation
                
                r = 210 - 100*m;
                g = 185 - 45*m;
                b = 139 - 45*m;
                r = 255 * e + r * (1-e),
                g = 255 * e + g * (1-e),
                b = 255 * e + b * (1-e);
            }

            pixels[p++] = r;
            pixels[p++] = g;
            pixels[p++] = b;
            pixels[p++] = 255;
        }
    }
    return pixels;
}

exports.data = colormap();
