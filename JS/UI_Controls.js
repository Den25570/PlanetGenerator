function resizeHandler()
{
	updateCamera();
	renderer.setSize(window.innerWidth, window.innerHeight);
}

function resetCamera()
{
	zoom = 1.0;
	zoomAnimationStartTime = null;
	zoomAnimationDuration = null;
	zoomAnimationStartValue = null;
	zoomAnimationEndValue = null;
	cameraLatitude = 0;
	cameraLongitude = 0;
}

function updateCamera()
{
	camera.aspect = window.innerWidth / window.innerHeight;
	
	var transformation = new THREE.Matrix4().makeRotationFromEuler(new THREE.Euler(cameraLatitude, cameraLongitude, 0, "YXZ"));
	camera.position.set(0, -50, 1050);
	camera.position.lerp(new Vector3(0, 0, 2000), Math.pow(zoom, 2.0));
	camera.position.applyMatrix4(transformation);
	camera.up.set(0, 1, 0);
	camera.up.applyMatrix4(transformation);
	camera.lookAt(new Vector3(0, 0, 1000).applyMatrix4(transformation));
	camera.updateProjectionMatrix();

	if (projectionRenderMode !== "globe")
	{
		renderMap();
	}
}

function zoomHandler(event)
{
	if (zoomAnimationStartTime === null)
	{
		zoomAnimationStartTime = Date.now();
		zoomAnimationStartValue = zoom;
		zoomAnimationEndValue = Math.max(0, Math.min(zoomAnimationStartValue - event.deltaY * 0.04, 1));
		zoomAnimationDuration = Math.abs(zoomAnimationStartValue - zoomAnimationEndValue) * 1000;
	}
	else
	{
		zoomAnimationStartTime = Date.now();
		zoomAnimationStartValue = zoom;
		zoomAnimationEndValue = Math.max(0, Math.min(zoomAnimationEndValue - event.deltaY * 0.04, 1));
		zoomAnimationDuration = Math.abs(zoomAnimationStartValue - zoomAnimationEndValue) * 1000;
	}
}

function selectTile(tile)
{
	if (tileSelection !== null)
	{
		if (tileSelection.tile === tile) return;
		deselectTile();
	}
	
	console.log(tile);
	
	var outerColor = new THREE.Color(0x000000);
	var innerColor = new THREE.Color(0xFFFFFF);
	
	var geometry = new THREE.Geometry();
	
	geometry.vertices.push(tile.averagePosition);
	for (var i = 0; i < tile.corners.length; ++i)
	{
		geometry.vertices.push(tile.corners[i].position);
		geometry.faces.push(new THREE.Face3(i + 1, (i + 1) % tile.corners.length + 1, 0, tile.normal, [ outerColor, outerColor, innerColor ]));
	}

	geometry.boundingSphere = tile.boundingSphere.clone();

	var material = new THREE.MeshLambertMaterial({ vertexColors: THREE.VertexColors, });
	material.transparent = true;
	material.opacity = 0.5;
	material.polygonOffset = true;
	material.polygonOffsetFactor = -2;
	material.polygonOffsetUnits = -2;
	tileSelection = { tile: tile, geometry: geometry, material: material, renderObject: new THREE.Mesh(geometry, material) };
	planet.renderData.surface.renderObject.add(tileSelection.renderObject);
	
	if (projectionRenderMode !== "globe")
	{
		project = mapProjections[projectionRenderMode];
		if (project)
		{
			tileSelection.mapMaterial = material.clone();
			projectMap(tileSelection, null, project);
			planet.renderData.surface.mapRenderObject.add(tileSelection.mapRenderObject);
		}
	}
}

function deselectTile()
{
	if (tileSelection !== null)
	{
		planet.renderData.surface.renderObject.remove(tileSelection.renderObject);
		if (tileSelection.mapRenderObject)
		{
			planet.renderData.surface.mapRenderObject.remove(tileSelection.mapRenderObject);
		}
		tileSelection = null;
	}
}

