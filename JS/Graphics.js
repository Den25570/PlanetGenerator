function distortMesh(mesh, degree, random, action)
{
	var totalSurfaceArea = 4 * Math.PI;
	var idealFaceArea = totalSurfaceArea / mesh.faces.length;
	var idealEdgeLength = Math.sqrt(idealFaceArea * 4 / Math.sqrt(3));
	var idealFaceHeight = idealEdgeLength * Math.sqrt(3) / 2;

	var rotationPredicate = function(oldNode0, oldNode1, newNode0, newNode1)
	{
		if (newNode0.f.length >= 7 ||
			newNode1.f.length >= 7 ||
			oldNode0.f.length <= 5 ||
			oldNode1.f.length <= 5) return false;
		var oldEdgeLength = oldNode0.p.distanceTo(oldNode1.p);
		var newEdgeLength = newNode0.p.distanceTo(newNode1.p);
		var ratio = oldEdgeLength / newEdgeLength;
		if (ratio >= 2 || ratio <= 0.5) return false;
		var v0 = oldNode1.p.clone().sub(oldNode0.p).divideScalar(oldEdgeLength);
		var v1 = newNode0.p.clone().sub(oldNode0.p).normalize();
		var v2 = newNode1.p.clone().sub(oldNode0.p).normalize();
		if (v0.dot(v1) < 0.2 || v0.dot(v2) < 0.2) return false;
		v0.negate();
		var v3 = newNode0.p.clone().sub(oldNode1.p).normalize();
		var v4 = newNode1.p.clone().sub(oldNode1.p).normalize();
		if (v0.dot(v3) < 0.2 || v0.dot(v4) < 0.2) return false;
		return true;
	};
	
	var i = 0;
	action.executeSubaction(function(action)
	{
		if (i >= degree) return;
		
		var consecutiveFailedAttempts = 0;
		var edgeIndex = random.integerExclusive(0, mesh.edges.length);
		while (!conditionalRotateEdge(mesh, edgeIndex, rotationPredicate))
		{
			if (++consecutiveFailedAttempts >= mesh.edges.length) return false;
			edgeIndex = (edgeIndex + 1) % mesh.edges.length;
		}
		
		++i;
		action.loop(i / degree);
	});

	return true;
}

function relaxMesh(mesh, multiplier, action)
{
	var totalSurfaceArea = 4 * Math.PI;
	var idealFaceArea = totalSurfaceArea / mesh.faces.length;
	var idealEdgeLength = Math.sqrt(idealFaceArea * 4 / Math.sqrt(3));
	var idealDistanceToCentroid = idealEdgeLength * Math.sqrt(3) / 3 * 0.9;
	
	var pointShifts = new Array(mesh.nodes.length);
	action.executeSubaction(function(action)
	{
		for (var i = 0; i < mesh.nodes.length; ++i)
			pointShifts[i] = new Vector3(0, 0, 0);
	}, 1);

	var i = 0;
	action.executeSubaction(function(action)
	{
		if (i >= mesh.faces.length) return;

		var face = mesh.faces[i];
		var n0 = mesh.nodes[face.n[0]];
		var n1 = mesh.nodes[face.n[1]];
		var n2 = mesh.nodes[face.n[2]];
		var p0 = n0.p;
		var p1 = n1.p;
		var p2 = n2.p;
		var e0 = p1.distanceTo(p0) / idealEdgeLength;
		var e1 = p2.distanceTo(p1) / idealEdgeLength;
		var e2 = p0.distanceTo(p2) / idealEdgeLength;
		var centroid = calculateFaceCentroid(p0, p1, p2).normalize();
		var v0 = centroid.clone().sub(p0);
		var v1 = centroid.clone().sub(p1);
		var v2 = centroid.clone().sub(p2);
		var length0 = v0.length();
		var length1 = v1.length();
		var length2 = v2.length();
		v0.multiplyScalar(multiplier * (length0 - idealDistanceToCentroid) / length0);
		v1.multiplyScalar(multiplier * (length1 - idealDistanceToCentroid) / length1);
		v2.multiplyScalar(multiplier * (length2 - idealDistanceToCentroid) / length2);
		pointShifts[face.n[0]].add(v0);
		pointShifts[face.n[1]].add(v1);
		pointShifts[face.n[2]].add(v2);
		
		++i;
		action.loop(i / mesh.faces.length);
	}, mesh.faces.length);
	
	var origin = new Vector3(0, 0, 0);
	var plane = new THREE.Plane();
	action.executeSubaction(function(action)
	{
		for (var i = 0; i < mesh.nodes.length; ++i)
		{
			plane.setFromNormalAndCoplanarPoint(mesh.nodes[i].p, origin);
			pointShifts[i] = mesh.nodes[i].p.clone().add(plane.projectPoint(pointShifts[i])).normalize();
		}
	}, mesh.nodes.length / 10);

	var rotationSupressions = new Array(mesh.nodes.length);
	for (var i = 0; i < mesh.nodes.length; ++i)
		rotationSupressions[i] = 0;
	
	var i = 0;
	action.executeSubaction(function(action)
	{
		if (i >= mesh.edges.length) return;

		var edge = mesh.edges[i];
		var oldPoint0 = mesh.nodes[edge.n[0]].p;
		var oldPoint1 = mesh.nodes[edge.n[1]].p;
		var newPoint0 = pointShifts[edge.n[0]];
		var newPoint1 = pointShifts[edge.n[1]];
		var oldVector = oldPoint1.clone().sub(oldPoint0).normalize();
		var newVector = newPoint1.clone().sub(newPoint0).normalize();
		var suppression = (1 - oldVector.dot(newVector)) * 0.5;
		rotationSupressions[edge.n[0]] = Math.max(rotationSupressions[edge.n[0]], suppression);
		rotationSupressions[edge.n[1]] = Math.max(rotationSupressions[edge.n[1]], suppression);
		
		++i;
		action.loop(i / mesh.edges.length);
	});
	
	var totalShift = 0;
	action.executeSubaction(function(action)
	{
		for (var i = 0; i < mesh.nodes.length; ++i)
		{
			var node = mesh.nodes[i];
			var point = node.p;
			var delta = point.clone();
			point.lerp(pointShifts[i], 1 - Math.sqrt(rotationSupressions[i])).normalize();
			delta.sub(point);
			totalShift += delta.length();
		}
	}, mesh.nodes.length / 20);
	
	return totalShift;
}

