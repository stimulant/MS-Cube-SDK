var img;
var bodyData;

function setup() {
  createCanvas(512, 424); 
  img = createImage(512, 424);
}

function updateDepth(imgArray) {
	img.loadPixels();
	  for(var x = 0; x < 512; x++) {
	    for(var y = 0; y < 424; y++) {
	    	var avg = imgArray[y*512 + x];
	      img.set(x, y, [avg, avg, avg, 255]); 
	    }
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
  image(img, 0, 0, 512, 424);
  drawBodies();
}