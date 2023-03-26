Servo servoMotor;

void abrirCochera() {
  // rotates from 0 degrees to 180 degrees
  for (int pos = 0; pos <= 90; pos += 1) {
    // in steps of 1 degree
    servoMotor.write(pos);
    delay(15);  // waits 15ms to reach the position
  }
}

void cerrarCochera() {
  // rotates from 180 degrees to 0 degrees
  for (int pos = 90; pos >= 0; pos -= 1) {
    servoMotor.write(pos);
    delay(15);  // waits 15ms to reach the position
  }
}
