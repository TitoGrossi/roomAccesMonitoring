#define pin_outer_sensor_laser 11;
#define pin_inner_sensor_laser 12;

#define pin_outer_sensor A4;
#define pin_inner_sensor A5;

#define pin_ld_light 10;
#define buzzer_alarm 8;

int max_num_people 3;
unsigned long max_time = 15000
int limit_natural_light_laser = 50;

void setup() {
  pinMode(pin_outer_sensor_laser, OUTPUT);
  pinMode(pin_inner_sensor_laser, OUTPUT);
  digitalWrite(pin_outer_sensor_laser, HIGH)
  digitalWrite(pin_inner_sensor_laser, HIGH)

  pinMode(pin_outer_sensor, INPUT);
  pinMode(pin_inner_sensor, INPUT);

  pinMode(pin_ld_light, OUTPUT);
  pinMode(buzzer_alarm, OUTPUT);
}

bool get_change_in_sensor_state(int prev, int curr) {
  if (abs(prev - curr) > 20)
    return true
  return false
}

void get_normal_update(int previous_outer, int previous_inner, char *state) {
  if (get_change_in_sensor_state(previous_outer, analogRead(pin_outer_sensor)))
    *state = 'i';
  else if (get_change_in_sensor_state(previous_inner, analogRead(pin_inner_sensor)))
    *state = 'o';
  else *state = 'n';
  return state;
}

int get_comming_in_update(int previous_outer, int previous_inner, int person_counter, char* state) {
  bool changed_outer = get_change_in_sensor_state(previous_outer, analogRead(pin_outer_sensor));
  bool changed_inner = get_change_in_sensor_state(previous_inner, analogRead(pin_inner_sensor));
  if (!changed_outer && changed_inner) { // if the person really went in
    person_counter++; // Increase counter
    *state = 'n'; // Change state back to n
  }
  else if (changed_outer and !changed_inner) // If the person came back out
    *state = 'n';

  return person_counter;
}

int get_comming_out_update(int previous_inner, int previous_outer, int person_counter, char* state) {
  bool changed_outer = get_change_in_sensor_state(previous_outer, analogRead(pin_outer_sensor));
  bool changed_inner = get_change_in_sensor_state(previous_inner, analogRead(pin_inner_sensor));

  if (!changed_inner && changed_outer && person_counter > 0) { // if person really went out
    person_counter--; // Decrease counter
    *state = 'n'; // Change state back to n
  }

  else if (changed_inner and !changed_outer) { // If the person came back in
    *state = 'n';
  }

  return person_counter;
}

void loop() {
  static char previous_state = 'n';
  char state;
  static int previous_outer = 0, previous_inner = 0, person_counter = 0, previous_person_counter = 0;
  static unsigned long time_num_people_exceeded = 0;

  switch (previous_state) {
    case 'n':
      get_normal_update(previous_outer, previous_inner, *state);
      break;
    case 'i':
      person_counter = get_comming_in_update(previous_outer, previous_inner, person_counter, *state);
      break;
    case 'o':
      counter = get_comming_out_update();
  }

  was_over_before = previous_person_counter > max_num_people;
  is_over_now = previous_person_counter > max_num_people;
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
    time_num_people_exceeded = 0
  }

  // Updating the previous_* variables for the next iteration
  previous_person_counter = person_counter;
  previous_state = state;
  previous_outer = analogRead(pin_outer_sensor);
  previous_inner = analogRead(pin_inner_sensor);
  previous_person_counter = person_counter;
}
