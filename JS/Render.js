function render()
{
	var currentRenderFrameTime = Date.now();
	var frameDuration = lastRenderFrameTime !== null ? Math.min((currentRenderFrameTime - lastRenderFrameTime) * 0.001, 0.1) : 0;
	
	var cameraNeedsUpdated = false;
	if (zoomAnimationStartTime !== null)
	{
		if (zoomAnimationStartTime + zoomAnimationDuration <= currentRenderFrameTime)
		{
			zoom = zoomAnimationEndValue;
			zoomAnimationStartTime = null;
			zoomAnimationDuration = null;
			zoomAnimationStartValue = null;
			zoomAnimationEndValue = null;
		}
		else
		{
			zoomAnimationProgress = (currentRenderFrameTime - zoomAnimationStartTime) / zoomAnimationDuration;
			zoom = (zoomAnimationEndValue - zoomAnimationStartValue) * zoomAnimationProgress + zoomAnimationStartValue;
		}
		cameraNeedsUpdated = true;
	}

	var cameraZoomDelta = getZoomDelta();
	if (frameDuration > 0 && cameraZoomDelta !== 0)
	{
		zoom = Math.max(0, Math.min(zoom + frameDuration * cameraZoomDelta * 0.5, 1));
		cameraNeedsUpdated = true;
	}
	
	var cameraLatitudeDelta = getLatitudeDelta();
	if (frameDuration > 0 && cameraLatitudeDelta !== 0)
	{
		cameraLatitude += frameDuration * -cameraLatitudeDelta * Math.PI * (zoom * 0.5 + (1 - zoom) * 1 / 20);
		cameraLatitude = Math.max(-Math.PI * 0.49, Math.min(cameraLatitude, Math.PI * 0.49));
		cameraNeedsUpdated = true;
	}
	
	var cameraLongitudeDelta = getLongitudeDelta();
	if (frameDuration > 0 && cameraLongitudeDelta !== 0)
	{
		cameraLongitude += frameDuration * cameraLongitudeDelta * Math.PI * (zoom * Math.PI / 8 + (1 - zoom) / (20 * Math.max(Math.cos(cameraLatitude), 0.1)));
		cameraLongitude = cameraLongitude - Math.floor(cameraLongitude / (Math.PI * 2)) * Math.PI * 2;
		cameraNeedsUpdated = true;
	}

	if (cameraNeedsUpdated) updateCamera();
	
	var sunTime = Math.PI * 2 * currentRenderFrameTime / 60000 + sunTimeOffset;
	directionalLight.position.set(Math.cos(sunTime), 0, Math.sin(sunTime)).normalize();

	requestAnimationFrame(render);
	if (projectionRenderMode === "globe")
	{
		renderer.render(scene, camera);
	}
	else
	{
		renderer.render(scene, mapCamera);
	}
	
	lastRenderFrameTime = currentRenderFrameTime;
}

function renderMap()
{
	var project = mapProjections[projectionRenderMode];
	if (project)
	{
		var surfaceColorArrayKeys = [ "terrainColors", "plateColors", "elevationColors", "temperatureColors", "moistureColors" ];
		projectMap(planet.renderData.surface, surfaceColorArrayKeys, project);
		projectMap(planet.renderData.plateBoundaries, null, project);
		projectMap(planet.renderData.plateMovements, null, project);
		projectMap(planet.renderData.airCurrents, null, project);

		scene.add(planet.renderData.surface.mapRenderObject);
		setSurfaceRenderMode(surfaceRenderMode, true);
		showHideSunlight(renderSunlight);
		showHidePlateBoundaries(renderPlateBoundaries);
		showHidePlateMovements(renderPlateMovements);
		showHideAirCurrents(renderAirCurrents);
		
		if (tileSelection !== null)
		{
			tileSelection.mapMaterial = tileSelection.material.clone();
			projectMap(tileSelection, null, project);
			planet.renderData.surface.mapRenderObject.add(tileSelection.mapRenderObject);
		}
	}
}

