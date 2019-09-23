function generatePlanetAsynchronous()
{
	var planet;
	
	var subdivisions = generationSettings.subdivisions;

	var distortionRate;
	if (generationSettings.distortionLevel < 0.25) distortionRate = adjustRange(generationSettings.distortionLevel, 0.00, 0.25, 0.000, 0.040);
	else if (generationSettings.distortionLevel < 0.50) distortionRate = adjustRange(generationSettings.distortionLevel, 0.25, 0.50, 0.040, 0.050);
	else if (generationSettings.distortionLevel < 0.75) distortionRate = adjustRange(generationSettings.distortionLevel, 0.50, 0.75, 0.050, 0.075);
	else distortionRate = adjustRange(generationSettings.distortionLevel, 0.75, 1.00, 0.075, 0.150);

	var originalSeed = generationSettings.seed;
	var seed;
	if (typeof(originalSeed) === "number") seed = originalSeed;
	else if (typeof(originalSeed) === "string") seed = hashString(originalSeed);
	else seed = Date.now();
	var random = new XorShift128(seed);
	
	var plateCount = generationSettings.plateCount;
	var oceanicRate = generationSettings.oceanicRate;
	var heatLevel = generationSettings.heatLevel;
	var moistureLevel = generationSettings.moistureLevel;
	
	activeAction = new SteppedAction(updateProgressUI)
		.executeSubaction(function(action) { ui.progressPanel.show(); }, 0)
		.executeSubaction(function(action) { generatePlanet(subdivisions, distortionRate, plateCount, oceanicRate, heatLevel, moistureLevel, random, action); }, 1, "Generating Planet")
		.getResult(function(result) { planet = result; planet.seed = seed; planet.originalSeed = originalSeed; })
		.executeSubaction(function(action) { displayPlanet(planet); setSeed(null); }, 0)
		.finalize(function(action) { activeAction = null; ui.progressPanel.hide(); }, 0)
		.execute();
}

function generatePlanet(icosahedronSubdivision, topologyDistortionRate, plateCount, oceanicRate, heatLevel, moistureLevel, random, action)
{
	var planet = new Planet();
	var mesh;
	action
		.executeSubaction(function(action) { generatePlanetMesh(icosahedronSubdivision, topologyDistortionRate, random, action); }, 6, "Generating Mesh")
			.getResult(function(result) { mesh = result; })
		.executeSubaction(function(action) { generatePlanetTopology(mesh, action); }, 1, "Generating Topology")
			.getResult(function(result) { planet.topology = result; })
		.executeSubaction(function(action) { generatePlanetPartition(planet.topology.tiles, action); }, 1, "Generating Spatial Partitions")
			.getResult(function(result) { planet.partition = result; })
		.executeSubaction(function(action) { generatePlanetTerrain(planet, plateCount, oceanicRate, heatLevel, moistureLevel, random, action); }, 8, "Generating Terrain")
		.executeSubaction(function(action) { generatePlanetRenderData(planet.topology, random, action); }, 1, "Building Visuals")
			.getResult(function(result) { planet.renderData = result; })
		.executeSubaction(function(action) { generatePlanetStatistics(planet.topology, planet.plates, action); }, 1, "Compiling Statistics")
			.getResult(function(result) { planet.statistics = result; })
		.provideResult(planet);
}