function clickHandler(event)
{
	if (planet)
	{
		var x = event.pageX / renderer.domElement.width * 2 - 1;
		var y = 1 - event.pageY / renderer.domElement.height * 2;
		var ray;
		if (projectionRenderMode === "globe")
		{
			var rayCaster = projector.pickingRay(new Vector3(x, y, 0), camera);
			ray = rayCaster.ray;
		}
		else
		{
			var projectInverse = mapProjectionsInverse[projectionRenderMode];
			if (!projectInverse) return;
			
			var pos = projectInverse(x, y);
			var origin = new Vector3(
				Math.sin(pos.longitude) * 2000 * Math.cos(pos.latitude),
				Math.sin(pos.latitude) * 2000,
				Math.cos(pos.longitude) * 2000 * Math.cos(pos.latitude));
			var transformation = new THREE.Matrix4().makeRotationFromEuler(new THREE.Euler(cameraLatitude, cameraLongitude, 0, "ZYX"));
			origin.applyMatrix4(transformation);
			ray = new THREE.Ray(origin, origin.clone().negate().normalize());
		}
		var intersection = planet.partition.intersectRay(ray);
		if (intersection !== false)
			selectTile(intersection);
		else
			deselectTile();
	}
}

function keyDownHandler(event)
{
	if (disableKeys === true ) return;
	if (event.ctrlKey || event.altKey || event.shiftKey || event.metaKey) return;
	
	switch (event.which)
	{
		case KEY.W:
		case KEY.A:
		case KEY.S:
		case KEY.D:
		case KEY.Z:
		case KEY.Q:
		case KEY_LEFTARROW:
		case KEY_RIGHTARROW:
		case KEY_UPARROW:
		case KEY_DOWNARROW:
		case KEY_PAGEUP:
		case KEY_PAGEDOWN:
		case KEY_NUMPAD_PLUS:
		case KEY_NUMPAD_MINUS:
			pressedKeys[event.which] = true;
			event.preventDefault();
			break;
	}
}

function keyUpHandler(event)
{
	if (disableKeys === true ) return;
	if (event.ctrlKey || event.altKey || event.shiftKey || event.metaKey) return;

	switch (event.which)
	{
		case KEY.W:
		case KEY.A:
		case KEY.S:
		case KEY.D:
		case KEY.Z:
		case KEY.Q:
		case KEY_LEFTARROW:
		case KEY_RIGHTARROW:
		case KEY_UPARROW:
		case KEY_DOWNARROW:
		case KEY_PAGEUP:
		case KEY_PAGEDOWN:
		case KEY_NUMPAD_PLUS:
		case KEY_NUMPAD_MINUS:
			pressedKeys[event.which] = false;
			event.preventDefault();
			break;
		case KEY_ESCAPE:
			if (activeAction !== null)
			{
				ui.progressCancelButton.click();
				event.preventDefault();
			}
			break;
		case KEY_FORWARD_SLASH:
		case KEY["0"]:
			showHideInterface();
			event.preventDefault();
			break;
		case KEY_SPACE:
			generatePlanetAsynchronous();
			event.preventDefault();
			break;
		case KEY["1"]:
			setSubdivisions(20);
			generatePlanetAsynchronous();
			event.preventDefault();
			break;
		case KEY["2"]:
			setSubdivisions(40);
			generatePlanetAsynchronous();
			event.preventDefault();
			break;
		case KEY["3"]:
			setSubdivisions(60);
			generatePlanetAsynchronous();
			event.preventDefault();
			break;
		case KEY["5"]:
			setSurfaceRenderMode("terrain");
			event.preventDefault();
			break;
		case KEY["6"]:
			setSurfaceRenderMode("plates");
			event.preventDefault();
			break;
		case KEY["7"]:
			setSurfaceRenderMode("elevation");
			event.preventDefault();
			break;
		case KEY["8"]:
			setSurfaceRenderMode("temperature");
			event.preventDefault();
			break;
		case KEY["9"]:
			setSurfaceRenderMode("moisture");
			event.preventDefault();
			break;
		case KEY.G:
			setProjectionRenderMode("globe");
			event.preventDefault();
			break;
		case KEY.H:
			setProjectionRenderMode("equalAreaMap");
			event.preventDefault();
			break;
		case KEY.J:
			setProjectionRenderMode("mercatorMap");
			event.preventDefault();
			break;
		case KEY.U:
			showHideSunlight();
			event.preventDefault();
			break;
		case KEY.I:
			showHidePlateBoundaries();
			event.preventDefault();
			break;
		case KEY.O:
			showHidePlateMovements();
			event.preventDefault();
			break;
		case KEY.P:
			showHideAirCurrents();
			event.preventDefault();
			break;
	}
}

