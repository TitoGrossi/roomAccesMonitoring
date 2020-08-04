#define pin_outer_sensor_laser 11
#define pin_inner_sensor_laser 12

#define pin_outer_sensor A0
#define pin_inner_sensor A1

#define pin_ld_light 10
#define buzzer_alarm 8

int max_num_people = 2;
unsigned long max_time = 10000;
int difference_natural_light_laser = 150;

void setup() {
  pinMode(pin_outer_sensor_laser, OUTPUT);
  pinMode(pin_inner_sensor_laser, OUTPUT);
  digitalWrite(pin_outer_sensor_laser, HIGH);
  digitalWrite(pin_inner_sensor_laser, HIGH);

  pinMode(pin_outer_sensor, INPUT);
  pinMode(pin_inner_sensor, INPUT);

  pinMode(pin_ld_light, OUTPUT);
  pinMode(buzzer_alarm, OUTPUT);
  delay(500);
}

bool get_change_in_sensor_state(int prev, int curr, int sig) {
  if ((curr - prev)*sig > difference_natural_light_laser)
    return true;
  return false;
}

void get_normal_update(int previous_outer, int previous_inner, char *state) {
  if (get_change_in_sensor_state(previous_outer, analogRead(pin_outer_sensor), 1))
    *state = 'i';
  else if (get_change_in_sensor_state(previous_inner, analogRead(pin_inner_sensor), 1))
    *state = 'o';
  else *state = 'n';
}

int get_comming_in_update(int previous_outer, int previous_inner, int person_counter, char *state) {
  bool received_outer = get_change_in_sensor_state(previous_outer, analogRead(pin_outer_sensor), -1);
  bool received_inner = get_change_in_sensor_state(previous_inner, analogRead(pin_inner_sensor), -1);

  if (!received_outer && received_inner) { // if the person really went in
    person_counter++; // Increase counter
    *state = 'n'; // Change state back to n
  }

  else if (!received_outer && received_inner) // If the person came back out
    *state = 'n';

  else
    *state = 'i';

  return person_counter;
}

int get_comming_out_update(int previous_outer, int previous_inner, int person_counter, char *state) {
  bool received_outer = get_change_in_sensor_state(previous_outer, analogRead(pin_outer_sensor), -1);
  bool received_inner = get_change_in_sensor_state(previous_inner, analogRead(pin_inner_sensor), -1);

  if (received_outer && !received_inner && person_counter > 0) { // if person really went out
    person_counter--; // Decrease counter
    *state = 'n'; // Change state back to n
  }

  else if (received_outer && !received_inner) // If the person came back in
    *state = 'n';

  else
    *state = 'o';

  return person_counter;
}

void loop() {
  static char previous_state = 'n';
  char state;
  static int previous_outer = 700, previous_inner = 700, person_counter = 0, previous_person_counter = 0;
  static unsigned long time_num_people_exceeded = 0;

  switch (previous_state) {
    case 'n':
      get_normal_update(previous_outer, previous_inner, &state);
      break;
    case 'i':
      person_counter = get_comming_in_update(previous_outer, previous_inner, person_counter, &state);
      break;
    case 'o':
      person_counter = get_comming_out_update(previous_outer, previous_inner, person_counter, &state);
  }

  bool was_over_before = previous_person_counter > max_num_people;
  bool is_over_now = person_counter > max_num_people;
  if (!was_over_before && is_over_now) { // the number of people just extrapolated
    digitalWrite(pin_ld_light, HIGH);
    time_num_people_exceeded = millis();
  }

  else if (was_over_before && is_over_now) { // The number of people continues higher than allowed
    if (millis() - time_num_people_exceeded > max_time)
      digitalWrite(buzzer_alarm, HIGH);
  }

  else if (was_over_before && !is_over_now) {
    digitalWrite(pin_ld_light, LOW);
    digitalWrite(buzzer_alarm, LOW);
    time_num_people_exceeded = 0;
  }

  // Updating the previous_* variables for the next iteration
  previous_person_counter = person_counter;
  previous_state = state;
  previous_outer = analogRead(pin_outer_sensor);
  previous_inner = analogRead(pin_inner_sensor);
  previous_person_counter = person_counter;

  delay(20);
}