function generatePlanetMesh(icosahedronSubdivision, topologyDistortionRate, random, action)
{
	var mesh;
	action.executeSubaction(function(action)
	{
		mesh = generateSubdividedIcosahedron(icosahedronSubdivision);
	}, 1, "Generating Subdivided Icosahedron");
	
	action.executeSubaction(function(action)
	{
		var totalDistortion = Math.ceil(mesh.edges.length * topologyDistortionRate);
		var remainingIterations = 6;
		action.executeSubaction(function(action)
		{
			var iterationDistortion = Math.floor(totalDistortion / remainingIterations);
			totalDistortion -= iterationDistortion;
			action.executeSubaction(function(action) { distortMesh(mesh, iterationDistortion, random, action); });
			action.executeSubaction(function(action) { relaxMesh(mesh, 0.5, action); });
			--remainingIterations;
			if (remainingIterations > 0) action.loop(1 - remainingIterations / 6);
		});
	}, 15, "Distorting Triangle Mesh");
	
	action.executeSubaction(function(action)
	{
		var initialIntervalIteration = action.intervalIteration;
	
		var averageNodeRadius = Math.sqrt(4 * Math.PI / mesh.nodes.length);
		var minShiftDelta = averageNodeRadius / 50000 * mesh.nodes.length;
		var maxShiftDelta = averageNodeRadius / 50 * mesh.nodes.length;

		var priorShift;
		var currentShift = relaxMesh(mesh, 0.5, action);
		action.executeSubaction(function(action)
		{
			priorShift = currentShift;
			currentShift = relaxMesh(mesh, 0.5, action);
			var shiftDelta = Math.abs(currentShift - priorShift);
			if (shiftDelta >= minShiftDelta && action.intervalIteration - initialIntervalIteration < 300)
			{
				action.loop(Math.pow(Math.max(0, (maxShiftDelta - shiftDelta) / (maxShiftDelta - minShiftDelta)), 4));
			}
		});
	}, 25, "Relaxing Triangle Mesh");
	
	action.executeSubaction(function(action)
	{
		for (var i = 0; i < mesh.faces.length; ++i)
		{
			var face = mesh.faces[i];
			var p0 = mesh.nodes[face.n[0]].p;
			var p1 = mesh.nodes[face.n[1]].p;
			var p2 = mesh.nodes[face.n[2]].p;
			face.centroid = calculateFaceCentroid(p0, p1, p2).normalize();
		}
	}, 1, "Calculating Triangle Centroids");

	action.executeSubaction(function(action)
	{
		for (var i = 0; i < mesh.nodes.length; ++i)
		{
			var node = mesh.nodes[i];
			var faceIndex = node.f[0];
			for (var j = 1; j < node.f.length - 1; ++j)
			{
				faceIndex = findNextFaceIndex(mesh, i, faceIndex);
				var k = node.f.indexOf(faceIndex);
				node.f[k] = node.f[j];
				node.f[j] = faceIndex;
			}
		}
	}, 1, "Reordering Triangle Nodes");
	
	action.provideResult(function() { return mesh; });
}

function generatePlanetPartition(tiles, action)
{
	var icosahedron = generateIcosahedron();
	action.executeSubaction(function(action)
	{
		for (var i = 0; i < icosahedron.faces.length; ++i)
		{
			var face = icosahedron.faces[i];
			var p0 = icosahedron.nodes[face.n[0]].p.clone().multiplyScalar(1000);
			var p1 = icosahedron.nodes[face.n[1]].p.clone().multiplyScalar(1000);
			var p2 = icosahedron.nodes[face.n[2]].p.clone().multiplyScalar(1000);
			var center = p0.clone().add(p1).add(p2).divideScalar(3);
			var radius = Math.max(center.distanceTo(p0), center.distanceTo(p2), center.distanceTo(p2));
			face.boundingSphere = new THREE.Sphere(center, radius);
			face.children = [];
		}
	});
	
	var unparentedTiles = [];
	var maxDistanceFromOrigin = 0;
	action.executeSubaction(function(action)
	{
		for (var i = 0; i < tiles.length; ++i)
		{
			var tile = tiles[i];
			maxDistanceFromOrigin = Math.max(maxDistanceFromOrigin, tile.boundingSphere.center.length() + tile.boundingSphere.radius);
			
			var parentFound = false;
			for (var j = 0; j < icosahedron.faces.length; ++j)
			{
				var face = icosahedron.faces[j];
				var distance = tile.boundingSphere.center.distanceTo(face.boundingSphere.center) + tile.boundingSphere.radius;
				if (distance < face.boundingSphere.radius)
				{
					face.children.push(tile);
					parentFound = true;
					break;
				}
			}
			if (!parentFound)
			{
				unparentedTiles.push(tile);
			}
		}
	});
	
	var rootPartition;
	action.executeSubaction(function(action)
	{
		rootPartition = new SpatialPartition(new THREE.Sphere(new Vector3(0, 0, 0), maxDistanceFromOrigin), [], unparentedTiles);
		for (var i = 0; i < icosahedron.faces.length; ++i)
		{
			var face = icosahedron.faces[i];
			rootPartition.partitions.push(new SpatialPartition(face.boundingSphere, [], face.children));
			delete face.boundingSphere;
			delete face.children;
		}
	});
	
	action.provideResult(function() { return rootPartition; });
}

