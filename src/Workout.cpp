#include "Workout.hpp"



// necessary declarations
DigitalOut situp_led(LED3);         // orange LED
DigitalOut pushup_led(LED4);        // green LED
DigitalOut jj_led(LED5);            // red LED
DigitalOut squat_led(LED6);         // blue LED
USBSerial serial;

// pi for calculations
const float pi = 3.1415926;
const int limit = 400;


// class constructor and destructor
Workout::Workout() {
    current_state = IDLE;
    for(int i = 0; i < 4; i++)
        counts[i] = 5;
};
Workout::~Workout() {} 


// class methods
State Workout::get_current_state() const {return current_state;}
void Workout::set_state(State new_state) { current_state = new_state; }
unsigned int Workout::get_count(State state) { return counts[state - 1]; }
void Workout::set_count(State state, unsigned int new_count) { counts[state - 1] = new_count; }
void Workout::check_reps() {
    int counter = 4;
    for(auto count: counts) {
        if(count != 0)
            continue;
        else
            counter -= 1;    
    }
    if(counter == 0)
        while(1) {
            situp_led = 1;
            wait(0.2);
            situp_led = 0;
            wait(0.2);
            jj_led = 1;
            wait(0.2);
            jj_led = 0;
            wait(0.2);
            squat_led = 1;
            wait(0.2);
            squat_led = 0;
            wait(0.2);
            pushup_led = 1;
            wait(0.2);
            pushup_led = 0;
            wait(0.5);
        }
}



void countdown_helper(DigitalOut led) {
    int seconds = 10;
    while(seconds-- > 0){
        led = 1;
        wait(.5);
        led = 0;
        wait(.5);
    }
}
void countdown(State state) {
    switch(state) {
        case SITUP:
            countdown_helper(situp_led);
            break;
        case PUSHUP:
            countdown_helper(pushup_led);
            break;
        case JJ:
            countdown_helper(jj_led);
            break;
        case SQUAT:
            countdown_helper(squat_led);
            break;
        case IDLE:
            idle();
            break;
    }
}