function cancelButtonHandler()
{
	if (activeAction !== null)
	{
		activeAction.cancel();
	}
}

function showHideInterface()
{
	ui.helpPanel.toggle();
	ui.controlPanel.toggle();
	ui.dataPanel.toggle();
}

function updateUI()
{
	ui.tileCountLabel.text(planet.statistics.tiles.count.toFixed(0));
	ui.pentagonCountLabel.text(planet.statistics.tiles.pentagonCount.toFixed(0));
	ui.hexagonCountLabel.text(planet.statistics.tiles.hexagonCount.toFixed(0));
	ui.heptagonCountLabel.text(planet.statistics.tiles.heptagonCount.toFixed(0));
	ui.plateCountLabel.text(planet.statistics.plates.count.toFixed(0));
	ui.waterPercentageLabel.text(((planet.statistics.tiles.biomeAreas["ocean"] + planet.statistics.tiles.biomeAreas["oceanGlacier"]) / planet.statistics.tiles.totalArea * 100).toFixed(0) + "%");

	ui.rawSeedLabel.val(planet.seed);
	ui.originalSeedLabel.val(planet.originalSeed !== null ? planet.originalSeed : "");

	ui.minAirCurrentSpeedLabel.text(planet.statistics.corners.airCurrent.min.toFixed(0));
	ui.avgAirCurrentSpeedLabel.text(planet.statistics.corners.airCurrent.avg.toFixed(0));
	ui.maxAirCurrentSpeedLabel.text(planet.statistics.corners.airCurrent.max.toFixed(0));

	ui.minElevationLabel.text((planet.statistics.tiles.elevation.min * 100).toFixed(0));
	ui.avgElevationLabel.text((planet.statistics.tiles.elevation.avg * 100).toFixed(0));
	ui.maxElevationLabel.text((planet.statistics.tiles.elevation.max * 100).toFixed(0));

	ui.minTemperatureLabel.text((planet.statistics.tiles.temperature.min * 100).toFixed(0));
	ui.avgTemperatureLabel.text((planet.statistics.tiles.temperature.avg * 100).toFixed(0));
	ui.maxTemperatureLabel.text((planet.statistics.tiles.temperature.max * 100).toFixed(0));

	ui.minMoistureLabel.text((planet.statistics.tiles.moisture.min * 100).toFixed(0));
	ui.avgMoistureLabel.text((planet.statistics.tiles.moisture.avg * 100).toFixed(0));
	ui.maxMoistureLabel.text((planet.statistics.tiles.moisture.max * 100).toFixed(0));

	ui.minPlateMovementSpeedLabel.text(planet.statistics.tiles.plateMovement.min.toFixed(0));
	ui.avgPlateMovementSpeedLabel.text(planet.statistics.tiles.plateMovement.avg.toFixed(0));
	ui.maxPlateMovementSpeedLabel.text(planet.statistics.tiles.plateMovement.max.toFixed(0));

	ui.minTileAreaLabel.text(planet.statistics.tiles.area.min.toFixed(0));
	ui.avgTileAreaLabel.text(planet.statistics.tiles.area.avg.toFixed(0));
	ui.maxTileAreaLabel.text(planet.statistics.tiles.area.max.toFixed(0));

	ui.minPlateAreaLabel.text((planet.statistics.plates.area.min / 1000).toFixed(0) + "K");
	ui.avgPlateAreaLabel.text((planet.statistics.plates.area.avg / 1000).toFixed(0) + "K");
	ui.maxPlateAreaLabel.text((planet.statistics.plates.area.max / 1000).toFixed(0) + "K");

	ui.minPlateCircumferenceLabel.text(planet.statistics.plates.circumference.min.toFixed(0));
	ui.avgPlateCircumferenceLabel.text(planet.statistics.plates.circumference.avg.toFixed(0));
	ui.maxPlateCircumferenceLabel.text(planet.statistics.plates.circumference.max.toFixed(0));
}