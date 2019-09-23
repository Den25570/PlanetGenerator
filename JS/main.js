// Copyright В© 2014 Andy Gainey <andy@experilous.com>
//
// Usage of the works is permitted provided that this instrument 
// is retained with the works, so that any entity that uses the
// works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

var scene = null;
var camera = null;
var renderer = null;
var projector = null;
var directionalLight = null;
var activeAction = null;
var planet = null;
var tileSelection = null;
var zoom = 1.0;
var zoomAnimationStartTime = null;
var zoomAnimationDuration = null;
var zoomAnimationStartValue = null;
var zoomAnimationEndValue = null;
var cameraLatitude = 0;
var cameraLongitude = 0;
var projectionRenderMode = "globe";
var surfaceRenderMode = "terrain";
var renderSunlight = true;
var renderPlateBoundaries = false;
var renderPlateMovements = false;
var renderAirCurrents = false;
var sunTimeOffset = 0;
var pressedKeys = {};
var disableKeys = false;
var ui = {};

var generationSettings =
{
	subdivisions: 20,
	distortionLevel: 1,
	plateCount: 36,
	oceanicRate: 0.7,
	heatLevel: 1.0,
	moistureLevel: 1.0,
	seed: null,
};

var Vector3 = THREE.Vector3;

var KEY_ENTER = 13;
var KEY_SHIFT = 16;
var KEY_ESCAPE = 27;
var KEY_SPACE = 32;
var KEY_LEFTARROW = 37;
var KEY_UPARROW = 38;
var KEY_RIGHTARROW = 39;
var KEY_DOWNARROW = 40;
var KEY_PAGEUP = 33;
var KEY_PAGEDOWN = 34;
var KEY_NUMPAD_PLUS = 107;
var KEY_NUMPAD_MINUS = 109;
var KEY_FORWARD_SLASH = 191;

var KEY = {};
for (var k = 0; k < 10; ++k) KEY[ String.fromCharCode(k + 48) ] = k + 48;
for (var k = 0; k < 26; ++k) KEY[ String.fromCharCode(k + 65) ] = k + 65;

$(document).ready(function onDocumentReady()
{
	scene = new THREE.Scene();
	camera = new THREE.PerspectiveCamera(75, 1, 0.2, 2000);
	mapCamera = new THREE.OrthographicCamera(-1, 1, -1, 1, 1, 10);
	renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
	projector = new THREE.Projector();

	mapCamera.position.set(0, 0, 8);
	mapCamera.up.set(0, 1, 0);
	mapCamera.lookAt(new Vector3(0, 0, 0));
	mapCamera.updateProjectionMatrix();
	
	renderer.setFaceCulling(THREE.CullFaceFront, THREE.FrontFaceDirectionCW);

	var ambientLight = new THREE.AmbientLight(0xFFFFFF);
	scene.add(ambientLight);
	
	directionalLight = new THREE.DirectionalLight(0xFFFFFF);
	directionalLight.position.set(-3, 3, 7).normalize();
	scene.add(directionalLight);

	requestAnimationFrame(render);

	resetCamera();
	updateCamera();

	ui.body = $("body");
	ui.frame = $("#viewportFrame");
	ui.rendererElement = $(renderer.domElement);
	ui.frame.append(ui.rendererElement);
	ui.rendererElement.on("mousewheel", zoomHandler);
	ui.rendererElement.on("click", clickHandler);
	ui.body.on("keydown", keyDownHandler);
	ui.body.on("keyup", keyUpHandler);
	ui.body.focus();
	
	ui.helpPanel = $("#helpPanel");

	ui.controlPanel = $("#controlPanel");
	ui.projectionDisplayButtons =
	{
		globe: $("#projectGlobe"),
		equalAreaMap: $("#projectEqualAreaMap"),
		mercatorMap: $("#projectMercatorMap"),
	};
	
	ui.projectionDisplayButtons.globe.click(setProjectionRenderMode.bind(null, "globe"));
	ui.projectionDisplayButtons.equalAreaMap.click(setProjectionRenderMode.bind(null, "equalAreaMap"));
	ui.projectionDisplayButtons.mercatorMap.click(setProjectionRenderMode.bind(null, "mercatorMap"));

	ui.surfaceDisplayButtons =
	{
		terrain: $("#showTerrainButton"),
		plates: $("#showPlatesButton"),
		elevation: $("#showElevationButton"),
		temperature: $("#showTemperatureButton"),
		moisture: $("#showMoistureButton"),
	};
	
	ui.surfaceDisplayButtons.terrain.click(setSurfaceRenderMode.bind(null, "terrain"));
	ui.surfaceDisplayButtons.plates.click(setSurfaceRenderMode.bind(null, "plates"));
	ui.surfaceDisplayButtons.elevation.click(setSurfaceRenderMode.bind(null, "elevation"));
	ui.surfaceDisplayButtons.temperature.click(setSurfaceRenderMode.bind(null, "temperature"));
	ui.surfaceDisplayButtons.moisture.click(setSurfaceRenderMode.bind(null, "moisture"));

	ui.showSunlightButton = $("#showSunlightButton");
	ui.showPlateBoundariesButton = $("#showPlateBoundariesButton");
	ui.showPlateMovementsButton = $("#showPlateMovementsButton");
	ui.showAirCurrentsButton = $("#showAirCurrentsButton");

	ui.showSunlightButton.click(showHideSunlight);
	ui.showPlateBoundariesButton.click(showHidePlateBoundaries);
	ui.showPlateMovementsButton.click(showHidePlateMovements);
	ui.showAirCurrentsButton.click(showHideAirCurrents);
	
	ui.lowDetailButton = $("#lowDetailButton");
	ui.mediumDetailButton = $("#mediumDetailButton");
	ui.highDetailButton = $("#highDetailButton");
	ui.generatePlanetButton = $("#generatePlanetButton");
	ui.advancedSettingsButton = $("#advancedSettingsButton");
	
	ui.lowDetailButton.click(setSubdivisions.bind(null, 20));
	ui.mediumDetailButton.click(setSubdivisions.bind(null, 40));
	ui.highDetailButton.click(setSubdivisions.bind(null, 60));
	ui.generatePlanetButton.click(generatePlanetAsynchronous);
	ui.advancedSettingsButton.click(showAdvancedSettings);

	ui.dataPanel = $("#dataPanel");

	ui.progressPanel = $("#progressPanel");
	ui.progressActionLabel = $("#progressActionLabel");
	ui.progressBarFrame = $("#progressBarFrame");
	ui.progressBar = $("#progressBar");
	ui.progressBarLabel = $("#progressBarLabel");
	ui.progressCancelButton = $("#progressCancelButton");
	ui.progressCancelButton.click(cancelButtonHandler);
	ui.progressPanel.hide();

	ui.tileCountLabel = $("#tileCountLabel");
	ui.pentagonCountLabel = $("#pentagonCountLabel");
	ui.hexagonCountLabel = $("#hexagonCountLabel");
	ui.heptagonCountLabel = $("#heptagonCountLabel");
	ui.plateCountLabel = $("#plateCountLabel");
	ui.waterPercentageLabel = $("#waterPercentageLabel");
	ui.rawSeedLabel = $("#rawSeedLabel");
	ui.originalSeedLabel = $("#originalSeedLabel");

	ui.minAirCurrentSpeedLabel = $("#minAirCurrentSpeedLabel");
	ui.avgAirCurrentSpeedLabel = $("#avgAirCurrentSpeedLabel");
	ui.maxAirCurrentSpeedLabel = $("#maxAirCurrentSpeedLabel");

	ui.minElevationLabel = $("#minElevationLabel");
	ui.avgElevationLabel = $("#avgElevationLabel");
	ui.maxElevationLabel = $("#maxElevationLabel");

	ui.minTemperatureLabel = $("#minTemperatureLabel");
	ui.avgTemperatureLabel = $("#avgTemperatureLabel");
	ui.maxTemperatureLabel = $("#maxTemperatureLabel");

	ui.minMoistureLabel = $("#minMoistureLabel");
	ui.avgMoistureLabel = $("#avgMoistureLabel");
	ui.maxMoistureLabel = $("#maxMoistureLabel");

	ui.minPlateMovementSpeedLabel = $("#minPlateMovementSpeedLabel");
	ui.avgPlateMovementSpeedLabel = $("#avgPlateMovementSpeedLabel");
	ui.maxPlateMovementSpeedLabel = $("#maxPlateMovementSpeedLabel");

	ui.minTileAreaLabel = $("#minTileAreaLabel");
	ui.avgTileAreaLabel = $("#avgTileAreaLabel");
	ui.maxTileAreaLabel = $("#maxTileAreaLabel");

	ui.minPlateAreaLabel = $("#minPlateAreaLabel");
	ui.avgPlateAreaLabel = $("#avgPlateAreaLabel");
	ui.maxPlateAreaLabel = $("#maxPlateAreaLabel");

	ui.minPlateCircumferenceLabel = $("#minPlateCircumferenceLabel");
	ui.avgPlateCircumferenceLabel = $("#avgPlateCircumferenceLabel");
	ui.maxPlateCircumferenceLabel = $("#maxPlateCircumferenceLabel");

	ui.generationSettingsPanel = $("#generationSettingsPanel");
	
	ui.detailLevelLabel = $("#detailLevelLabel");
	ui.detailLevelRange = $("#detailLevelRange");
	ui.distortionLevelLabel = $("#distortionLevelLabel");
	ui.distortionLevelRange = $("#distortionLevelRange");
	ui.tectonicPlateCountLabel = $("#tectonicPlateCountLabel");
	ui.tectonicPlateCountRange = $("#tectonicPlateCountRange");
	ui.oceanicRateLabel = $("#oceanicRateLabel");
	ui.oceanicRateRange = $("#oceanicRateRange");
	ui.heatLevelLabel = $("#heatLevelLabel");
	ui.heatLevelRange = $("#heatLevelRange");
	ui.moistureLevelLabel = $("#moistureLevelLabel");
	ui.moistureLevelRange = $("#moistureLevelRange");
	ui.seedTextBox = $("#seedTextBox");
	ui.advancedGeneratePlanetButton = $("#advancedGeneratePlanetButton");
	ui.advancedCancelButton = $("#advancedCancelButton");
	
	ui.detailLevelRange.on("input", function() { setSubdivisions(parseInt(ui.detailLevelRange.val())); });
	ui.distortionLevelRange.on("input", function() { setDistortionLevel(parseInt(ui.distortionLevelRange.val()) / 100); });
	ui.tectonicPlateCountRange.on("input", function() { setPlateCount(Math.floor(Math.pow(2, parseInt(ui.tectonicPlateCountRange.val()) / 300 * (Math.log(1000) / Math.log(2) - 1) + 1))); });
	ui.oceanicRateRange.on("input", function() { setOceanicRate(parseInt(ui.oceanicRateRange.val()) / 100); });
	ui.heatLevelRange.on("input", function() { setHeatLevel(parseInt(ui.heatLevelRange.val()) / 100 + 1); });
	ui.moistureLevelRange.on("input", function() { setMoistureLevel(parseInt(ui.moistureLevelRange.val()) / 100 + 1); });
	ui.seedTextBox.on("input", function() { setSeed(ui.seedTextBox.val()); });
	ui.advancedGeneratePlanetButton.click(function() { hideAdvancedSettings(); generatePlanetAsynchronous(); });
	ui.advancedCancelButton.click(hideAdvancedSettings);
	
	$("button").on("click", function(b) { $(this).blur(); });
	$("button").on("focus", function() { disableKeys = true; });
	$("input").on("focus", function() { disableKeys = true; });
	$("button").on("blur", function() { disableKeys = false; });
	$("input").on("blur", function() { disableKeys = false; });
	
	hideAdvancedSettings();
	setPlateCount(50);

	setProjectionRenderMode(projectionRenderMode, true);
	setSurfaceRenderMode(surfaceRenderMode, true);
	showHideSunlight(renderSunlight);
	showHidePlateBoundaries(renderPlateBoundaries);
	showHidePlateMovements(renderPlateMovements);
	showHideAirCurrents(renderAirCurrents);

	ui.lowDetailButton.click();

	//saveToFileSystem(serializePlanetMesh(planet.mesh, "function getPregeneratedPlanetMesh() { return ", "; }\n"));

	window.addEventListener("resize", resizeHandler);
	resizeHandler();
	
	ui.generatePlanetButton.click();
});

