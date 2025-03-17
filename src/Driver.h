/** 
 * Driver.h 
 * poker
 * created 03/17/25 by frank collebrusco
 */
#ifndef DRIVER_H
#define DRIVER_H

#include <stdint.h>
#include <cstddef>
#include <flgl.h>
#include "Stopwatch.h"

// typedef void(*driver_init_f)(void); 
// typedef void(*driver_update_f)(void);
// typedef void(*driver_render_f)(void);
// typedef void(*driver_destroy_f)(void);
// typedef void(*driver_update_f)(float dt, Keyboard const& kb, Mouse const& mouse);

// struct Driver {
//     driver_init_f init;
//     driver_update_f update;
//     driver_render_f render;
//     driver_destroy_f destroy;
// };


#define DRIVER_MAIN_FUNCTION(main_name, Classname) int main_name() {Classname *driver = new Classname(); driver->start(); delete driver; return 0;}

class GameDriver {
protected:
	void close();
public:
    void start();
    void loop();
    GameDriver();

    float const& dt;
    Stopwatch const& launch_timer;

private:
	bool _close;
	float _dt;
	Stopwatch _launch_timer;
    Stopwatch delta_timer;
    virtual void user_create() = 0;
    virtual void user_update(float dt, Keyboard const& kb, Mouse const& mouse) = 0;
    virtual void user_render() = 0;
    virtual void user_destroy() = 0;
    void exit();
};


#endif /* DRIVER_H */
