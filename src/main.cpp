#include <mbed.h>
#include <Workout.hpp>


// necessary declarations
LIS3DSH acc(PA_7, PA_6, PA_5, PE_3);
InterruptIn button(PA_0, PullDown);
Workout workout = Workout();


// button interrupt action
void toggle();


int main() {
  /* set up */
  // attaches toggle interrupt handler to button
  button.rise(&toggle);

  // checks accelerometer is working
  if(acc.Detect() != 1) {
    while(1) {};
  }

  // idle state until button is pressed
  while(workout.get_current_state() == IDLE) {
    idle();
  }

  /* working loop */
  while(1) {
    switch(workout.get_current_state()) {
      case SITUP:
        process_situp(&acc, &workout);
        break;
      case PUSHUP:
        process_pushup(&acc, &workout);
        break;
      case JJ:
        process_jj(&acc, &workout);
        break;
      case SQUAT:
        process_squat(&acc, &workout);
        workout.check_reps();
        break;
      default:
        workout.set_state(SITUP);
        break;
    }
  }
}


void toggle() {
  switch(workout.get_current_state()){
    case IDLE:
      workout.set_state(SITUP);
      break;
    case SITUP:
      workout.set_state(PUSHUP);
      break;
    case PUSHUP:
      workout.set_state(JJ);
      break;
    case JJ:
      workout.set_state(SQUAT);
      break;
    case SQUAT:
      workout.set_state(SITUP);
      break;
  }
}