function setSubdivisions(subdivisions)
{
	if (typeof(subdivisions) === "number" && subdivisions >= 4)
	{
		generationSettings.subdivisions = subdivisions;
		$("#detailDisplaylist>button.toggled").removeClass("toggled");
		if (subdivisions === 20) ui.lowDetailButton.addClass("toggled");
		else if (subdivisions === 40) ui.mediumDetailButton.addClass("toggled");
		else if (subdivisions === 60) ui.highDetailButton.addClass("toggled");
		
		subdivisions = subdivisions.toFixed(0);
		if (ui.detailLevelRange.val() !== subdivisions) ui.detailLevelRange.val(subdivisions);
		ui.detailLevelLabel.text("Detail Level (" + subdivisions + ")");
	}
}

function setDistortionLevel(distortionLevel)
{
	if (typeof(distortionLevel) === "number" && distortionLevel >= 0 && distortionLevel <= 1)
	{
		generationSettings.distortionLevel = distortionLevel;
		
		distortionLevel = Math.floor(distortionLevel * 100 + 0.5).toFixed(0);
		
		if (ui.distortionLevelRange.val() !== distortionLevel) ui.distortionLevelRange.val(distortionLevel);
		ui.distortionLevelLabel.text("Distortion Level (" + distortionLevel + "%)");
	}
}

function setPlateCount(plateCount)
{
	if (typeof(plateCount) === "number" && plateCount >= 0)
	{
		generationSettings.plateCount = plateCount;
		
		var sliderVal = Math.ceil((Math.log(plateCount) / Math.log(2) - 1) / (Math.log(1000) / Math.log(2) - 1) * 300).toFixed(0);
		if (ui.tectonicPlateCountRange.val() !== sliderVal) ui.tectonicPlateCountRange.val(sliderVal);
		ui.tectonicPlateCountLabel.text(plateCount.toFixed(0));
	}
}

function setOceanicRate(oceanicRate)
{
	if (typeof(oceanicRate) === "number" && oceanicRate >= 0 && oceanicRate <= 1)
	{
		generationSettings.oceanicRate = oceanicRate;
		
		oceanicRate = Math.floor(oceanicRate * 100 + 0.5).toFixed(0);
		
		if (ui.oceanicRateRange.val() !== oceanicRate) ui.oceanicRateRange.val(oceanicRate);
		ui.oceanicRateLabel.text(oceanicRate);
	}
}

function setHeatLevel(heatLevel)
{
	if (typeof(heatLevel) === "number" && heatLevel >= 0)
	{
		generationSettings.heatLevel = heatLevel;
		
		heatLevel = Math.floor(heatLevel * 100 - 100).toFixed(0);
		
		if (ui.heatLevelRange.val() !== heatLevel) ui.heatLevelRange.val(heatLevel);
		if (generationSettings.heatLevel > 1) heatLevel = "+" + heatLevel;
		else if (generationSettings.heatLevel < 1) heatLevel = "-" + heatLevel;
		ui.heatLevelLabel.text(heatLevel);
	}
}

