#include <PipedStream.h>

PipedStreamPair pipes;
PipedStream& streamPing = pipes.first;
PipedStream& streamPong = pipes.second;

void setup() {
  Serial.begin(9600);
  streamPing.write(0);
}

void pingPong(Stream& stream, const char* pingpong) {
  if (stream.available()) {
    uint8_t v = stream.read();
    Serial.print(pingpong);
    Serial.print(" (");
    Serial.print(v);
    Serial.println(")");
    stream.write(v+1);
    delay(500);
  }
}

void loop() {
  pingPong(streamPing, "Ping");
  pingPong(streamPong, "Pong");
}