/*    exercise processing functions    */
// idle state
void idle() {
    situp_led = 1;
    pushup_led = 1;
    jj_led = 1;
    squat_led = 1;
    wait(0.75);

    situp_led = 0;
    pushup_led = 0;
    jj_led = 0;
    squat_led = 0;
    wait(0.75);
}
// situp processing
void process_situp(LIS3DSH* acc, Workout* workout) {
    if(workout->get_current_state() == SITUP) {
        // if reps left to do, perform countdown
        if(workout->get_count(SITUP) > 0)
            countdown(SITUP);

        int16_t z = 0;
        float z_gravity = 0;
        float angle = 0;

        Position position = DOWN;          // detects whether lying down or not
        int flag = 0;                       // used for rep keeping


        // problem: when switching to new exercise, 
        // buffer gets wonky values and throws off
        // initial rep

        // solution: wrap buffer in a counter in order
        // to ignore the initial wonky buffer values
        int counter = 0;                  

        
        // filter + parameters
        const uint8_t size = 20;
        float buffer[size];
        uint8_t index = 0;

        
        while(workout->get_current_state() == SITUP && workout->get_count(SITUP) > 0) {
            // processing input
            acc->ReadData(nullptr, nullptr, &z);           //read z value
            z_gravity = (float)z/17694.0;
            buffer[index++] = z_gravity;

            if(index >= size)
                index = 0;


            // filtering
            float filtered_z = 0;
            for(auto value: buffer)
                filtered_z += value;
            
            filtered_z /= (float)size;

            if(filtered_z > 1)
                filtered_z = 1;
            // end filtering


            if(counter++ > limit) {
                angle = 180*acos(filtered_z)/pi;
                switch(position) {
                    case DOWN:
                        situp_led = 0;
                        
                        if(flag == 1) {
                            flag = 0; 
                            workout->set_count(SITUP, workout->get_count(SITUP) - 1);
                            serial.printf("situp reps: %d\n", workout->get_count(SITUP));
                        }
                        if(angle > 55)
                            position = UP;
                        break;
                    case UP:
                        situp_led = 1;

                        flag = 1;
                        if(angle < 20)
                            position = DOWN;
                        break;
                }
            }
        }
        workout->set_state(PUSHUP);
    }
}
// pushup processing
void process_pushup(LIS3DSH* acc, Workout* workout) { 
    if(workout->get_current_state() == PUSHUP) {
        // if reps left to do, perform countdown
        if(workout->get_count(PUSHUP) > 0)
            countdown(PUSHUP);

        int16_t z = 0;
        float z_gravity = 0;
        float angle = 0;
        
        Position position = UP;             // detects position (up or down)
        int flag = 0;                       // used for rep keeping

        
        // filter + parameters
        const uint8_t size = 20;
        float buffer[size];
        uint8_t index = 0;

        
        while(workout->get_current_state() == PUSHUP && workout->get_count(PUSHUP) > 0) {
            // processing input
            acc->ReadData(nullptr, nullptr, &z);           //read z value
            z_gravity = (float)z/17694.0;
            buffer[index++] = z_gravity;

            if(index >= size)
                index = 0;


            // filtering
            float filtered_z = 0;
            for(auto value: buffer)
                filtered_z += value;
            
            filtered_z /= (float)size;

            if(filtered_z > 1)
                filtered_z = 1;
            // end filtering


            
            angle = 180*acos(filtered_z)/pi;
            switch(position) {
                case DOWN:
                    pushup_led = 1;

                    if(angle > 25)
                        position = UP;
                    flag = 1;
                    break;
 
                case UP:
                    pushup_led = 0;
                    
                    if(flag == 1) {
                        flag = 0;
                        workout->set_count(PUSHUP, workout->get_count(PUSHUP) - 1);
                        serial.printf("pushup reps left: %d\n", workout->get_count(PUSHUP));
                    }
                    if(angle < 5)
                        position = DOWN;
                    break;
                }
        }
        workout->set_state(JJ);
    }
}
// jumping jack processing
void process_jj(LIS3DSH* acc, Workout* workout) { 
    if(workout->get_current_state() == JJ) {
        // if reps left to do, perform countdown
        if(workout->get_count(JJ) > 0)
            countdown(JJ);

        float pitch = 0;

        Position position = DOWN;           // detects whether lying down or not
        int flag = 0;                       // used for rep keeping

        // filter + parameters
        const uint8_t size = 20;
        float buffer[size];
        uint8_t index = 0;

        
        while(workout->get_current_state() == JJ && workout->get_count(JJ) > 0) {
            // processing input
            acc->ReadAngles( &pitch, nullptr);           
            buffer[index++] = pitch;

            // if index outgrows buffer size, reset index
            if(index >= size)
                index = 0;


            // filtering
            float filtered_pitch = 0;
            for(auto value: buffer)
                filtered_pitch += value;
            
            filtered_pitch /= (float)size;
            // end filtering


            //wait(0.01);
            switch(position) {
                case DOWN:
                    jj_led = 0;
                    
                    if(flag == 1) {
                        flag = 0; 
                        workout->set_count(JJ, workout->get_count(JJ) - 1);
                        serial.printf("jj reps left: %d\n", workout->get_count(JJ));
                    }
                    if(filtered_pitch < 205)
                        position = UP;
                    break;
                case UP:
                    jj_led = 1;
                    flag = 1;
                    if(filtered_pitch > 290)
                        position = DOWN;
                    break;
            }          
        }
        workout->set_state(SQUAT);
    }
}
// squat processing
void process_squat(LIS3DSH* acc, Workout* workout) {
    if(workout->get_current_state() == SQUAT) {
        // if reps left to do, perform countdown
        if(workout->get_count(SQUAT) > 0)
            countdown(SQUAT);

        int16_t z = 0;
        float z_gravity = 0;
        float angle = 0;

        Position position = UP;          // detects whether lying down or not
        int flag = 0;                       // used for rep keeping


        int counter = 0;
        
        // filter + parameters
        const uint8_t size = 20;
        float buffer[size];
        uint8_t index = 0;

        
        while(workout->get_current_state() == SQUAT && workout->get_count(SQUAT) > 0) {
            // processing input
            acc->ReadData(nullptr, nullptr, &z);           //read z value
            z_gravity = (float)z/17694.0;
            buffer[index++] = z_gravity;

            if(index >= size)
                index = 0;


            // filtering
            float filtered_z = 0;
            for(auto value: buffer)
                filtered_z += value;
            
            filtered_z /= (float)size;

            if(filtered_z > 1)
                filtered_z = 1;
            // end filtering

        if(counter++ > limit) {
            angle = 180*acos(filtered_z)/pi;
            serial.printf("angle: %.2f\n", angle);
            switch(position) {
                case UP:
                    squat_led = 0;
                    
                    if(flag == 1) {
                        flag = 0; 
                        workout->set_count(SQUAT, workout->get_count(SQUAT) - 1);
                        serial.printf("squat reps: %d\n", workout->get_count(SQUAT));
                    }
                             
                    if(angle < 20)
                        position = DOWN;
                    break;
                case DOWN:
                    squat_led = 1;
                    flag = 1;
                    if(angle > 75)
                        position = UP;
                    break;
            }
            }
        }
        workout->set_state(SITUP);
    }
}