function setMoistureLevel(moistureLevel)
{
	if (typeof(moistureLevel) === "number" && moistureLevel >= 0)
	{
		generationSettings.moistureLevel = moistureLevel;
		
		moistureLevel = Math.floor(moistureLevel * 100 - 100).toFixed(0);
		
		if (ui.moistureLevelRange.val() !== moistureLevel) ui.moistureLevelRange.val(moistureLevel);
		if (generationSettings.moistureLevel > 1) moistureLevel = "+" + moistureLevel;
		else if (generationSettings.moistureLevel < 1) moistureLevel = "-" + moistureLevel;
		ui.moistureLevelLabel.text(moistureLevel);
	}
}

function setSeed(seed)
{
	if (!seed) generationSettings.seed = null;
	if (typeof(seed) === "number")
	{
		generationSettings.seed = Math.floor(seed);
		ui.seedTextBox.val(generationSettings.seed.toFixed(0));
	}
	else if (typeof(seed) === "string")
	{
		var asInt = parseInt(seed);
		if (isNaN(asInt) || asInt.toFixed(0) !== seed)
		{
			generationSettings.seed = seed;
		}
		else
		{
			generationSettings.seed = asInt;
			ui.seedTextBox.val(generationSettings.seed.toFixed(0));
		}
	}
	else
	{
		generationSettings.seed = null;
		ui.seedTextBox.val("");
	}
}

function showAdvancedSettings()
{
	ui.generationSettingsPanel.show();
}

function hideAdvancedSettings()
{
	ui.generationSettingsPanel.hide();
}

function calculateCornerDistancesToPlateRoot(plates, action)
{
	var distanceCornerQueue = [];
	for (var i = 0; i < plates.length; ++i)
	{
		var corner = plates[i].root;
		corner.distanceToPlateRoot = 0;
		for (var j = 0; j < corner.corners.length; ++j)
		{
			distanceCornerQueue.push({ corner: corner.corners[j], distanceToPlateRoot: corner.borders[j].length() });
		}
	}
	
	var distanceCornerQueueSorter = function(left, right) { return left.distanceToPlateRoot - right.distanceToPlateRoot; };

	action.executeSubaction(function(action)
	{
		if (distanceCornerQueue.length === 0) return;

		var iEnd = iEnd = distanceCornerQueue.length;
		for (var i = 0; i < iEnd; ++i)
		{
			var front = distanceCornerQueue[i];
			var corner = front.corner;
			var distanceToPlateRoot = front.distanceToPlateRoot;
			if (!corner.distanceToPlateRoot || corner.distanceToPlateRoot > distanceToPlateRoot)
			{
				corner.distanceToPlateRoot = distanceToPlateRoot;
				for (var j = 0; j < corner.corners.length; ++j)
				{
					distanceCornerQueue.push({ corner: corner.corners[j], distanceToPlateRoot: distanceToPlateRoot + corner.borders[j].length() });
				}
			}
		}
		distanceCornerQueue.splice(0, iEnd);
		distanceCornerQueue.sort(distanceCornerQueueSorter);
		
		action.loop();
	});
}

function identifyBoundaryBorders(borders, action)
{
	for (var i = 0; i < borders.length; ++i)
	{
		var border = borders[i];
		if (border.tiles[0].plate !== border.tiles[1].plate)
		{
			border.betweenPlates = true;
			border.corners[0].betweenPlates = true;
			border.corners[1].betweenPlates = true;
			border.tiles[0].plate.boundaryBorders.push(border);
			border.tiles[1].plate.boundaryBorders.push(border);
		}
	}
}

function collectBoundaryCorners(corners, action)
{
	var boundaryCorners = [];
	for (var j = 0; j < corners.length; ++j)
	{
		var corner = corners[j];
		if (corner.betweenPlates)
		{
			boundaryCorners.push(corner);
			corner.tiles[0].plate.boundaryCorners.push(corner);
			if (corner.tiles[1].plate !== corner.tiles[0].plate) corner.tiles[1].plate.boundaryCorners.push(corner);
			if (corner.tiles[2].plate !== corner.tiles[0].plate && corner.tiles[2].plate !== corner.tiles[1].plate) corner.tiles[2].plate.boundaryCorners.push(corner);
		}
	}
	
	action.provideResult(boundaryCorners);
}

function calculatePlateBoundaryStress(boundaryCorners, action)
{
	var boundaryCornerInnerBorderIndexes = new Array(boundaryCorners.length);
	for (var i = 0; i < boundaryCorners.length; ++i)
	{
		var corner = boundaryCorners[i];
		corner.distanceToPlateBoundary = 0;
	
		var innerBorder;
		var innerBorderIndex;
		for (var j = 0; j < corner.borders.length; ++j)
		{
			var border = corner.borders[j];
			if (!border.betweenPlates)
			{
				innerBorder = border;
				innerBorderIndex = j;
				break;
			}
		}
		
		if (innerBorder)
		{
			boundaryCornerInnerBorderIndexes[i] = innerBorderIndex;
			var outerBorder0 = corner.borders[(innerBorderIndex + 1) % corner.borders.length];
			var outerBorder1 = corner.borders[(innerBorderIndex + 2) % corner.borders.length]
			var farCorner0 = outerBorder0.oppositeCorner(corner);
			var farCorner1 = outerBorder1.oppositeCorner(corner);
			var plate0 = innerBorder.tiles[0].plate;
			var plate1 = outerBorder0.tiles[0].plate !== plate0 ? outerBorder0.tiles[0].plate : outerBorder0.tiles[1].plate;
			var boundaryVector = farCorner0.vectorTo(farCorner1);
			var boundaryNormal = boundaryVector.clone().cross(corner.position);
			var stress = calculateStress(plate0.calculateMovement(corner.position), plate1.calculateMovement(corner.position), boundaryVector, boundaryNormal);
			corner.pressure = stress.pressure;
			corner.shear = stress.shear;
		}
		else
		{
			boundaryCornerInnerBorderIndexes[i] = null;
			var plate0 = corner.tiles[0].plate;
			var plate1 = corner.tiles[1].plate;
			var plate2 = corner.tiles[2].plate;
			var boundaryVector0 = corner.corners[0].vectorTo(corner);
			var boundaryVector1 = corner.corners[1].vectorTo(corner);
			var boundaryVector2 = corner.corners[2].vectorTo(corner);
			var boundaryNormal0 = boundaryVector0.clone().cross(corner.position);
			var boundaryNormal1 = boundaryVector1.clone().cross(corner.position);
			var boundaryNormal2 = boundaryVector2.clone().cross(corner.position);
			var stress0 = calculateStress(plate0.calculateMovement(corner.position), plate1.calculateMovement(corner.position), boundaryVector0, boundaryNormal0);
			var stress1 = calculateStress(plate1.calculateMovement(corner.position), plate2.calculateMovement(corner.position), boundaryVector1, boundaryNormal1);
			var stress2 = calculateStress(plate2.calculateMovement(corner.position), plate0.calculateMovement(corner.position), boundaryVector2, boundaryNormal2);
			
			corner.pressure = (stress0.pressure + stress1.pressure + stress2.pressure) / 3;
			corner.shear = (stress0.shear + stress1.shear + stress2.shear) / 3;
		}
	}
	
	action.provideResult(boundaryCornerInnerBorderIndexes);
}

function calculateStress(movement0, movement1, boundaryVector, boundaryNormal)
{
	var relativeMovement = movement0.clone().sub(movement1);
	var pressureVector = relativeMovement.clone().projectOnVector(boundaryNormal);
	var pressure = pressureVector.length();
	if (pressureVector.dot(boundaryNormal) > 0) pressure = -pressure;
	var shear = relativeMovement.clone().projectOnVector(boundaryVector).length();
	return { pressure: 2 / (1 + Math.exp(-pressure / 30)) - 1, shear: 2 / (1 + Math.exp(-shear / 30)) - 1 };
}

