
char buf[200];
int chars_in_buf = 0;

int motor_fl_pwm_pin = 5;
int motor_fr_pwm_pin = 11;
int motor_bl_pwm_pin = 9;
int motor_br_pwm_pin = 10;
int motor_fl_gpio_pin = 2;
int motor_fr_gpio_pin = 3;
int motor_bl_gpio_pin = 4;
int motor_br_gpio_pin = 8;

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(10);

  pinMode(motor_fl_pwm_pin, OUTPUT);
  pinMode(motor_fr_pwm_pin, OUTPUT);
  pinMode(motor_bl_pwm_pin, OUTPUT);
  pinMode(motor_br_pwm_pin, OUTPUT);
  pinMode(motor_fl_gpio_pin, OUTPUT);
  pinMode(motor_fr_gpio_pin, OUTPUT);
  pinMode(motor_bl_gpio_pin, OUTPUT);
  pinMode(motor_br_gpio_pin, OUTPUT);
}

void loop()
{
  int new_chars = Serial.readBytesUntil('\n', buf + chars_in_buf, sizeof(buf) - chars_in_buf);
  chars_in_buf += new_chars;
  if (new_chars > 0 && buf[chars_in_buf - 1] == '\n') {
    Serial.println(buf);
    // process command
    if (strncmp(buf, "motor", 5) == 0) {
      float motor_speed = atof(buf + 9);
      if (strncmp(buf + 6, "fl", 2) == 0) {
        write_motor_speed(motor_speed, motor_fl_pwm_pin, motor_fl_gpio_pin);
      } else if (strncmp(buf + 6, "fr", 2) == 0) {
        write_motor_speed(motor_speed, motor_fr_pwm_pin, motor_fr_gpio_pin);
      } else if (strncmp(buf + 6, "bl", 2) == 0) {
        write_motor_speed(motor_speed, motor_bl_pwm_pin, motor_bl_gpio_pin);
      } else if (strncmp(buf + 6, "br", 2) == 0) {
        write_motor_speed(motor_speed, motor_br_pwm_pin, motor_br_gpio_pin);
      }
    }
  }
  Serial.println("listening");
}

void write_motor_speed(float speed, int pwm_pin, int gpio_pin) {
  if (speed >= 0) {
    digitalWrite(gpio_pin, 0);
    analogWrite(pwm_pin, speed * 255);
  } else {
    digitalWrite(gpio_pin, 1);
    analogWrite(pwm_pin, (1 + speed) * 255);
  }
}