function setSurfaceRenderMode(mode, force)
{
	if (mode !== surfaceRenderMode || force === true)
	{
		$("#surfaceDisplayList>button").removeClass("toggled");
		ui.surfaceDisplayButtons[mode].addClass("toggled");

		surfaceRenderMode = mode;
		
		if (!planet) return;

		var colors;
		var geometry;
		if (projectionRenderMode === "globe")
		{
			geometry = planet.renderData.surface.geometry;
			if (mode === "terrain") colors = planet.renderData.surface.terrainColors;
			else if (mode === "plates") colors = planet.renderData.surface.plateColors;
			else if (mode === "elevation") colors = planet.renderData.surface.elevationColors;
			else if (mode === "temperature") colors = planet.renderData.surface.temperatureColors;
			else if (mode === "moisture") colors = planet.renderData.surface.moistureColors;
			else return;
		}
		else
		{
			geometry = planet.renderData.surface.mapGeometry;
			if (mode === "terrain") colors = planet.renderData.surface.mapColorArrays.terrainColors;
			else if (mode === "plates") colors = planet.renderData.surface.mapColorArrays.plateColors;
			else if (mode === "elevation") colors = planet.renderData.surface.mapColorArrays.elevationColors;
			else if (mode === "temperature") colors = planet.renderData.surface.mapColorArrays.temperatureColors;
			else if (mode === "moisture") colors = planet.renderData.surface.mapColorArrays.moistureColors;
			else return;
		}

		var faces = geometry.faces;
		for (var i = 0; i < faces.length; ++i) faces[i].vertexColors = colors[i];
		
		geometry.colorsNeedUpdate = true;
	}
}

function generatePlanetRenderData(topology, random, action)
{
	var renderData = {};
	
	action
		.executeSubaction(function(action) { buildSurfaceRenderObject(topology.tiles, random, action); }, 8, "Building Surface Visuals")
			.getResult(function(result) { renderData.surface = result; })
		.executeSubaction(function(action) { buildPlateBoundariesRenderObject(topology.borders, action); }, 1, "Building Plate Boundary Visuals")
			.getResult(function(result) { renderData.plateBoundaries = result; })
		.executeSubaction(function(action) { buildPlateMovementsRenderObject(topology.tiles, action); }, 2, "Building Plate Movement Visuals")
			.getResult(function(result) { renderData.plateMovements = result; })
		.executeSubaction(function(action) { buildAirCurrentsRenderObject(topology.corners, action); }, 2, "Building Air Current Visuals")
			.getResult(function(result) { renderData.airCurrents = result; });
		
	action.provideResult(renderData);
}