function blurPlateBoundaryStress(boundaryCorners, stressBlurIterations, stressBlurCenterWeighting, action)
{
	var newCornerPressure = new Array(boundaryCorners.length);
	var newCornerShear = new Array(boundaryCorners.length);
	for (var i = 0; i < stressBlurIterations; ++i)
	{
		for (var j = 0; j < boundaryCorners.length; ++j)
		{
			var corner = boundaryCorners[j];
			var averagePressure = 0;
			var averageShear = 0;
			var neighborCount = 0;
			for (var k = 0; k < corner.corners.length; ++k)
			{
				var neighbor = corner.corners[k];
				if (neighbor.betweenPlates)
				{
					averagePressure += neighbor.pressure;
					averageShear += neighbor.shear;
					++neighborCount;
				}
			}
			newCornerPressure[j] = corner.pressure * stressBlurCenterWeighting + (averagePressure / neighborCount) * (1 - stressBlurCenterWeighting);
			newCornerShear[j] = corner.shear * stressBlurCenterWeighting + (averageShear / neighborCount) * (1 - stressBlurCenterWeighting);
		}
		
		for (var j = 0; j < boundaryCorners.length; ++j)
		{
			var corner = boundaryCorners[j];
			if (corner.betweenPlates)
			{
				corner.pressure = newCornerPressure[j];
				corner.shear = newCornerShear[j];
			}
		}
	}
}

function populateElevationBorderQueue(boundaryCorners, boundaryCornerInnerBorderIndexes, action)
{
	var elevationBorderQueue = [];
	for (var i = 0; i < boundaryCorners.length; ++i)
	{
		var corner = boundaryCorners[i];
		
		var innerBorderIndex = boundaryCornerInnerBorderIndexes[i];
		if (innerBorderIndex !== null)
		{
			var innerBorder = corner.borders[innerBorderIndex];
			var outerBorder0 = corner.borders[(innerBorderIndex + 1) % corner.borders.length];
			var plate0 = innerBorder.tiles[0].plate;
			var plate1 = outerBorder0.tiles[0].plate !== plate0 ? outerBorder0.tiles[0].plate : outerBorder0.tiles[1].plate;
			
			var calculateElevation;
			
			if (corner.pressure > 0.3)
			{
				corner.elevation = Math.max(plate0.elevation, plate1.elevation) + corner.pressure;
				if (plate0.oceanic === plate1.oceanic)
					calculateElevation = calculateCollidingElevation;
				else if (plate0.oceanic)
					calculateElevation = calculateSubductingElevation;
				else
					calculateElevation = calculateSuperductingElevation;
			}
			else if (corner.pressure < -0.3)
			{
				corner.elevation = Math.max(plate0.elevation, plate1.elevation) - corner.pressure / 4;
				calculateElevation = calculateDivergingElevation;
			}
			else if (corner.shear > 0.3)
			{
				corner.elevation = Math.max(plate0.elevation, plate1.elevation) + corner.shear / 8;
				calculateElevation = calculateShearingElevation;
			}
			else
			{
				corner.elevation = (plate0.elevation + plate1.elevation) / 2;
				calculateElevation = calculateDormantElevation;
			}
			
			var nextCorner = innerBorder.oppositeCorner(corner);
			if (!nextCorner.betweenPlates)
			{
				elevationBorderQueue.push({
					origin: {
						corner: corner,
						pressure: corner.pressure,
						shear: corner.shear,
						plate: plate0,
						calculateElevation: calculateElevation },
					border: innerBorder,
					corner: corner,
					nextCorner: nextCorner,
					distanceToPlateBoundary: innerBorder.length(),
				});
			}
		}
		else
		{
			var plate0 = corner.tiles[0].plate;
			var plate1 = corner.tiles[1].plate;
			var plate2 = corner.tiles[2].plate;
			
			elevation = 0;

			if (corner.pressure > 0.3)
			{
				corner.elevation = Math.max(plate0.elevation, plate1.elevation, plate2.elevation) + corner.pressure;
			}
			else if (corner.pressure < -0.3)
			{
				corner.elevation = Math.max(plate0.elevation, plate1.elevation, plate2.elevation) + corner.pressure / 4;
			}
			else if (corner.shear > 0.3)
			{
				corner.elevation = Math.max(plate0.elevation, plate1.elevation, plate2.elevation) + corner.shear / 8;
			}
			else
			{
				corner.elevation = (plate0.elevation + plate1.elevation + plate2.elevation) / 3;
			}
		}
	}
	
	action.provideResult(elevationBorderQueue);
}

function processElevationBorderQueue(elevationBorderQueue, elevationBorderQueueSorter, action)
{
	if (elevationBorderQueue.length === 0) return;

	var iEnd = iEnd = elevationBorderQueue.length;
	for (var i = 0; i < iEnd; ++i)
	{
		var front = elevationBorderQueue[i];
		var corner = front.nextCorner;
		if (!corner.elevation)
		{
			corner.distanceToPlateBoundary = front.distanceToPlateBoundary;
			corner.elevation = front.origin.calculateElevation(
				corner.distanceToPlateBoundary,
				corner.distanceToPlateRoot,
				front.origin.corner.elevation,
				front.origin.plate.elevation,
				front.origin.pressure,
				front.origin.shear);
				
			for (var j = 0; j < corner.borders.length; ++j)
			{
				var border = corner.borders[j];
				if (!border.betweenPlates)
				{
					var nextCorner = corner.corners[j];
					var distanceToPlateBoundary = corner.distanceToPlateBoundary + border.length();
					if (!nextCorner.distanceToPlateBoundary || nextCorner.distanceToPlateBoundary > distanceToPlateBoundary)
					{
						elevationBorderQueue.push({
							origin: front.origin,
							border: border,
							corner: corner,
							nextCorner: nextCorner,
							distanceToPlateBoundary: distanceToPlateBoundary,
						});
					}
				}
			}
		}
	}
	elevationBorderQueue.splice(0, iEnd);
	elevationBorderQueue.sort(elevationBorderQueueSorter);
	
	action.loop();
}

function calculateTileAverageElevations(tiles, action)
{
	for (var i = 0; i < tiles.length; ++i)
	{
		var tile = tiles[i];
		var elevation = 0;
		for (var j = 0; j < tile.corners.length; ++j)
		{
			elevation += tile.corners[j].elevation;
		}
		tile.elevation = elevation / tile.corners.length;
	}
}