function generatePlanetTerrain(planet, plateCount, oceanicRate, heatLevel, moistureLevel, random, action)
{
	action
		.executeSubaction(function(action) { generatePlanetTectonicPlates(planet.topology, plateCount, oceanicRate, random, action); }, 3, "Generating Tectonic Plates")
			.getResult(function(result) { planet.plates = result; })
		.executeSubaction(function(action) { generatePlanetElevation(planet.topology, planet.plates, action); }, 4, "Generating Elevation")
		.executeSubaction(function(action) { generatePlanetWeather(planet.topology, planet.partition, heatLevel, moistureLevel, random, action); }, 16, "Generating Weather")
		.executeSubaction(function(action) { generatePlanetBiomes(planet.topology.tiles, 1000, action); }, 1, "Generating Biomes");
}

function generatePlanetTectonicPlates(topology, plateCount, oceanicRate, random, action)
{
	var plates = [];
	var platelessTiles = [];
	var platelessTilePlates = [];
	action.executeSubaction(function(action)
	{
		var failedCount = 0;
		while (plates.length < plateCount && failedCount < 10000)
		{
			var corner = topology.corners[random.integerExclusive(0, topology.corners.length)];
			var adjacentToExistingPlate = false;
			for (var i = 0; i < corner.tiles.length; ++i)
			{
				if (corner.tiles[i].plate)
				{
					adjacentToExistingPlate = true;
					failedCount += 1;
					break;
				}
			}
			if (adjacentToExistingPlate) continue;
			
			failedCount = 0;
			
			var oceanic = (random.unit() < oceanicRate);
			var plate = new Plate(
				new THREE.Color(random.integer(0, 0xFFFFFF)),
				randomUnitVector(random),
				random.realInclusive(-Math.PI / 30, Math.PI / 30),
				random.realInclusive(-Math.PI / 30, Math.PI / 30),
				oceanic ? random.realInclusive(-0.8, -0.3) : random.realInclusive(0.1, 0.5),
				oceanic,
				corner);
				
			plates.push(plate);

			for (var i = 0; i < corner.tiles.length; ++i)
			{
				corner.tiles[i].plate = plate;
				plate.tiles.push(corner.tiles[i]);
			}

			for (var i = 0; i < corner.tiles.length; ++i)
			{
				var tile = corner.tiles[i];
				for (var j = 0; j < tile.tiles.length; ++j)
				{
					var adjacentTile = tile.tiles[j];
					if (!adjacentTile.plate)
					{
						platelessTiles.push(adjacentTile);
						platelessTilePlates.push(plate);
					}
				}
			}
		}
	});
	
	action.executeSubaction(function(action)
	{
		while (platelessTiles.length > 0)
		{
			var tileIndex = Math.floor(Math.pow(random.unit(), 2) * platelessTiles.length);
			var tile = platelessTiles[tileIndex];
			var plate = platelessTilePlates[tileIndex];
			platelessTiles.splice(tileIndex, 1);
			platelessTilePlates.splice(tileIndex, 1);
			if (!tile.plate)
			{
				tile.plate = plate;
				plate.tiles.push(tile);
				for (var j = 0; j < tile.tiles.length; ++j)
				{
					if (!tile.tiles[j].plate)
					{
						platelessTiles.push(tile.tiles[j]);
						platelessTilePlates.push(plate);
					}
				}
			}
		}
	});
	
	action.executeSubaction(calculateCornerDistancesToPlateRoot.bind(null, plates));
	
	action.provideResult(plates);
}