function buildSurfaceRenderObject(tiles, random, action)
{
	var planetGeometry = new THREE.Geometry();
	var terrainColors = [];
	var plateColors = [];
	var elevationColors = [];
	var temperatureColors = [];
	var moistureColors = [];
	
	var i = 0;
	action.executeSubaction(function(action)
	{
		if (i >= tiles.length) return;
		
		var tile = tiles[i];
		
		var colorDeviance = new THREE.Color(random.unit(), random.unit(), random.unit());
		var terrainColor;
		if (tile.elevation <= 0)
		{
			var normalizedElevation = Math.min(-tile.elevation, 1);
			if (tile.biome === "ocean") terrainColor = new THREE.Color(0x0066FF).lerp(new THREE.Color(0x0044BB), Math.min(-tile.elevation, 1)).lerp(colorDeviance, 0.10);
			else if (tile.biome === "oceanGlacier") terrainColor = new THREE.Color(0xDDEEFF).lerp(colorDeviance, 0.10);
			else terrainColor = new THREE.Color(0xFF00FF);
		}
		else if (tile.elevation < 0.6)
		{
			var normalizedElevation = tile.elevation / 0.6;
			if (tile.biome === "desert") terrainColor = new THREE.Color(0xDDDD77).lerp(new THREE.Color(0xBBBB55), normalizedElevation).lerp(colorDeviance, 0.10);
			else if (tile.biome === "rainForest") terrainColor = new THREE.Color(0x44DD00).lerp(new THREE.Color(0x229900), normalizedElevation).lerp(colorDeviance, 0.20);
			else if (tile.biome === "rocky") terrainColor = new THREE.Color(0xAA9977).lerp(new THREE.Color(0x887755), normalizedElevation).lerp(colorDeviance, 0.15);
			else if (tile.biome === "plains") terrainColor = new THREE.Color(0x99BB44).lerp(new THREE.Color(0x667722), normalizedElevation).lerp(colorDeviance, 0.10);
			else if (tile.biome === "grassland") terrainColor = new THREE.Color(0x77CC44).lerp(new THREE.Color(0x448822), normalizedElevation).lerp(colorDeviance, 0.15);
			else if (tile.biome === "swamp") terrainColor = new THREE.Color(0x77AA44).lerp(new THREE.Color(0x446622), normalizedElevation).lerp(colorDeviance, 0.25);
			else if (tile.biome === "deciduousForest") terrainColor = new THREE.Color(0x33AA22).lerp(new THREE.Color(0x116600), normalizedElevation).lerp(colorDeviance, 0.10);
			else if (tile.biome === "tundra") terrainColor = new THREE.Color(0x9999AA).lerp(new THREE.Color(0x777788), normalizedElevation).lerp(colorDeviance, 0.15);
			else if (tile.biome === "landGlacier") terrainColor = new THREE.Color(0xDDEEFF).lerp(colorDeviance, 0.10);
			else terrainColor = new THREE.Color(0xFF00FF);
		}
		else if (tile.elevation < 0.8)
		{
			var normalizedElevation = (tile.elevation - 0.6) / 0.2;
			if (tile.biome === "tundra") terrainColor = new THREE.Color(0x777788).lerp(new THREE.Color(0x666677), normalizedElevation).lerp(colorDeviance, 0.10);
			else if (tile.biome === "coniferForest") terrainColor = new THREE.Color(0x338822).lerp(new THREE.Color(0x116600), normalizedElevation).lerp(colorDeviance, 0.10);
			else if (tile.biome === "snow") terrainColor = new THREE.Color(0xEEEEEE).lerp(new THREE.Color(0xDDDDDD), normalizedElevation).lerp(colorDeviance, 0.10);
			else if (tile.biome === "mountain") terrainColor = new THREE.Color(0x555544).lerp(new THREE.Color(0x444433), normalizedElevation).lerp(colorDeviance, 0.05);
			else terrainColor = new THREE.Color(0xFF00FF);
		}
		else
		{
			var normalizedElevation = Math.min((tile.elevation - 0.8) / 0.5, 1);
			if (tile.biome === "mountain") terrainColor = new THREE.Color(0x444433).lerp(new THREE.Color(0x333322), normalizedElevation).lerp(colorDeviance, 0.05);
			else if (tile.biome === "snowyMountain") terrainColor = new THREE.Color(0xDDDDDD).lerp(new THREE.Color(0xFFFFFF), normalizedElevation).lerp(colorDeviance, 0.10);
			else terrainColor = new THREE.Color(0xFF00FF);
		}
		
		var plateColor = tile.plate.color.clone();

		var elevationColor;
		if (tile.elevation <= 0) elevationColor = new THREE.Color(0x224488).lerp(new THREE.Color(0xAADDFF), Math.max(0, Math.min((tile.elevation + 3/4) / (3/4), 1)));
		else if (tile.elevation < 0.75) elevationColor = new THREE.Color(0x997755).lerp(new THREE.Color(0x553311), Math.max(0, Math.min((tile.elevation) / (3/4), 1)));
		else elevationColor = new THREE.Color(0x553311).lerp(new THREE.Color(0x222222), Math.max(0, Math.min((tile.elevation - 3/4) / (1/2), 1)));

		var temperatureColor;
		if (tile.temperature <= 0) temperatureColor = new THREE.Color(0x0000FF).lerp(new THREE.Color(0xBBDDFF), Math.max(0, Math.min((tile.temperature + 2/3) / (2/3), 1)));
		else temperatureColor = new THREE.Color(0xFFFF00).lerp(new THREE.Color(0xFF0000), Math.max(0, Math.min((tile.temperature) / (3/3), 1)));

		var moistureColor = new THREE.Color(0xFFCC00).lerp(new THREE.Color(0x0066FF), Math.max(0, Math.min(tile.moisture, 1)));
		
		var baseIndex = planetGeometry.vertices.length;
		planetGeometry.vertices.push(tile.averagePosition);
		for (var j = 0; j < tile.corners.length; ++j)
		{
			var cornerPosition = tile.corners[j].position;
			planetGeometry.vertices.push(cornerPosition);
			planetGeometry.vertices.push(tile.averagePosition.clone().sub(cornerPosition).multiplyScalar(0.1).add(cornerPosition));

			var i0 = j * 2;
			var i1 = ((j + 1) % tile.corners.length) * 2;
			buildTileWedge(planetGeometry.faces, baseIndex, i0, i1, tile.normal);
			buildTileWedgeColors(terrainColors, terrainColor, terrainColor.clone().multiplyScalar(0.5));
			buildTileWedgeColors(plateColors, plateColor, plateColor.clone().multiplyScalar(0.5));
			buildTileWedgeColors(elevationColors, elevationColor, elevationColor.clone().multiplyScalar(0.5));
			buildTileWedgeColors(temperatureColors, temperatureColor, temperatureColor.clone().multiplyScalar(0.5));
			buildTileWedgeColors(moistureColors, moistureColor, moistureColor.clone().multiplyScalar(0.5));
			for (var k = planetGeometry.faces.length - 3; k < planetGeometry.faces.length; ++k) planetGeometry.faces[k].vertexColors = terrainColors[k];
		}
		
		++i;
		
		action.loop(i / tiles.length);
	});
	
	planetGeometry.dynamic = true;
	planetGeometry.boundingSphere = new THREE.Sphere(new Vector3(0, 0, 0), 1000);
	var planetMaterial = new THREE.MeshLambertMaterial({ color: new THREE.Color(0x000000), ambient: new THREE.Color(0xFFFFFF), vertexColors: THREE.VertexColors, });
	var planetRenderObject = new THREE.Mesh(planetGeometry, planetMaterial);
	
	var mapGeometry = new THREE.Geometry();
	mapGeometry.dynamic = true;
	var mapMaterial = new THREE.MeshBasicMaterial({ vertexColors: THREE.VertexColors, });
	var mapRenderObject = new THREE.Mesh(mapGeometry, mapMaterial);
	
	action.provideResult({
		geometry: planetGeometry,
		terrainColors: terrainColors,
		plateColors: plateColors,
		elevationColors: elevationColors,
		temperatureColors: temperatureColors,
		moistureColors: moistureColors,
		material: planetMaterial,
		renderObject: planetRenderObject,
		mapGeometry: mapGeometry,
		mapMaterial: mapMaterial,
		mapRenderObject: mapRenderObject,
	});
}