function generateAirCurrentWhorls(planetRadius, random, action)
{
	var whorls = [];
	var direction = random.integer(0, 1) ? 1 : -1;
	var layerCount = random.integer(4, 7);
	var circumference = Math.PI * 2 * planetRadius;
	var fullRevolution = Math.PI * 2;
	var baseWhorlRadius = circumference / (2 * (layerCount - 1));

	whorls.push({
		center: new Vector3(0, planetRadius, 0)
			.applyAxisAngle(new Vector3(1, 0, 0), random.realInclusive(0, fullRevolution / (2 * (layerCount + 4))))
			.applyAxisAngle(new Vector3(0, 1, 0), random.real(0, fullRevolution)),
		strength: random.realInclusive(fullRevolution / 36, fullRevolution / 24) * direction,
		radius: random.realInclusive(baseWhorlRadius * 0.8, baseWhorlRadius * 1.2) });

	for (var i = 1; i < layerCount - 1; ++i)
	{
		direction = -direction;
		var baseTilt = i / (layerCount - 1) * fullRevolution / 2;
		var layerWhorlCount = Math.ceil((Math.sin(baseTilt) * planetRadius * fullRevolution) / baseWhorlRadius);
		for (var j = 0; j < layerWhorlCount; ++j)
		{
			whorls.push({
				center: new Vector3(0, planetRadius, 0)
					.applyAxisAngle(new Vector3(1, 0, 0), random.realInclusive(0, fullRevolution / (2 * (layerCount + 4))))
					.applyAxisAngle(new Vector3(0, 1, 0), random.real(0, fullRevolution))
					.applyAxisAngle(new Vector3(1, 0, 0), baseTilt)
					.applyAxisAngle(new Vector3(0, 1, 0), fullRevolution * (j + (i % 2) / 2) / layerWhorlCount),
				strength: random.realInclusive(fullRevolution / 48, fullRevolution / 32) * direction,
				radius: random.realInclusive(baseWhorlRadius * 0.8, baseWhorlRadius * 1.2) });
		}
	}

	direction = -direction;
	whorls.push({
		center: new Vector3(0, planetRadius, 0)
			.applyAxisAngle(new Vector3(1, 0, 0), random.realInclusive(0, fullRevolution / (2 * (layerCount + 4))))
			.applyAxisAngle(new Vector3(0, 1, 0), random.real(0, fullRevolution))
			.applyAxisAngle(new Vector3(1, 0, 0), fullRevolution / 2),
		strength: random.realInclusive(fullRevolution / 36, fullRevolution / 24) * direction,
		radius: random.realInclusive(baseWhorlRadius * 0.8, baseWhorlRadius * 1.2) });
		
	action.provideResult(whorls);
}

function calculateAirCurrents(corners, whorls, planetRadius, action)
{
	var i = 0;
	action.executeSubaction(function(action)
	{
		if (i >= corners.length) return;

		var corner = corners[i];
		var airCurrent = new Vector3(0, 0, 0);
		var weight = 0;
		for (var j = 0; j < whorls.length; ++j)
		{
			var whorl = whorls[j];
			var angle = whorl.center.angleTo(corner.position);
			var distance = angle * planetRadius;
			if (distance < whorl.radius)
			{
				var normalizedDistance = distance / whorl.radius;
				var whorlWeight = 1 - normalizedDistance;
				var whorlStrength = planetRadius * whorl.strength * whorlWeight * normalizedDistance;
				var whorlCurrent = whorl.center.clone().cross(corner.position).setLength(whorlStrength);
				airCurrent.add(whorlCurrent);
				weight += whorlWeight;
			}
		}
		airCurrent.divideScalar(weight);
		corner.airCurrent = airCurrent;
		corner.airCurrentSpeed = airCurrent.length(); //kilometers per hour
		
		corner.airCurrentOutflows = new Array(corner.borders.length);
		var airCurrentDirection = airCurrent.clone().normalize();
		var outflowSum = 0;
		for (var j = 0; j < corner.corners.length; ++j)
		{
			var vector = corner.vectorTo(corner.corners[j]).normalize();
			var dot = vector.dot(airCurrentDirection);
			if (dot > 0)
			{
				corner.airCurrentOutflows[j] = dot;
				outflowSum += dot;
			}
			else
			{
				corner.airCurrentOutflows[j] = 0;
			}
		}

		if (outflowSum > 0)
		{
			for (var j = 0; j < corner.borders.length; ++j)
			{
				corner.airCurrentOutflows[j] /= outflowSum;
			}
		}
		
		++i;
		action.loop(i / corners.length);
	});
}

function initializeAirHeat(corners, heatLevel, action)
{
	var activeCorners = [];
	var airHeat = 0;
	for (var i = 0; i < corners.length; ++i)
	{
		var corner = corners[i];
		corner.airHeat = corner.area * heatLevel;
		corner.newAirHeat = 0;
		corner.heat = 0;

		corner.heatAbsorption = 0.1 * corner.area / Math.max(0.1, Math.min(corner.airCurrentSpeed, 1));
		if (corner.elevation <= 0)
		{
			corner.maxHeat = corner.area;
		}
		else
		{
			corner.maxHeat = corner.area;
			corner.heatAbsorption *= 2;
		}
		
		activeCorners.push(corner);
		airHeat += corner.airHeat;
	}
	
	action.provideResult({ corners: activeCorners, airHeat: airHeat });
}

function processAirHeat(activeCorners, action)
{
	var consumedHeat = 0;
	var activeCornerCount = activeCorners.length;
	for (var i = 0; i < activeCornerCount; ++i)
	{
		var corner = activeCorners[i];
		if (corner.airHeat === 0) continue;
		
		var heatChange = Math.max(0, Math.min(corner.airHeat, corner.heatAbsorption * (1 - corner.heat / corner.maxHeat)));
		corner.heat += heatChange;
		consumedHeat += heatChange;
		var heatLoss = corner.area * (corner.heat / corner.maxHeat) * 0.02;
		heatChange = Math.min(corner.airHeat, heatChange + heatLoss);
		
		var remainingCornerAirHeat = corner.airHeat - heatChange;
		corner.airHeat = 0;
		
		for (var j = 0; j < corner.corners.length; ++j)
		{
			var outflow = corner.airCurrentOutflows[j];
			if (outflow > 0)
			{
				corner.corners[j].newAirHeat += remainingCornerAirHeat * outflow;
				activeCorners.push(corner.corners[j]);
			}
		}
	}
	
	activeCorners.splice(0, activeCornerCount);
	
	for (var i = 0; i < activeCorners.length; ++i)
	{
		var corner = activeCorners[i];
		corner.airHeat = corner.newAirHeat;
	}
	for (var i = 0; i < activeCorners.length; ++i)
	{
		activeCorners[i].newAirHeat = 0;
	}
	
	return consumedHeat;
}

function calculateTemperature(corners, tiles, planetRadius, action)
{
	for (var i = 0; i < corners.length; ++i)
	{
		var corner = corners[i];
		var latitudeEffect = Math.sqrt(1 - Math.abs(corner.position.y) / planetRadius);
		var elevationEffect = 1 - Math.pow(Math.max(0, Math.min(corner.elevation * 0.8, 1)), 2);
		var normalizedHeat = corner.heat / corner.area;
		corner.temperature = (latitudeEffect * elevationEffect * 0.7 + normalizedHeat * 0.3) * 5/3 - 2/3;
		delete corner.airHeat;
		delete corner.newAirHeat;
		delete corner.heat;
		delete corner.maxHeat;
		delete corner.heatAbsorption;
	}

	for (var i = 0; i < tiles.length; ++i)
	{
		var tile = tiles[i];
		tile.temperature = 0;
		for (var j = 0; j < tile.corners.length; ++j)
		{
			tile.temperature += tile.corners[j].temperature;
		}
		tile.temperature /= tile.corners.length;
	}
}

function initializeAirMoisture(corners, moistureLevel, action)
{
	activeCorners = [];
	airMoisture = 0;
	for (var i = 0; i < corners.length; ++i)
	{
		var corner = corners[i];
		corner.airMoisture = (corner.elevation > 0) ? 0 : corner.area * moistureLevel * Math.max(0, Math.min(0.5 + corner.temperature * 0.5, 1));
		corner.newAirMoisture = 0;
		corner.precipitation = 0;

		corner.precipitationRate = 0.0075 * corner.area / Math.max(0.1, Math.min(corner.airCurrentSpeed, 1));
		corner.precipitationRate *= 1 + (1 - Math.max(0, Math.max(corner.temperature, 1))) * 0.1;
		if (corner.elevation > 0)
		{
			corner.precipitationRate *= 1 + corner.elevation * 0.5;
			corner.maxPrecipitation = corner.area * (0.25 + Math.max(0, Math.min(corner.elevation, 1)) * 0.25);
		}
		else
		{
			corner.maxPrecipitation = corner.area * 0.25;
		}
		
		activeCorners.push(corner);
		airMoisture += corner.airMoisture;
	}
	
	action.provideResult({ corners: activeCorners, airMoisture: airMoisture });
}

