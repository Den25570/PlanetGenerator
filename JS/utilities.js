////////////////////////////////////////////////////////////////////////////////
// UTILITIES                                                                  //
////////////////////////////////////////////////////////////////////////////////

function Signal()
{
	this.nextToken = 1;
	this.listeners = {};
}

Signal.prototype.addListener = function Signal_addListener(callback, token)
{
	if (typeof(token) !== "string")
	{
		token = this.nextToken.toFixed(0);
		this.nextToken += 1;
	}
	this.listeners[token] = callback;
};

Signal.prototype.removeListener = function Signal_removeListener(token)
{
	delete this.listeners[token];
};

Signal.prototype.fire = function Signal_fire()
{
	for (var key in this.listeners)
	{
		if (this.listeners.hasOwnProperty(key))
		{
			this.listeners[key].apply(null, arguments);
		}
	}
};

function XorShift128(x, y, z, w)
{
	this.x = (x ? x >>> 0 : 123456789);
	this.y = (y ? y >>> 0 : 362436069);
	this.z = (z ? z >>> 0 : 521288629);
	this.w = (w ? w >>> 0 : 88675123);
}

XorShift128.prototype.next = function XorShift128_next()
{
	var t = this.x ^ (this.x << 11) & 0x7FFFFFFF;
	this.x = this.y;
	this.y = this.z;
	this.z = this.w;
	this.w = (this.w ^ (this.w >> 19)) ^ (t ^ (t >> 8));
	return this.w;
};

XorShift128.prototype.unit = function XorShift128_unit()
{
	return this.next() / 0x80000000;
};

XorShift128.prototype.unitInclusive = function XorShift128_unitInclusive()
{
	return this.next() / 0x7FFFFFFF;
};

XorShift128.prototype.integer = function XorShift128_integer(min, max)
{
	return this.integerExclusive(min, max + 1);
};

XorShift128.prototype.integerExclusive = function XorShift128_integerExclusive(min, max)
{
	min = Math.floor(min);
	max = Math.floor(max);
	return Math.floor(this.unit() * (max - min)) + min;
};

XorShift128.prototype.real = function XorShift128_real(min, max)
{
	return this.unit() * (max - min) + min;
};

XorShift128.prototype.realInclusive = function XorShift128_realInclusive(min, max)
{
	return this.unitInclusive() * (max - min) + min;
};

XorShift128.prototype.reseed = function XorShift128_reseed(x, y, z, w)
{
	this.x = (x ? x >>> 0 : 123456789);
	this.y = (y ? y >>> 0 : 362436069);
	this.z = (z ? z >>> 0 : 521288629);
	this.w = (w ? w >>> 0 : 88675123);
};

function saveToFileSystem(content)
{
	var requestFileSystem = window.requestFileSystem || window.webkitRequestFileSystem;

	requestFileSystem(window.TEMPORARY, content.length,
		function(fs)
		{
			fs.root.getFile("planetMesh.js", { create: true },
				function (fileEntry)
				{
					fileEntry.createWriter(
						function (fileWriter)
						{
							fileWriter.addEventListener("writeend",
								function()
								{
									$("body").append("<a href=\"" + fileEntry.toURL() + "\" download=\"planetMesh.js\" target=\"_blank\">Mesh Data</a>");
									$("body>a").focus();
								}, false);

							fileWriter.write(new Blob([content]));
						},
						function (error) {});
				},
				function (error) {});
		},
		function (error) {});
}

function slerp(p0, p1, t)
{
	var omega = Math.acos(p0.dot(p1));
	return p0.clone().multiplyScalar(Math.sin((1 - t) * omega)).add(p1.clone().multiplyScalar(Math.sin(t * omega))).divideScalar(Math.sin(omega));
}

function randomUnitVector(random)
{
	var theta = random.real(0, Math.PI * 2);
	var phi = Math.acos(random.realInclusive(-1, 1));
	var sinPhi = Math.sin(phi);
	return new Vector3(
		Math.cos(theta) * sinPhi,
		Math.sin(theta) * sinPhi,
		Math.cos(phi));
}

function randomQuaternion(random)
{
	var theta = random.real(0, Math.PI * 2);
	var phi = Math.acos(random.realInclusive(-1, 1));
	var sinPhi = Math.sin(phi);
	var gamma = random.real(0, Math.PI * 2);
	var sinGamma = Math.sin(gamma);
	return new Quaternion(
		Math.cos(theta) * sinPhi * sinGamma,
		Math.sin(theta) * sinPhi * sinGamma,
		Math.cos(phi) * sinGamma,
		Math.cos(gamma));
}

function intersectRayWithSphere(ray, sphere)
{
	var v1 = sphere.center.clone().sub(ray.origin);
	var v2 = v1.clone().projectOnVector(ray.direction);
	var d = v1.distanceTo(v2);
	return (d <= sphere.radius);
}

function calculateTriangleArea(pa, pb, pc)
{
	var vab = new THREE.Vector3().subVectors(pb, pa);
	var vac = new THREE.Vector3().subVectors(pc, pa);
	var faceNormal = new THREE.Vector3().crossVectors(vab, vac);
	var vabNormal = new THREE.Vector3().crossVectors(faceNormal, vab).normalize();
	var plane = new THREE.Plane().setFromNormalAndCoplanarPoint(vabNormal, pa);
	var height = plane.distanceToPoint(pc);
	var width = vab.length();
	var area = width * height * 0.5;
	return area;
}

function accumulateArray(array, state, accumulator)
{
	for (var i = 0; i < array.length; ++i)
	{
		state = accumulator(state, array[i]);
	}
	return state;
}

function adjustRange(value, oldMin, oldMax, newMin, newMax)
{
	return (value - oldMin) / (oldMax - oldMin) * (newMax - newMin) + newMin;
}

//Adapted from http://stackoverflow.com/a/7616484/3874364
function hashString(s)
{
	var hash = 0;
	var length = s.length;
	if (length === 0) return hash;
	for (var i = 0; i < length; ++i)
	{
		var character = s.charCodeAt(1);
		hash = ((hash << 5) - hash) + character;
		hash |= 0;
	}
	return hash;
}