function updateProgressUI(action)
{
	var progress = action.getProgress();
	ui.progressBar.css("width", (progress * 100).toFixed(0) + "%");
	ui.progressBarLabel.text((progress * 100).toFixed(0) + "%");
	ui.progressActionLabel.text(action.getCurrentActionName());
}

function projectMap(renderData, globeColorArrayKeys, project)
{
	var mapGeometry = new THREE.Geometry();
	var globeGeometry = renderData.geometry;
	
	if (globeColorArrayKeys)
	{
		renderData.mapColorArrays = {};
		for (var i = 0; i < globeColorArrayKeys.length; ++i)
		{
			var globeColorArray = renderData[globeColorArrayKeys[i]];
			var mapColorArray = [];
			for (var j = 0; j < globeColorArray.length; ++j)
			{
				mapColorArray.push(globeColorArray[j].slice());
			}
			renderData.mapColorArrays[globeColorArrayKeys[i]] = mapColorArray;
		}
	}

	var transformation = new THREE.Matrix4().makeRotationFromEuler(new THREE.Euler(-cameraLatitude, -cameraLongitude, 0, "XYZ"));
	
	for (var i = 0; i < globeGeometry.vertices.length; ++i)
	{
		var globePosition = globeGeometry.vertices[i].clone();
		globePosition.applyMatrix4(transformation);
		globePosition.multiplyScalar(0.001);
		var longitude = Math.atan2(globePosition.x, globePosition.z);
		var xSquared = globePosition.x * globePosition.x;
		var ySquared = globePosition.y * globePosition.y;
		var zSquared = globePosition.z * globePosition.z;
		var latitude = -Math.asin(globePosition.y);
		mapGeometry.vertices.push(project(longitude, latitude, Math.sqrt(xSquared + ySquared + zSquared) - 1));
	}
	
	if (globeColorArrayKeys)
	{
		for (var i = 0; i < globeGeometry.faces.length; ++i)
		{
			var globeFace = globeGeometry.faces[i];
			mapGeometry.faces.push(new THREE.Face3(globeFace.a, globeFace.b, globeFace.c, globeFace.normal));
		}
	}
	else
	{
		for (var i = 0; i < globeGeometry.faces.length; ++i)
		{
			var globeFace = globeGeometry.faces[i];
			mapGeometry.faces.push(new THREE.Face3(globeFace.a, globeFace.b, globeFace.c, globeFace.normal, globeFace.vertexColors));
		}
	}
	
	var checkForWrap = function checkForWrap(a, b)
	{
		if (a < -0.5 && b > 0.5) return 2;
		else if (b < -0.5 && a > 0.5) return -2;
		else return 0;
	};
	
	var mergeWrap = function checkForWrap(a, b)
	{
		if (a !== 0) return a;
		else if (b !== 0) return b;
		else return 0;
	};
	
	var addVertex = function addVertex(x, y, z, oldIndex)
	{
		var index = mapGeometry.vertices.length;
		mapGeometry.vertices.push(new Vector3(x, y, z));
		return index;
	};
	
	var addFace;
	if (globeColorArrayKeys)
	{
		addFace = function addFace(faceIndex, vertexIndex0, vertexIndex1, vertexIndex2)
		{
			mapGeometry.faces.push(new THREE.Face3(vertexIndex0, vertexIndex1, vertexIndex2, globeGeometry.faces[faceIndex].normal));
			for (var i = 0; i < globeColorArrayKeys.length; ++i)
			{
				var key = globeColorArrayKeys[i];
				renderData.mapColorArrays[key].push(renderData[globeColorArrayKeys[i]][faceIndex].slice());
			}
		};
	}
	else
	{
		addFace = function addFace(faceIndex, vertexIndex0, vertexIndex1, vertexIndex2)
		{
			mapGeometry.faces.push(new THREE.Face3(vertexIndex0, vertexIndex1, vertexIndex2, globeGeometry.faces[faceIndex].normal, globeGeometry.faces[faceIndex].vertexColors));
		};
	}
	
	var cornerWrap = function cornerWrap(wrap0, wrap1)
	{
		var delta = wrap0 - wrap1;
		var sum = wrap0 + wrap1;
		return delta * delta / sum;
	};
	
	for (var i = 0; i < globeGeometry.faces.length; ++i)
	{
		var face = mapGeometry.faces[i];
		var p0 = mapGeometry.vertices[face.a];
		var p1 = mapGeometry.vertices[face.b];
		var p2 = mapGeometry.vertices[face.c];
		
		var xWrap01 = checkForWrap(p0.x, p1.x);
		var yWrap01 = checkForWrap(p0.y, p1.y);
		var xWrap02 = checkForWrap(p0.x, p2.x);
		var yWrap02 = checkForWrap(p0.y, p2.y);
		var xWrap = mergeWrap(xWrap01, xWrap02);
		var yWrap = mergeWrap(yWrap01, yWrap02);
		
		if (xWrap !== 0)
		{
			if (yWrap !== 0)
			{
				var xWrap12 = xWrap02 - xWrap01;
				var yWrap12 = yWrap02 - yWrap01;
				
				addFace(i,
					addVertex(p0.x + xWrap01, p0.y + yWrap01, p0.z),
					face.b,
					addVertex(p2.x - xWrap12, p2.y - yWrap12, p2.z));
				
				addFace(i,
					addVertex(p0.x + xWrap02, p0.y + yWrap02, p0.z),
					addVertex(p1.x + xWrap12, p1.y + yWrap12, p1.z),
					face.c);
				
				addFace(i,
					addVertex(p0.x + cornerWrap(xWrap01, xWrap02), p0.y + cornerWrap(xWrap01, xWrap02), p0.z),
					addVertex(p1.x + cornerWrap(xWrap12, -xWrap01), p1.y + cornerWrap(xWrap12, -xWrap01), p1.z),
					addVertex(p2.x + cornerWrap(-xWrap02, -xWrap12), p2.y + cornerWrap(-xWrap02, -xWrap12), p2.z));
				
				face.b = addVertex(p1.x - xWrap01, p1.y - yWrap01, p1.z);
				face.c = addVertex(p2.x - xWrap02, p2.y - yWrap02, p2.z);
			}
			else
			{
				if (xWrap01 !== 0)
				{
					if (xWrap02 !== 0)
					{
						addFace(i,
							face.a,
							addVertex(p1.x - xWrap, p1.y, p1.z),
							addVertex(p2.x - xWrap, p2.y, p2.z));
						face.a = addVertex(p0.x + xWrap, p0.y, p0.z);
					}
					else
					{
						addFace(i,
							addVertex(p0.x + xWrap, p0.y, p0.z),
							face.b,
							addVertex(p2.x + xWrap, p2.y, p2.z));
						face.b = addVertex(p1.x - xWrap, p1.y, p1.z);
					}
				}
				else if (xWrap02 !== 0)
				{
					addFace(i,
						addVertex(p0.x + xWrap, p0.y, p0.z),
						addVertex(p1.x + xWrap, p1.y, p1.z),
						face.c);
					face.c = addVertex(p2.x - xWrap, p2.y, p2.z);
				}
			}
		}
		else if (yWrap !== 0)
		{
			if (yWrap01 !== 0)
			{
				if (yWrap02 !== 0)
				{
					addFace(i,
						face.a,
						addVertex(p1.x, p1.y - yWrap, p1.z),
						addVertex(p2.x, p2.y - yWrap, p2.z));
					face.a = addVertex(p0.x, p0.y + yWrap, p0.z);
				}
				else
				{
					addFace(i,
						addVertex(p0.x, p0.y + yWrap, p0.z),
						face.b,
						addVertex(p2.x, p2.y + yWrap, p2.z));
					face.b = addVertex(p1.x, p1.y - yWrap, p1.z);
				}
			}
			else if (yWrap02 !== 0)
			{
				addFace(i,
					addVertex(p0.x, p0.y + yWrap, p0.z),
					addVertex(p1.x, p1.y + yWrap, p1.z),
					face.c);
				face.c = addVertex(p2.x, p2.y - yWrap, p2.z);
			}
		}
	}
	
	if (renderData.mapRenderObject && renderData.mapRenderObject.parent) renderData.mapRenderObject.parent.remove(renderData.mapRenderObject);
	
	renderData.mapGeometry = mapGeometry;
	renderData.mapRenderObject = new THREE.Mesh(renderData.mapGeometry, renderData.mapMaterial);
}