function processAirMoisture(activeCorners, action)
{
	var consumedMoisture = 0;
	var activeCornerCount = activeCorners.length;
	for (var i = 0; i < activeCornerCount; ++i)
	{
		var corner = activeCorners[i];
		if (corner.airMoisture === 0) continue;
		
		var moistureChange = Math.max(0, Math.min(corner.airMoisture, corner.precipitationRate * (1 - corner.precipitation / corner.maxPrecipitation)));
		corner.precipitation += moistureChange;
		consumedMoisture += moistureChange;
		var moistureLoss = corner.area * (corner.precipitation / corner.maxPrecipitation) * 0.02;
		moistureChange = Math.min(corner.airMoisture, moistureChange + moistureLoss);
		
		var remainingCornerAirMoisture = corner.airMoisture - moistureChange;
		corner.airMoisture = 0;
		
		for (var j = 0; j < corner.corners.length; ++j)
		{
			var outflow = corner.airCurrentOutflows[j];
			if (outflow > 0)
			{
				corner.corners[j].newAirMoisture += remainingCornerAirMoisture * outflow;
				activeCorners.push(corner.corners[j]);
			}
		}
	}

	activeCorners.splice(0, activeCornerCount);

	for (var i = 0; i < activeCorners.length; ++i)
	{
		var corner = activeCorners[i];
		corner.airMoisture = corner.newAirMoisture;
	}
	for (var i = 0; i < activeCorners.length; ++i)
	{
		activeCorners[i].newAirMoisture = 0;
	}
	
	return consumedMoisture;
}

function calculateMoisture(corners, tiles, action)
{
	for (var i = 0; i < corners.length; ++i)
	{
		var corner = corners[i];
		corner.moisture = corner.precipitation / corner.area / 0.5;
		delete corner.airMoisture;
		delete corner.newAirMoisture;
		delete corner.precipitation;
		delete corner.maxPrecipitation;
		delete corner.precipitationRate;
	}

	for (var i = 0; i < tiles.length; ++i)
	{
		var tile = tiles[i];
		tile.moisture = 0;
		for (var j = 0; j < tile.corners.length; ++j)
		{
			tile.moisture += tile.corners[j].moisture;
		}
		tile.moisture /= tile.corners.length;
	}
}

function generatePlanetStatistics(topology, plates, action)
{
	var statistics = {};
	
	var updateMinMaxAvg = function(stats, value) { stats.min = Math.min(stats.min, value); stats.max = Math.max(stats.max, value); stats.avg += value; };
	
	statistics.corners =
	{
		count: topology.corners.length,
		airCurrent: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		elevation: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		temperature: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		moisture: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		distanceToPlateBoundary: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		distanceToPlateRoot: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		pressure: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		shear: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		doublePlateBoundaryCount: 0,
		triplePlateBoundaryCount: 0,
		innerLandBoundaryCount: 0,
		outerLandBoundaryCount: 0,
	};
	
	for (var i = 0; i < topology.corners.length; ++i)
	{
		corner = topology.corners[i];
		updateMinMaxAvg(statistics.corners.airCurrent, corner.airCurrent.length());
		updateMinMaxAvg(statistics.corners.elevation, corner.elevation);
		updateMinMaxAvg(statistics.corners.temperature, corner.temperature);
		updateMinMaxAvg(statistics.corners.moisture, corner.moisture);
		updateMinMaxAvg(statistics.corners.distanceToPlateBoundary, corner.distanceToPlateBoundary);
		updateMinMaxAvg(statistics.corners.distanceToPlateRoot, corner.distanceToPlateRoot);
		if (corner.betweenPlates)
		{
			updateMinMaxAvg(statistics.corners.pressure, corner.pressure);
			updateMinMaxAvg(statistics.corners.shear, corner.shear);
			if (!corner.borders[0].betweenPlates || !corner.borders[1].betweenPlates || !corner.borders[2].betweenPlates)
			{
				statistics.corners.doublePlateBoundaryCount += 1;
			}
			else
			{
				statistics.corners.triplePlateBoundaryCount += 1;
			}
		}
		var landCount = ((corner.tiles[0].elevation > 0) ? 1 : 0) + ((corner.tiles[1].elevation > 0) ? 1 : 0) + ((corner.tiles[2].elevation > 0) ? 1 : 0);
		if (landCount === 2)
		{
			statistics.corners.innerLandBoundaryCount += 1;
		}
		else if (landCount === 1)
		{
			statistics.corners.outerLandBoundaryCount += 1;
		}
		if (corner.corners.length !== 3) throw "Corner has as invalid number of neighboring corners.";
		if (corner.borders.length !== 3) throw "Corner has as invalid number of borders.";
		if (corner.tiles.length !== 3) throw "Corner has as invalid number of tiles.";
	}
	
	statistics.corners.airCurrent.avg /= statistics.corners.count;
	statistics.corners.elevation.avg /= statistics.corners.count;
	statistics.corners.temperature.avg /= statistics.corners.count;
	statistics.corners.moisture.avg /= statistics.corners.count;
	statistics.corners.distanceToPlateBoundary.avg /= statistics.corners.count;
	statistics.corners.distanceToPlateRoot.avg /= statistics.corners.count;
	statistics.corners.pressure.avg /= (statistics.corners.doublePlateBoundaryCount + statistics.corners.triplePlateBoundaryCount);
	statistics.corners.shear.avg /= (statistics.corners.doublePlateBoundaryCount + statistics.corners.triplePlateBoundaryCount);

	statistics.borders =
	{
		count: topology.borders.length,
		length: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		plateBoundaryCount: 0,
		plateBoundaryPercentage: 0,
		landBoundaryCount: 0,
		landBoundaryPercentage: 0,
	};
	
	for (var i = 0; i < topology.borders.length; ++i)
	{
		border = topology.borders[i];
		var length = border.length();
		updateMinMaxAvg(statistics.borders.length, length);
		if (border.betweenPlates)
		{
			statistics.borders.plateBoundaryCount += 1;
			statistics.borders.plateBoundaryPercentage += length;
		}
		if (border.isLandBoundary())
		{
			statistics.borders.landBoundaryCount += 1;
			statistics.borders.landBoundaryPercentage += length;
		}
		if (border.corners.length !== 2) throw "Border has as invalid number of corners.";
		if (border.borders.length !== 4) throw "Border has as invalid number of neighboring borders.";
		if (border.tiles.length !== 2) throw "Border has as invalid number of tiles.";
	}

	statistics.borders.plateBoundaryPercentage /= statistics.borders.length.avg;
	statistics.borders.landBoundaryPercentage /= statistics.borders.length.avg;
	statistics.borders.length.avg /= statistics.borders.count;
	
	statistics.tiles =
	{
		count: topology.tiles.length,
		totalArea: 0,
		area: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		elevation: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		temperature: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		moisture: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		plateMovement: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		biomeCounts: {},
		biomeAreas: {},
		pentagonCount: 0,
		hexagonCount: 0,
		heptagonCount: 0,
	};
	
	for (var i = 0; i < topology.tiles.length; ++i)
	{
		var tile = topology.tiles[i];
		updateMinMaxAvg(statistics.tiles.area, tile.area);
		updateMinMaxAvg(statistics.tiles.elevation, tile.elevation);
		updateMinMaxAvg(statistics.tiles.temperature, tile.temperature);
		updateMinMaxAvg(statistics.tiles.moisture, tile.moisture);
		updateMinMaxAvg(statistics.tiles.plateMovement, tile.plateMovement.length());
		if (!statistics.tiles.biomeCounts[tile.biome]) statistics.tiles.biomeCounts[tile.biome] = 0;
		statistics.tiles.biomeCounts[tile.biome] += 1;
		if (!statistics.tiles.biomeAreas[tile.biome]) statistics.tiles.biomeAreas[tile.biome] = 0;
		statistics.tiles.biomeAreas[tile.biome] += tile.area;
		if (tile.tiles.length === 5) statistics.tiles.pentagonCount += 1;
		else if (tile.tiles.length === 6) statistics.tiles.hexagonCount += 1;
		else if (tile.tiles.length === 7) statistics.tiles.heptagonCount += 1;
		else throw "Tile has an invalid number of neighboring tiles.";
		if (tile.tiles.length !== tile.borders.length) throw "Tile has a neighbor and border count that do not match.";
		if (tile.tiles.length !== tile.corners.length) throw "Tile has a neighbor and corner count that do not match.";
	}

	statistics.tiles.totalArea = statistics.tiles.area.avg;
	statistics.tiles.area.avg /= statistics.tiles.count;
	statistics.tiles.elevation.avg /= statistics.tiles.count;
	statistics.tiles.temperature.avg /= statistics.tiles.count;
	statistics.tiles.moisture.avg /= statistics.tiles.count;
	statistics.tiles.plateMovement.avg /= statistics.tiles.count;
	
	statistics.plates =
	{
		count: plates.length,
		tileCount: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		area: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		boundaryElevation: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		boundaryBorders: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
		circumference: { min: Number.POSITIVE_INFINITY, max: Number.NEGATIVE_INFINITY, avg: 0 },
	};

	for (var i = 0; i < plates.length; ++i)
	{
		var plate = plates[i];
		updateMinMaxAvg(statistics.plates.tileCount, plate.tiles.length);
		plate.area = 0;
		for (var j = 0; j < plate.tiles.length; ++j)
		{
			var tile = plate.tiles[j];
			plate.area += tile.area;
		}
		updateMinMaxAvg(statistics.plates.area, plate.area);
		var elevation = 0;
		for (var j = 0; j < plate.boundaryCorners.length; ++j)
		{
			var corner = plate.boundaryCorners[j];
			elevation += corner.elevation;
		}
		updateMinMaxAvg(statistics.plates.boundaryElevation, elevation / plate.boundaryCorners.length);
		updateMinMaxAvg(statistics.plates.boundaryBorders, plate.boundaryBorders.length);
		plate.circumference = 0;
		for (var j = 0; j < plate.boundaryBorders.length; ++j)
		{
			var border = plate.boundaryBorders[j];
			plate.circumference += border.length();
		}
		updateMinMaxAvg(statistics.plates.circumference, plate.circumference);
	}

	statistics.plates.tileCount.avg /= statistics.plates.count;
	statistics.plates.area.avg /= statistics.plates.count;
	statistics.plates.boundaryElevation.avg /= statistics.plates.count;
	statistics.plates.boundaryBorders.avg /= statistics.plates.count;
	statistics.plates.circumference.avg /= statistics.plates.count;
	
	action.provideResult(statistics);
}

