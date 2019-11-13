#pragma once

#include <glm/glm.hpp>
#include <vector>

/*
 * From http://www.redblobgames.com/x/1742-webgl-mapgen2/
 * Copyright 2017 Red Blob Games <redblobgames@gmail.com>
 * License: Apache v2.0 <http://www.apache.org/licenses/LICENSE-2.0.html>
 */

/* Generate the biome colormap indexed by elevation -1:+1 and rainfall 0:1 */
int ewidth = 64;
int eheight = 64;

std::vector<glm::vec4> colormap();