function generatePlanetPartition(tiles, action)
{
	var icosahedron = generateIcosahedron();
	action.executeSubaction(function(action)
	{
		for (var i = 0; i < icosahedron.faces.length; ++i)
		{
			var face = icosahedron.faces[i];
			var p0 = icosahedron.nodes[face.n[0]].p.clone().multiplyScalar(1000);
			var p1 = icosahedron.nodes[face.n[1]].p.clone().multiplyScalar(1000);
			var p2 = icosahedron.nodes[face.n[2]].p.clone().multiplyScalar(1000);
			var center = p0.clone().add(p1).add(p2).divideScalar(3);
			var radius = Math.max(center.distanceTo(p0), center.distanceTo(p2), center.distanceTo(p2));
			face.boundingSphere = new THREE.Sphere(center, radius);
			face.children = [];
		}
	});
	
	var unparentedTiles = [];
	var maxDistanceFromOrigin = 0;
	action.executeSubaction(function(action)
	{
		for (var i = 0; i < tiles.length; ++i)
		{
			var tile = tiles[i];
			maxDistanceFromOrigin = Math.max(maxDistanceFromOrigin, tile.boundingSphere.center.length() + tile.boundingSphere.radius);
			
			var parentFound = false;
			for (var j = 0; j < icosahedron.faces.length; ++j)
			{
				var face = icosahedron.faces[j];
				var distance = tile.boundingSphere.center.distanceTo(face.boundingSphere.center) + tile.boundingSphere.radius;
				if (distance < face.boundingSphere.radius)
				{
					face.children.push(tile);
					parentFound = true;
					break;
				}
			}
			if (!parentFound)
			{
				unparentedTiles.push(tile);
			}
		}
	});
	
	var rootPartition;
	action.executeSubaction(function(action)
	{
		rootPartition = new SpatialPartition(new THREE.Sphere(new Vector3(0, 0, 0), maxDistanceFromOrigin), [], unparentedTiles);
		for (var i = 0; i < icosahedron.faces.length; ++i)
		{
			var face = icosahedron.faces[i];
			rootPartition.partitions.push(new SpatialPartition(face.boundingSphere, [], face.children));
			delete face.boundingSphere;
			delete face.children;
		}
	});
	
	action.provideResult(function() { return rootPartition; });
}

function generatePlanetTerrain(planet, plateCount, oceanicRate, heatLevel, moistureLevel, random, action)
{
	action
		.executeSubaction(function(action) { generatePlanetTectonicPlates(planet.topology, plateCount, oceanicRate, random, action); }, 3, "Generating Tectonic Plates")
			.getResult(function(result) { planet.plates = result; })
		.executeSubaction(function(action) { generatePlanetElevation(planet.topology, planet.plates, action); }, 4, "Generating Elevation")
		.executeSubaction(function(action) { generatePlanetWeather(planet.topology, planet.partition, heatLevel, moistureLevel, random, action); }, 16, "Generating Weather")
		.executeSubaction(function(action) { generatePlanetBiomes(planet.topology.tiles, 1000, action); }, 1, "Generating Biomes");
}

function generatePlanetTectonicPlates(topology, plateCount, oceanicRate, random, action)
{
	var plates = [];
	var platelessTiles = [];
	var platelessTilePlates = [];
	action.executeSubaction(function(action)
	{
		var failedCount = 0;
		while (plates.length < plateCount && failedCount < 10000)
		{
			var corner = topology.corners[random.integerExclusive(0, topology.corners.length)];
			var adjacentToExistingPlate = false;
			for (var i = 0; i < corner.tiles.length; ++i)
			{
				if (corner.tiles[i].plate)
				{
					adjacentToExistingPlate = true;
					failedCount += 1;
					break;
				}
			}
			if (adjacentToExistingPlate) continue;
			
			failedCount = 0;
			
			var oceanic = (random.unit() < oceanicRate);
			var plate = new Plate(
				new THREE.Color(random.integer(0, 0xFFFFFF)),
				randomUnitVector(random),
				random.realInclusive(-Math.PI / 30, Math.PI / 30),
				random.realInclusive(-Math.PI / 30, Math.PI / 30),
				oceanic ? random.realInclusive(-0.8, -0.3) : random.realInclusive(0.1, 0.5),
				oceanic,
				corner);
				
			plates.push(plate);

			for (var i = 0; i < corner.tiles.length; ++i)
			{
				corner.tiles[i].plate = plate;
				plate.tiles.push(corner.tiles[i]);
			}

			for (var i = 0; i < corner.tiles.length; ++i)
			{
				var tile = corner.tiles[i];
				for (var j = 0; j < tile.tiles.length; ++j)
				{
					var adjacentTile = tile.tiles[j];
					if (!adjacentTile.plate)
					{
						platelessTiles.push(adjacentTile);
						platelessTilePlates.push(plate);
					}
				}
			}
		}
	});
	
	action.executeSubaction(function(action)
	{
		while (platelessTiles.length > 0)
		{
			var tileIndex = Math.floor(Math.pow(random.unit(), 2) * platelessTiles.length);
			var tile = platelessTiles[tileIndex];
			var plate = platelessTilePlates[tileIndex];
			platelessTiles.splice(tileIndex, 1);
			platelessTilePlates.splice(tileIndex, 1);
			if (!tile.plate)
			{
				tile.plate = plate;
				plate.tiles.push(tile);
				for (var j = 0; j < tile.tiles.length; ++j)
				{
					if (!tile.tiles[j].plate)
					{
						platelessTiles.push(tile.tiles[j]);
						platelessTilePlates.push(plate);
					}
				}
			}
		}
	});
	
	action.executeSubaction(calculateCornerDistancesToPlateRoot.bind(null, plates));
	
	action.provideResult(plates);
}