function buildPlateBoundariesRenderObject(borders, action)
{
	var geometry = new THREE.Geometry();

	var i = 0;
	action.executeSubaction(function(action)
	{
		if (i >= borders.length) return;
		
		var border = borders[i];
		if (border.betweenPlates)
		{
			var normal = border.midpoint.clone().normalize();
			var offset = normal.clone().multiplyScalar(1);

			var borderPoint0 = border.corners[0].position;
			var borderPoint1 = border.corners[1].position;
			var tilePoint0 = border.tiles[0].averagePosition;
			var tilePoint1 = border.tiles[1].averagePosition;
			
			var baseIndex = geometry.vertices.length;
			geometry.vertices.push(borderPoint0.clone().add(offset));
			geometry.vertices.push(borderPoint1.clone().add(offset));
			geometry.vertices.push(tilePoint0.clone().sub(borderPoint0).multiplyScalar(0.2).add(borderPoint0).add(offset));
			geometry.vertices.push(tilePoint0.clone().sub(borderPoint1).multiplyScalar(0.2).add(borderPoint1).add(offset));
			geometry.vertices.push(tilePoint1.clone().sub(borderPoint0).multiplyScalar(0.2).add(borderPoint0).add(offset));
			geometry.vertices.push(tilePoint1.clone().sub(borderPoint1).multiplyScalar(0.2).add(borderPoint1).add(offset));
			
			var pressure = Math.max(-1, Math.min((border.corners[0].pressure + border.corners[1].pressure) / 2, 1));
			var shear = Math.max(0, Math.min((border.corners[0].shear + border.corners[1].shear) / 2, 1));
			var innerColor = (pressure <= 0) ? new THREE.Color(1 + pressure, 1, 0) : new THREE.Color(1, 1 - pressure, 0);
			var outerColor = new THREE.Color(0, shear / 2, shear);
			
			geometry.faces.push(new THREE.Face3(baseIndex + 0, baseIndex + 1, baseIndex + 2, normal, [ innerColor, innerColor, outerColor ] ));
			geometry.faces.push(new THREE.Face3(baseIndex + 1, baseIndex + 3, baseIndex + 2, normal, [ innerColor, outerColor, outerColor ] ));
			geometry.faces.push(new THREE.Face3(baseIndex + 1, baseIndex + 0, baseIndex + 5, normal, [ innerColor, innerColor, outerColor ] ));
			geometry.faces.push(new THREE.Face3(baseIndex + 0, baseIndex + 4, baseIndex + 5, normal, [ innerColor, outerColor, outerColor ] ));
		}
		
		++i;
		
		action.loop(i / borders.length);
	});
	
	geometry.boundingSphere = new THREE.Sphere(new Vector3(0, 0, 0), 1010);
	var material = new THREE.MeshBasicMaterial({ vertexColors: THREE.VertexColors, });
	var renderObject = new THREE.Mesh(geometry, material);
	
	var mapGeometry = new THREE.Geometry();
	mapGeometry.dynamic = true;
	var mapMaterial = new THREE.MeshBasicMaterial({ vertexColors: THREE.VertexColors, });
	var mapRenderObject = new THREE.Mesh(mapGeometry, mapMaterial);
	
	action.provideResult({
		geometry: geometry,
		material: material,
		renderObject: renderObject,
		mapGeometry: mapGeometry,
		mapMaterial: mapMaterial,
		mapRenderObject: mapRenderObject,
	});
}

