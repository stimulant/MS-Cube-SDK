import processing.net.*;
import java.nio.*;

Server myServer;
Client myClient;
int val = 0;
byte[] recvBytes = new byte[247815];

void setup() {
  size(512, 424);
  // Starts a myServer on port 3000
  myServer = new Server(this, 3000); 
}

void parseBodies(ByteBuffer buffer) {
  background(0);
  
  // parse bodies        
  int bodyCount = (int)(byteBuffer.getShort() & 0xffff);
  //println("parse bodies: " + bodyCount);
  long[] trackingIds = new long[6];
  for (int i=0; i<6; i++) {
    trackingIds[i] = byteBuffer.getLong();
  } 
   
  // parse out joints
  for (int i=0; i<6; i++) {
    for (int j=0; j<25; j++) {
      
      float x = byteBuffer.getFloat(); 
      float y = byteBuffer.getFloat();
      float z = byteBuffer.getFloat();
      
      // draw joint
      if (trackingIds[i] != 0) {
        ellipse(x * width/2 +width/2.0, 
              (1.0-y) * height/2, 
              20, 20);
      }
    }
  }
}

void parseDepth(ByteBuffer buffer) {
  background(0);
  
  // parse depth
  int depthWidth = (int)(byteBuffer.getShort() & 0xffff);
  int depthHeight = (int)(byteBuffer.getShort() & 0xffff);
  //println("parse depth: " + depthWidth + ", " + depthHeight);
  for (int y=0; y<depthHeight; y++) {
    for (int x=0; x<depthWidth; x++) {
      byte depth = byteBuffer.get();
      set(x, y, color(depth));
    }
  }
}

int command = -1;
int commandLength = 0;
ByteBuffer byteBuffer;

void draw() {
  
  if (myClient != null && myClient.available() > 0) {
    
    // Read in the bytes
    int byteCount = myClient.readBytes(recvBytes); 
    if (byteCount > 0 ) {
        //println("byteCount: " + byteCount);
        
        int byteOffset = 0;
        while(byteCount > 0) {
        
          if (command == -1) {
            command = (int)(recvBytes[byteOffset] & 0xFF);
            commandLength = ((0xFF & recvBytes[byteOffset+1]) << 0) | ((0xFF & recvBytes[byteOffset+2]) << 8) |
              ((0xFF & recvBytes[byteOffset+3]) << 16) | ((0xFF & recvBytes[byteOffset+4]) << 24);
            //println("command: " + command + " commandLength: " + commandLength);
            byteBuffer = ByteBuffer.allocate(commandLength);
            byteOffset += 5;
            byteCount -= 5;
          }
        
          // put btes 
          int bytePutCount = min(commandLength, byteCount);
          //println("bytePutCount: " + bytePutCount + " byteOffset: " + byteOffset + " commandLength: " + commandLength + " bytecapacity: " + byteBuffer.capacity());
          byteBuffer.put(recvBytes, byteOffset, bytePutCount);
          byteOffset += bytePutCount;
          byteCount -= bytePutCount;
          commandLength -= bytePutCount;
          
          //println("byteOffset: " + bytePutCount + " commandLength: " + commandLength);
          if (commandLength == 0) {
            byteBuffer.flip();
            byteBuffer.order(ByteOrder.LITTLE_ENDIAN);
        
            if (command == 0) {
              parseBodies(byteBuffer);
            }
            else if (command == 1) {
              parseDepth(byteBuffer);
            }
            command = -1;
          }
        } 
    }
  }
}

void disconnectEvent(Client someClient) {
  print("Client disconnected");
}

// ServerEvent message is generated when a new client connects 
// to an existing server.
void serverEvent(Server someServer, Client someClient) {
  println("We have a new client: " + someClient.ip());
  myClient = someClient;
}