function setProjectionRenderMode(mode, force)
{
	if (mode !== projectionRenderMode || force === true)
	{
		$("#projectionDisplayList>button").removeClass("toggled");
		ui.projectionDisplayButtons[mode].addClass("toggled");
		
		if (!planet) return;

		if (projectionRenderMode === "globe")
		{
			scene.remove(planet.renderData.surface.renderObject);
		}
		else
		{
			scene.remove(planet.renderData.surface.mapRenderObject);
		}

		projectionRenderMode = mode;
		
		if (mode === "globe")
		{
			scene.add(planet.renderData.surface.renderObject);
			setSurfaceRenderMode(surfaceRenderMode, true);
		}
		else
		{
			renderMap();
		}
	}
}

function showHideSunlight(show)
{
	if (typeof(show) === "boolean") renderSunlight = show;
	else renderSunlight = !renderSunlight;
	if (renderSunlight) ui.showSunlightButton.addClass("toggled");
	if (!renderSunlight) ui.showSunlightButton.removeClass("toggled");

	if (!planet) return;
	
	var material = planet.renderData.surface.material;
	if (renderSunlight)
	{
		material.color = new THREE.Color(0xFFFFFF);
		material.ambient = new THREE.Color(0x444444);
	}
	else
	{
		material.color = new THREE.Color(0x000000);
		material.ambient = new THREE.Color(0xFFFFFF);
	}
	material.needsUpdate = true;
}

