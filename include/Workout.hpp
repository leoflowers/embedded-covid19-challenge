#include "LIS3DSH.h"
#include <vector>
#include <USBSerial.h>


// possible states within each workout
typedef enum {
    IDLE,
    SITUP,
    PUSHUP,
    JJ,
    SQUAT
} State;


// possible states within each rep
typedef enum {
    UP,
    DOWN
} Position;


// workout class; holds relevant information about the workout
class Workout {
    private:
        State current_state;
        unsigned int counts[4];
        
    public:
        Workout();       
        State get_current_state() const;
        unsigned int get_count(State state);
        void set_count(State state, unsigned int new_count);
        void set_state(State new_state);
        void check_reps();
        ~Workout(); 
};


// misc.
void countdown_helper(DigitalOut led);
void countdown(State state);


// relevant methods for each of the states possible
void idle();
void process_situp(LIS3DSH* acc, Workout* workout);
void process_pushup(LIS3DSH* acc, Workout* workout);
void process_jj(LIS3DSH* acc, Workout* workout);
void process_squat(LIS3DSH* acc, Workout* workout);



// button interrupt setup
void toggle();