function SteppedAction(progressUpdater, unbrokenInterval, sleepInterval)
{
	this.callStack = null;
	this.subactions = [];
	this.finalizers = [];
	this.unbrokenInterval = (typeof(unbrokenInterval) === "number" && unbrokenInterval >= 0) ? unbrokenInterval : 16;
	this.sleepInterval = (typeof(sleepInterval) === "number" && sleepInterval >= 0) ? sleepInterval : 0;
	this.loopAction = false;
	this.started = false;
	this.canceled = false;
	this.completed = false;
	this.intervalIteration = 0; //number of times an unbroken interval has been completed
	this.stepIteration = 0; //number of times any of the stepper functions have been called
	this.intervalStepIteration = null; //number of times any of the stepper functions have been called during the current interval
	this.intervalStartTime = null; //begin time of the current interval
	this.intervalEndTime = null; //end time of the current interval
	this.progressUpdater = (typeof(progressUpdater) === "function") ? progressUpdater : null;
}

SteppedAction.prototype.execute = function SteppedAction_execute()
{
	if (!this.canceled && !this.completed && this.callStack === null && this.started === false)
	{
		this.started = true;
		if (this.subactions.length > 0)
		{
			this.beginSubactions(0, 1);
			if (this.progressUpdater !== null) this.progressUpdater(this);
			window.setTimeout(this.step.bind(this), this.sleepInterval);
		}
		else
		{
			this.completed = true;
		}
	}
	return this;
};

SteppedAction.prototype.step = function SteppedAction_step()
{
	this.intervalStartTime = Date.now();
	this.intervalEndTime = this.intervalStartTime + this.unbrokenInterval;
	this.intervalStepIteration = 0;
	while (Date.now() < this.intervalEndTime && !this.canceled && !this.completed)
	{
		var action = this.callStack.actions[this.callStack.index];
	
		this.callStack.loop = false;
		action.action(this);
		this.intervalStepIteration += 1;
		this.stepIteration += 1;
		
		if (this.subactions.length > 0)
		{
			this.beginSubactions(this.getProgress(), (this.callStack.loop) ? 0 : (1 - this.callStack.loopProgress) * action.proportion / this.callStack.proportionSum * this.callStack.parentProgressRange);
		}
		else
		{
			while (this.callStack !== null && this.callStack.loop === false && this.callStack.index === this.callStack.actions.length - 1)
			{
				for (var i = 0; i < this.callStack.finalizers.length; ++i)
				{
					this.callStack.finalizers[i](this);
				}
				this.callStack = this.callStack.parent;
			}
			if (this.callStack !== null)
			{
				if (this.callStack.loop === false)
				{
					this.callStack.loopProgress = 0;
					this.callStack.index += 1;
				}
			}
			else
			{
				this.completed = true;
			}
		}
	}
	this.intervalStartTime = null;
	this.intervalEndTime = null;
	this.intervalStepIteration = null;
	
	if (this.progressUpdater !== null) this.progressUpdater(this);

	this.intervalIteration += 1;
	if (this.canceled)
	{
		while (this.callStack !== null)
		{
			for (var i = 0; i < this.callStack.finalizers.length; ++i)
			{
				this.callStack.finalizers[i](this);
			}
			this.callStack = this.callStack.parent;
		}
	}
	else if (!this.completed)
	{
		window.setTimeout(this.step.bind(this), this.sleepInterval);
	}
};

SteppedAction.prototype.beginSubactions = function(parentProgress, parentProgressRange)
{
	this.callStack = {
		actions: this.subactions,
		finalizers: this.finalizers,
		proportionSum: accumulateArray(this.subactions, 0, function(sum, subaction) { return sum + subaction.proportion; }),
		index: 0,
		loop: false,
		loopProgress: 0,
		parent: this.callStack,
		parentProgress: parentProgress,
		parentProgressRange : parentProgressRange,
	};
	this.subactions = [];
	this.finalizers = [];
};

SteppedAction.prototype.cancel = function SteppedAction_cancel()
{
	this.canceled = true;
};

SteppedAction.prototype.provideResult = function SteppedAction_provideResult(resultProvider)
{
	this.callStack.resultProvider = resultProvider;
};

SteppedAction.prototype.loop = function SteppedAction_loop(progress)
{
	this.callStack.loop = true;
	if (typeof(progress) === "number" && progress >= 0 && progress < 1)
	{
		this.callStack.loopProgress = progress;
	}
};

SteppedAction.prototype.executeSubaction = function SteppedAction_executeSubaction(subaction, proportion, name)
{
	proportion = (typeof(proportion) === "number" && proportion >= 0) ? proportion : 1;
	this.subactions.push({ action: subaction, proportion: proportion, name: name });
	return this;
};