function showHidePlateBoundaries(show)
{
	if (typeof(show) === "boolean") renderPlateBoundaries = show;
	else renderPlateBoundaries = !renderPlateBoundaries;
	if (renderPlateBoundaries) ui.showPlateBoundariesButton.addClass("toggled");
	if (!renderPlateBoundaries) ui.showPlateBoundariesButton.removeClass("toggled");

	if (!planet) return;
	
	if (renderPlateBoundaries)
	{
		planet.renderData.surface.renderObject.add(planet.renderData.plateBoundaries.renderObject);
		planet.renderData.surface.mapRenderObject.add(planet.renderData.plateBoundaries.mapRenderObject);
	}
	else
	{
		planet.renderData.surface.renderObject.remove(planet.renderData.plateBoundaries.renderObject);
		planet.renderData.surface.mapRenderObject.remove(planet.renderData.plateBoundaries.mapRenderObject);
	}
}

function showHidePlateMovements(show)
{
	if (typeof(show) === "boolean") renderPlateMovements = show;
	else renderPlateMovements = !renderPlateMovements;
	if (renderPlateMovements) ui.showPlateMovementsButton.addClass("toggled");
	if (!renderPlateMovements) ui.showPlateMovementsButton.removeClass("toggled");

	if (!planet) return;
	
	if (renderPlateMovements)
	{
		planet.renderData.surface.renderObject.add(planet.renderData.plateMovements.renderObject);
		planet.renderData.surface.mapRenderObject.add(planet.renderData.plateMovements.mapRenderObject);
	}
	else
	{
		planet.renderData.surface.renderObject.remove(planet.renderData.plateMovements.renderObject);
		planet.renderData.surface.mapRenderObject.remove(planet.renderData.plateMovements.mapRenderObject);
	}
}