function generatePlanetElevation(topology, plates, action)
{
	var boundaryCorners;
	var boundaryCornerInnerBorderIndexes;
	var elevationBorderQueue;
	var elevationBorderQueueSorter = function(left, right) { return left.distanceToPlateBoundary - right.distanceToPlateBoundary; };

	action
		.executeSubaction(function(action) { identifyBoundaryBorders(topology.borders, action); }, 1)
		.executeSubaction(function(action) { collectBoundaryCorners(topology.corners, action); }, 1)
			.getResult(function(result) { boundaryCorners = result; })
		.executeSubaction(function(action) { calculatePlateBoundaryStress(boundaryCorners, action); }, 2)
			.getResult(function(result) { boundaryCornerInnerBorderIndexes = result; })
		.executeSubaction(function(action) { blurPlateBoundaryStress(boundaryCorners, 3, 0.4, action); }, 2)
		.executeSubaction(function(action) { populateElevationBorderQueue(boundaryCorners, boundaryCornerInnerBorderIndexes, action); }, 2)
			.getResult(function(result) { elevationBorderQueue = result; })
		.executeSubaction(function(action) { processElevationBorderQueue(elevationBorderQueue, elevationBorderQueueSorter, action); }, 10)
		.executeSubaction(function(action) { calculateTileAverageElevations(topology.tiles, action); }, 2);
}

function generatePlanetWeather(topology, partitions, heatLevel, moistureLevel, random, action)
{
	var planetRadius = 1000;
	var whorls;
	var activeCorners;
	var totalHeat;
	var remainingHeat;
	var totalMoisture;
	var remainingMoisture;
	
	action
		.executeSubaction(function(action) { generateAirCurrentWhorls(planetRadius, random, action); }, 1, "Generating Air Currents")
			.getResult(function(result) { whorls = result; })
		.executeSubaction(function(action) { calculateAirCurrents(topology.corners, whorls, planetRadius, action); }, 1, "Generating Air Currents")
		.executeSubaction(function(action) { initializeAirHeat(topology.corners, heatLevel, action); }, 2, "Calculating Temperature")
			.getResult(function(result) { activeCorners = result.corners; totalHeat = result.airHeat; remainingHeat = result.airHeat; })
		.executeSubaction(function(action)
			{
				var consumedHeat = processAirHeat(activeCorners, action);
				remainingHeat -= consumedHeat;
				if (remainingHeat > 0 && consumedHeat >= 0.0001) action.loop(1 - remainingHeat / totalHeat);
			}, 8, "Calculating Temperature")
		.executeSubaction(function(action) { calculateTemperature(topology.corners, topology.tiles, planetRadius, action); }, 1, "Calculating Temperature")
		.executeSubaction(function(action) { initializeAirMoisture(topology.corners, moistureLevel, action); }, 2, "Calculating Moisture")
			.getResult(function(result) { activeCorners = result.corners; totalMoisture = result.airMoisture; remainingMoisture = result.airMoisture; })
		.executeSubaction(function(action)
			{
				var consumedMoisture = processAirMoisture(activeCorners, action);
				remainingMoisture -= consumedMoisture;
				if (remainingMoisture > 0 && consumedMoisture >= 0.0001) action.loop(1 - remainingMoisture / totalMoisture);
			}, 32, "Calculating Moisture")
		.executeSubaction(function(action) { calculateMoisture(topology.corners, topology.tiles, action); }, 1, "Calculating Moisture");
}

