var img;
var bodyData;
var imgWidth = 512;
var imgHeight = 424;

function setup() {
  createCanvas(imgWidth, imgHeight); 
  img = createImage(imgWidth, imgHeight);
}

function updateDepth(imgArray) {
	img.loadPixels();

	for (var i = 0; i<(imgWidth * imgHeight); i++){
		var idx = i * 4;
		var color = imgArray[ i*2 ];
		img.pixels[ idx ] = color;
		img.pixels[ idx + 1 ] = color;
		img.pixels[ idx + 2 ] = color;
		img.pixels[ idx + 3 ] = 255;
	}

	img.updatePixels();
}

function updateBodies(data) {
	bodyData = data;
}

function drawBodies() {
	if (bodyData != undefined) {
		noStroke();
		fill(255,50);
		ellipseMode(CENTER);

		for (var b in bodyData.bodies) {
			if (bodyData.bodyTrackingIds[b] != 0) {
				for (var i in bodyData.bodies[b]) {
				  ellipse(bodyData.bodies[b][i].x * canvas.width/2 + canvas.width/2.0,
								(1.0-bodyData.bodies[b][i].y) * canvas.height/2,16,16);
				}
			}
		}
	}
}

function draw() {
  background(0);
  image(img, 0, 0, imgWidth, imgHeight);
  drawBodies();
}