function showHideAirCurrents(show)
{
	if (typeof(show) === "boolean") renderAirCurrents = show;
	else renderAirCurrents = !renderAirCurrents;
	if (renderAirCurrents) ui.showAirCurrentsButton.addClass("toggled");
	if (!renderAirCurrents) ui.showAirCurrentsButton.removeClass("toggled");

	if (!planet) return;
	
	if (renderAirCurrents)
	{
		planet.renderData.surface.renderObject.add(planet.renderData.airCurrents.renderObject);
		planet.renderData.surface.mapRenderObject.add(planet.renderData.airCurrents.mapRenderObject);
	}
	else
	{
		planet.renderData.surface.renderObject.remove(planet.renderData.airCurrents.renderObject);
		planet.renderData.surface.mapRenderObject.remove(planet.renderData.airCurrents.mapRenderObject);
	}
}

function serializePlanetMesh(mesh, prefix, suffix)
{
	var stringPieces = [];
	
	stringPieces.push(prefix, "{nodes:[");
	for (var i = 0; i < mesh.nodes.length; ++i)
	{
		var node = mesh.nodes[i];
		stringPieces.push(i !== 0 ? ",\n{p:new THREE.Vector3(" : "\n{p:new THREE.Vector3(", node.p.x.toString(), ",", node.p.y.toString(), ",", node.p.z.toString(), "),e:[", node.e[0].toFixed(0));
		for (var j = 1; j < node.e.length; ++j) stringPieces.push(",", node.e[j].toFixed(0));
		stringPieces.push("],f:[", node.f[0].toFixed(0));
		for (var j = 1; j < node.f.length; ++j) stringPieces.push(",", node.f[j].toFixed(0));
		stringPieces.push("]}");
	}
	stringPieces.push("\n],edges:[");
	for (var i = 0; i < mesh.edges.length; ++i)
	{
		var edge = mesh.edges[i];
		stringPieces.push(i !== 0 ? ",\n{n:[" : "\n{n:[", edge.n[0].toFixed(0), ",", edge.n[1].toFixed(0), "],f:[", edge.f[0].toFixed(0), ",", edge.f[1].toFixed(0), "]}");
	}
	stringPieces.push("\n],faces:[");
	for (var i = 0; i < mesh.faces.length; ++i)
	{
		var face = mesh.faces[i];
		stringPieces.push(i !== 0 ? ",\n{n:[" : "\n{n:[", face.n[0].toFixed(0), ",", face.n[1].toFixed(0), ",", face.n[2].toFixed(0), "],e:[", face.e[0].toFixed(0), ",", face.e[1].toFixed(0), ",", face.e[2].toFixed(0), "]}");
	}
	stringPieces.push("\n]}", suffix);
	
	return stringPieces.join("");
}

function displayPlanet(newPlanet)
{
	if (planet)
	{
		tileSelection = null;
		scene.remove(planet.renderData.surface.renderObject);
		scene.remove(planet.renderData.surface.mapRenderObject);
	}
	else
	{
		sunTimeOffset = Math.PI * 2 * (1/12 - Date.now() / 60000);
	}

	planet = newPlanet;
	scene.add(planet.renderData.surface.renderObject);
	
	setProjectionRenderMode(projectionRenderMode, true);
	setSurfaceRenderMode(surfaceRenderMode, true);
	showHideSunlight(renderSunlight);
	showHidePlateBoundaries(renderPlateBoundaries);
	showHidePlateMovements(renderPlateMovements);
	showHideAirCurrents(renderAirCurrents);

	updateCamera();
	updateUI();
	
	console.log("Original Seed", planet.originalSeed);
	console.log("Raw Seed", planet.seed);
	console.log("Statistics", planet.statistics);
}