function generatePlanetBiomes(tiles, planetRadius, action)
{
	for (var i = 0; i < tiles.length; ++i)
	{
		var tile = tiles[i];
		var elevation = Math.max(0, tile.elevation);
		var latitude = Math.abs(tile.position.y / planetRadius);
		var temperature = tile.temperature;
		var moisture = tile.moisture;
		
		if (elevation <= 0)
		{
			if (temperature > 0)
			{
				tile.biome = "ocean";
			}
			else
			{
				tile.biome = "oceanGlacier";
			}
		}
		else if (elevation < 0.6)
		{
			if (temperature > 0.75)
			{
				if (moisture < 0.25)
				{
					tile.biome = "desert";
				}
				else
				{
					tile.biome = "rainForest";
				}
			}
			else if (temperature > 0.5)
			{
				if (moisture < 0.25)
				{
					tile.biome = "rocky";
				}
				else if (moisture < 0.50)
				{
					tile.biome = "plains";
				}
				else
				{
					tile.biome = "swamp";
				}
			}
			else if (temperature > 0)
			{
				if (moisture < 0.25)
				{
					tile.biome = "plains";
				}
				else if (moisture < 0.50)
				{
					tile.biome = "grassland";
				}
				else
				{
					tile.biome = "deciduousForest";
				}
			}
			else
			{
				if (moisture < 0.25)
				{
					tile.biome = "tundra";
				}
				else
				{
					tile.biome = "landGlacier";
				}
			}
		}
		else if (elevation < 0.8)
		{
			if (temperature > 0)
			{
				if (moisture < 0.25)
				{
					tile.biome = "tundra";
				}
				else
				{
					tile.biome = "coniferForest";
				}
			}
			else
			{
				tile.biome = "tundra";
			}
		}
		else
		{
			if (temperature > 0 || moisture < 0.25)
			{
				tile.biome = "mountain";
			}
			else
			{
				tile.biome = "snowyMountain";
			}
		}
	}
}

