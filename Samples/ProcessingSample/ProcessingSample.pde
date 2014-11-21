import processing.net.*;
import java.nio.*;

Server myServer;
Client myClient;
int val = 0;
byte[] recvBytes = new byte[247815];

void setup() {
  size(800, 800);
  // Starts a myServer on port 3000
  myServer = new Server(this, 3000); 
}

void draw() {
  background(0);
  
  if (myClient != null && myClient.available() > 0) {
    
    // Read in the bytes
    int byteCount = myClient.readBytes(recvBytes); 
    if (byteCount > 0 ) {
      try {
        // Convert the byte array to a ByteBufer
        ByteBuffer byteBuffer = ByteBuffer.wrap(recvBytes, 0, byteCount);
        println("byteBuffer: " + byteBuffer.limit());
        
        int command = (int)(byteBuffer.get() & 0xFF);
        int commandLength = byteBuffer.getInt() & 0xFFFF;
        
        println("command: " + command + " commandLength: " + commandLength);
        if (command == 0) {
          // parse bodies        
          int bodyCount = byteBuffer.get() & 0xFFFF;
          println("parse bodies: " + bodyCount);
          long[] trackingIds = new long[6];
          for (int i=0; i<6; i++) {
            trackingIds[i] = byteBuffer.getLong();
          } 
          println("parse trackingIds: " + trackingIds[0] + " " + trackingIds[1] + " " + trackingIds[2] + " "
           + trackingIds[3] + " " + trackingIds[4] + " " + trackingIds[5]);
           
          // parse out joints
          for (int i=0; i<6; i++) {
            for (int j=0; j<25; j++) {
              
              float x = byteBuffer.getFloat(); 
              float y = byteBuffer.getFloat();
              float z = byteBuffer.getFloat();
              //if (x != 0.0)
                //println( "x: " + x );
              
              // draw joint
              if (trackingIds[i] != 0) {
                ellipse(x * width/2 +width/2.0, 
                      (1.0-y) * height/2, 
                      20, 20);
              }
            }
          }
        }
        else if (command == 1) {
          // parse depth
          println("parse depth: " + byteBuffer.limit());
        } 
      } catch (BufferUnderflowException bue) {
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