SteppedAction.prototype.getResult = function SteppedAction_getResult(recipient)
{
	this.subactions.push({
		action: function(action)
		{
			var resultProvider = action.callStack.resultProvider;
			var resultProviderType = typeof(resultProvider);
			if (resultProviderType === "function")
				recipient(resultProvider());
			else if (resultProviderType !== "undefined")
				recipient(resultProvider);
			else
				recipient();
		},
		proportion: 0,
	});
	return this;
};

SteppedAction.prototype.finalize = function SteppedAction_finalize(finalizer)
{
	this.finalizers.push(finalizer);
	return this;
};

SteppedAction.prototype.getTimeRemainingInInterval = function SteppedAction_getTimeRemainingInInterval()
{
	if (this.intervalEndTime !== null)
	{
		return Math.max(0, this.intervalEndTime - Date.now());
	}
	else
	{
		return 0;
	}
};

SteppedAction.prototype.getProgress = function SteppedAction_getProgress()
{
	if (this.callStack !== null)
	{
		if (this.callStack.proportionSum === 0) return this.callStack.parentProgress;

		var currentProportionSum = 0;
		for (var i = 0; i < this.callStack.index; ++i)
		{
			currentProportionSum += this.callStack.actions[i].proportion;
		}
		currentProportionSum += this.callStack.loopProgress * this.callStack.actions[this.callStack.index].proportion;
		return this.callStack.parentProgress + currentProportionSum / this.callStack.proportionSum * this.callStack.parentProgressRange;
	}
	else
	{
		return this.completed ? 1 : 0;
	}
};

SteppedAction.prototype.getCurrentActionName = function SteppedAction_getCurrentActionName()
{
	var callStack = this.callStack;
	while (callStack !== null)
	{
		var action = callStack.actions[callStack.index];
		if (typeof(action.name) === "string") return action.name;
		callStack = callStack.parent;
	}
	
	return "";
};

var lastRenderFrameTime = null;

function getZoomDelta()
{
	var zoomIn = (pressedKeys[KEY_NUMPAD_PLUS] || pressedKeys[KEY_PAGEUP]);
	var zoomOut = (pressedKeys[KEY_NUMPAD_MINUS] || pressedKeys[KEY_PAGEDOWN]);
	if (zoomIn && !zoomOut) return -1;
	if (zoomOut && !zoomIn) return +1;
	return 0;
}

function getLatitudeDelta()
{
	var up = (pressedKeys[KEY.W] || pressedKeys[KEY.Z] || pressedKeys[KEY_UPARROW]);
	var down = (pressedKeys[KEY.S] || pressedKeys[KEY_DOWNARROW]);
	if (up && !down) return +1;
	if (down && !up) return -1;
	return 0;
}

function getLongitudeDelta()
{
	var left = (pressedKeys[KEY.A] || pressedKeys[KEY.Q] || pressedKeys[KEY_LEFTARROW]);
	var right = (pressedKeys[KEY.D] || pressedKeys[KEY_RIGHTARROW]);
	if (right && !left) return +1;
	if (left && !right) return -1;
	return 0;
}

function Corner(id, position, cornerCount, borderCount, tileCount)
{
	this.id = id;
	this.position = position;
	this.corners = new Array(cornerCount);
	this.borders = new Array(borderCount);
	this.tiles = new Array(tileCount);
}

Corner.prototype.vectorTo = function Corner_vectorTo(corner)
{
	return corner.position.clone().sub(this.position);
};

Corner.prototype.toString = function Corner_toString()
{
	return "Corner " + this.id.toFixed(0) + " < " + this.position.x.toFixed(0) + ", " + this.position.y.toFixed(0) + ", " + this.position.z.toFixed(0) + " >";
};

function Border(id, cornerCount, borderCount, tileCount)
{
	this.id = id;
	this.corners = new Array(cornerCount);
	this.borders = new Array(borderCount);
	this.tiles = new Array(tileCount);
}

Border.prototype.oppositeCorner = function Border_oppositeCorner(corner)
{
	return (this.corners[0] === corner) ? this.corners[1] : this.corners[0];
};

Border.prototype.oppositeTile = function Border_oppositeTile(tile)
{
	return (this.tiles[0] === tile) ? this.tiles[1] : this.tiles[0];
};

Border.prototype.length = function Border_length()
{
	return this.corners[0].position.distanceTo(this.corners[1].position);
};

Border.prototype.isLandBoundary = function Border_isLandBoundary()
{
	return (this.tiles[0].elevation > 0) !== (this.tiles[1].elevation > 0);
};

Border.prototype.toString = function Border_toString()
{
	return "Border " + this.id.toFixed(0);
};

function Tile(id, position, cornerCount, borderCount, tileCount)
{
	this.id = id;
	this.position = position;
	this.corners = new Array(cornerCount);
	this.borders = new Array(borderCount);
	this.tiles = new Array(tileCount);
}

Tile.prototype.intersectRay = function Tile_intersectRay(ray)
{
	if (!intersectRayWithSphere(ray, this.boundingSphere)) return false;

	var surface = new THREE.Plane().setFromNormalAndCoplanarPoint(this.normal, this.averagePosition);
	if (surface.distanceToPoint(ray.origin) <= 0) return false;

	var denominator = surface.normal.dot(ray.direction);
	if (denominator === 0) return false;

	var t = -(ray.origin.dot(surface.normal) + surface.constant) / denominator;
	var point = ray.direction.clone().multiplyScalar(t).add(ray.origin);
	
	var origin = new Vector3(0, 0, 0);
	for (var i = 0; i < this.corners.length; ++i)
	{
		var j = (i + 1) % this.corners.length;
		var side = new THREE.Plane().setFromCoplanarPoints(this.corners[j].position, this.corners[i].position, origin);

		if (side.distanceToPoint(point) < 0) return false;
	}
	
	return true;
};

Tile.prototype.toString = function Tile_toString()
{
	return "Tile " + this.id.toFixed(0) + " (" + this.tiles.length.toFixed(0) + " Neighbors) < " + this.position.x.toFixed(0) + ", " + this.position.y.toFixed(0) + ", " + this.position.z.toFixed(0) + " >";
};

function Plate(color, driftAxis, driftRate, spinRate, elevation, oceanic, root)
{
	this.color = color;
	this.driftAxis = driftAxis;
	this.driftRate = driftRate;
	this.spinRate = spinRate;
	this.elevation = elevation;
	this.oceanic = oceanic;
	this.root = root;
	this.tiles = [];
	this.boundaryCorners = [];
	this.boundaryBorders = [];
}

Plate.prototype.calculateMovement = function Plate_calculateMovement(position)
{
	var movement = this.driftAxis.clone().cross(position).setLength(this.driftRate * position.clone().projectOnVector(this.driftAxis).distanceTo(position));
	movement.add(this.root.position.clone().cross(position).setLength(this.spinRate * position.clone().projectOnVector(this.root.position).distanceTo(position)));
	return movement;
};

function SpatialPartition(boundingSphere, partitions, tiles)
{
	this.boundingSphere = boundingSphere;
	this.partitions = partitions;
	this.tiles = tiles;
}

SpatialPartition.prototype.intersectRay = function SpatialPartition_intersectRay(ray)
{
	if (intersectRayWithSphere(ray, this.boundingSphere))
	{
		for (var i = 0; i < this.partitions.length; ++i)
		{
			var intersection = this.partitions[i].intersectRay(ray);
			if (intersection !== false)
			{
				return intersection;
			}
		}

		for (var i = 0; i < this.tiles.length; ++i)
		{
			if (this.tiles[i].intersectRay(ray))
			{
				return this.tiles[i];
			}
		}
	}

	return false;
};