function buildPlateMovementsRenderObject(tiles, action)
{
	var geometry = new THREE.Geometry();

	var i = 0;
	action.executeSubaction(function(action)
	{
		if (i >= tiles.length) return;
		
		var tile = tiles[i];
		var plate = tile.plate;
		var movement = plate.calculateMovement(tile.position);
		var plateMovementColor = new THREE.Color(1 - plate.color.r, 1 - plate.color.g, 1 - plate.color.b);

		buildArrow(geometry, tile.position.clone().multiplyScalar(1.002), movement.clone().multiplyScalar(0.5), tile.position.clone().normalize(), Math.min(movement.length(), 4), plateMovementColor);
		
		tile.plateMovement = movement;

		++i;
		
		action.loop(i / tiles.length);
	});

	geometry.boundingSphere = new THREE.Sphere(new Vector3(0, 0, 0), 1010);
	var material = new THREE.MeshBasicMaterial({ vertexColors: THREE.VertexColors, });
	var renderObject = new THREE.Mesh(geometry, material);
	
	var mapGeometry = new THREE.Geometry();
	mapGeometry.dynamic = true;
	var mapMaterial = new THREE.MeshBasicMaterial({ vertexColors: THREE.VertexColors, });
	var mapRenderObject = new THREE.Mesh(mapGeometry, mapMaterial);
	
	action.provideResult({
		geometry: geometry,
		material: material,
		renderObject: renderObject,
		mapGeometry: mapGeometry,
		mapMaterial: mapMaterial,
		mapRenderObject: mapRenderObject,
	});
}