function generatePlanetTopology(mesh, action)
{
	var corners = new Array(mesh.faces.length);
	var borders = new Array(mesh.edges.length);
	var tiles = new Array(mesh.nodes.length);
	
	action.executeSubaction(function(action)
	{
		for (var i = 0; i < mesh.faces.length; ++i)
		{
			var face = mesh.faces[i];
			corners[i] = new Corner(i, face.centroid.clone().multiplyScalar(1000), face.e.length, face.e.length, face.n.length);
		}
	});

	action.executeSubaction(function(action)
	{
		for (var i = 0; i < mesh.edges.length; ++i)
		{
			var edge = mesh.edges[i];
			borders[i] = new Border(i, 2, 4, 2); //edge.f.length, mesh.faces[edge.f[0]].e.length + mesh.faces[edge.f[1]].e.length - 2, edge.n.length
		}
	});

	action.executeSubaction(function(action)
	{
		for (var i = 0; i < mesh.nodes.length; ++i)
		{
			var node = mesh.nodes[i];
			tiles[i] = new Tile(i, node.p.clone().multiplyScalar(1000), node.f.length, node.e.length, node.e.length);
		}
	});
	
	action.executeSubaction(function(action)
	{
		for (var i = 0; i < corners.length; ++i)
		{
			var corner = corners[i];
			var face = mesh.faces[i];
			for (var j = 0; j < face.e.length; ++j)
			{
				corner.borders[j] = borders[face.e[j]];
			}
			for (var j = 0; j < face.n.length; ++j)
			{
				 corner.tiles[j] = tiles[face.n[j]];
			}
		}
	});
	
	action.executeSubaction(function(action)
	{
		for (var i = 0; i < borders.length; ++i)
		{
			var border = borders[i];
			var edge = mesh.edges[i];
			var averageCorner = new Vector3(0, 0, 0);
			var n = 0;
			for (var j = 0; j < edge.f.length; ++j)
			{
				var corner = corners[edge.f[j]]
				averageCorner.add(corner.position);
				border.corners[j] = corner;
				for (var k = 0; k < corner.borders.length; ++k)
				{
					if (corner.borders[k] !== border) border.borders[n++] = corner.borders[k];
				}
			}
			border.midpoint = averageCorner.multiplyScalar(1 / border.corners.length);
			for (var j = 0; j < edge.n.length; ++j)
			{
				border.tiles[j] = tiles[edge.n[j]];
			}
		}
	});
	
	action.executeSubaction(function(action)
	{
		for (var i = 0; i < corners.length; ++i)
		{
			var corner = corners[i];
			for (var j = 0; j < corner.borders.length; ++j)
			{
				corner.corners[j] = corner.borders[j].oppositeCorner(corner);
			}
		}
	});
	
	action.executeSubaction(function(action)
	{
		for (var i = 0; i < tiles.length; ++i)
		{
			var tile = tiles[i];
			var node = mesh.nodes[i];
			for (var j = 0; j < node.f.length; ++j)
			{
				tile.corners[j] = corners[node.f[j]];
			}
			for (var j = 0; j < node.e.length; ++j)
			{
				var border = borders[node.e[j]];
				if (border.tiles[0] === tile)
				{
					for (var k = 0; k < tile.corners.length; ++k)
					{
						var corner0 = tile.corners[k];
						var corner1 = tile.corners[(k + 1) % tile.corners.length];
						if (border.corners[1] === corner0 && border.corners[0] === corner1)
						{
							border.corners[0] = corner0;
							border.corners[1] = corner1;
						}
						else if (border.corners[0] !== corner0 || border.corners[1] !== corner1)
						{
							continue;
						}
						tile.borders[k] = border;
						tile.tiles[k] = border.oppositeTile(tile);
						break;
					}
				}
				else
				{
					for (var k = 0; k < tile.corners.length; ++k)
					{
						var corner0 = tile.corners[k];
						var corner1 = tile.corners[(k + 1) % tile.corners.length];
						if (border.corners[0] === corner0 && border.corners[1] === corner1)
						{
							border.corners[1] = corner0;
							border.corners[0] = corner1;
						}
						else if (border.corners[1] !== corner0 || border.corners[0] !== corner1)
						{
							continue;
						}
						tile.borders[k] = border;
						tile.tiles[k] = border.oppositeTile(tile);
						break;
					}
				}
			}

			tile.averagePosition = new Vector3(0, 0, 0);
			for (var j = 0; j < tile.corners.length; ++j)
			{
				tile.averagePosition.add(tile.corners[j].position);
			}
			tile.averagePosition.multiplyScalar(1 / tile.corners.length);
			
			var maxDistanceToCorner = 0;
			for (var j = 0; j < tile.corners.length; ++j)
			{
				maxDistanceToCorner = Math.max(maxDistanceToCorner, tile.corners[j].position.distanceTo(tile.averagePosition));
			}
			
			var area = 0;
			for (var j = 0; j < tile.borders.length; ++j)
			{
				area += calculateTriangleArea(tile.position, tile.borders[j].corners[0].position, tile.borders[j].corners[1].position);
			}
			tile.area = area;
			
			tile.normal = tile.position.clone().normalize();
			
			tile.boundingSphere = new THREE.Sphere(tile.averagePosition, maxDistanceToCorner);
		}
	});
	
	action.executeSubaction(function(action)
	{
		for (var i = 0; i < corners.length; ++i)
		{
			var corner = corners[i];
			corner.area = 0;
			for (var j = 0; j < corner.tiles.length; ++j)
			{
				corner.area += corner.tiles[j].area / corner.tiles[j].corners.length;
			}
		}
	});
	
	action.provideResult({ corners: corners, borders: borders, tiles: tiles });
}