function buildAirCurrentsRenderObject(corners, action)
{
	var geometry = new THREE.Geometry();

	var i = 0;
	action.executeSubaction(function(action)
	{
		if (i >= corners.length) return;
		
		var corner = corners[i];
		buildArrow(geometry, corner.position.clone().multiplyScalar(1.002), corner.airCurrent.clone().multiplyScalar(0.5), corner.position.clone().normalize(), Math.min(corner.airCurrent.length(), 4));

		++i;
		
		action.loop(i / corners.length);
	});

	geometry.boundingSphere = new THREE.Sphere(new Vector3(0, 0, 0), 1010);
	var material = new THREE.MeshBasicMaterial({ color: new THREE.Color(0xFFFFFF), });
	var renderObject = new THREE.Mesh(geometry, material);
	
	var mapGeometry = new THREE.Geometry();
	mapGeometry.dynamic = true;
	var mapMaterial = new THREE.MeshBasicMaterial({ color: new THREE.Color(0xFFFFFF), });
	var mapRenderObject = new THREE.Mesh(mapGeometry, mapMaterial);
	
	action.provideResult({
		geometry: geometry,
		material: material,
		renderObject: renderObject,
		mapGeometry: mapGeometry,
		mapMaterial: mapMaterial,
		mapRenderObject: mapRenderObject,
	});
}

function buildArrow(geometry, position, direction, normal, baseWidth, color)
{
	if (direction.lengthSq() === 0) return;
	var sideOffset = direction.clone().cross(normal).setLength(baseWidth / 2);
	var baseIndex = geometry.vertices.length;
	geometry.vertices.push(position.clone().add(sideOffset), position.clone().add(direction), position.clone().sub(sideOffset));
	geometry.faces.push(new THREE.Face3(baseIndex, baseIndex + 2, baseIndex + 1, normal, [ color, color, color ]));
}

function buildTileWedge(f, b, s, t, n)
{
	f.push(new THREE.Face3(b + s + 2, b + t + 2, b, n));
	f.push(new THREE.Face3(b + s + 1, b + t + 1, b + t + 2, n));
	f.push(new THREE.Face3(b + s + 1, b + t + 2, b + s + 2, n));
}

function buildTileWedgeColors(f, c, bc)
{
	f.push([ c, c, c ]);
	f.push([ bc, bc, c ]);
	f.push([ bc, c, c ]);
}


// MAP PROJECTIONS

var mapProjections =
{
	equalAreaMap: function equalAreaMap(longitude, latitude, z)
	{
		return new Vector3(longitude / Math.PI, Math.sin(latitude), z);
	},
	mercatorMap: function mercatorMap(longitude, latitude, z)
	{
		return new Vector3(longitude / Math.PI, Math.log(Math.tan(Math.PI / 4 + Math.max(-1.5, Math.min(latitude * 0.4, 1.5)))) / 1.75, z);
	},
};

var mapProjectionsInverse =
{
	equalAreaMap: function equalAreaMap(x, y)
	{
		return { longitude: x * Math.PI, latitude: Math.asin(y) };
	},
	mercatorMap: function mercatorMap(x, y)
	{
		return { longitude: x * Math.PI, latitude: (Math.atan(Math.exp(y * 1.75)) - Math.PI / 4) * 2.5 };
	},
};