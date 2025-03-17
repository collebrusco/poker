#include "Driver.h"
#include <cmath>
#include <flgl/logger.h>
#include <flgl.h>
LOG_MODULE(driver);

#define COMPLICATED_LOOP 0

GameDriver::GameDriver() :  dt(_dt), launch_timer(_launch_timer),
                            _close(false),
                            _dt(1.f/60.f),
                            _launch_timer(SECONDS),
                            delta_timer(SECONDS)
{}

void GameDriver::exit() {
    user_destroy();
}

#ifdef BENCHMARK
static Stopwatch t_upd(MICROSECONDS);
static Stopwatch t_ren(MICROSECONDS);
#endif /* BENCHMARK */


#define min(x,y) ((x)<(y)?(x):(y))
void GameDriver::loop() {

#ifdef BENCHMARK
    static float tu[32]; 
    static float tr[32];
    static uint8_t t = 0;
    t_upd.reset_start();
#endif /* BENCHMARK */

#if COMPLICATED_LOOP == 1
    if (_dt > 1.f/30.f) {
        while (_dt > 0.f) {
            user_update(min(_dt, 1.f/60.f), window.keyboard, window.mouse);
            _dt -= 1.f/60.f;
        }
    } else {
        user_update(_dt, window.keyboard, window.mouse);
    }
#else 
    user_update(_dt, window.keyboard, window.mouse);
#endif /* COMPLICATED_LOOP */
#ifdef BENCHMARK
    tu[(t)&0x1F] = t_upd.stop();
    t_ren.reset_start();
#endif /* BENCHMARK */
    user_render();

#ifdef BENCHMARK

    glFinish();

    tr[(t)&0x1F] = t_ren.read();
    t++;

    if (!(t%32)) {
        float tua = 0;
        float tra = 0;
        for (int i = 0; i < 32; i++) {
            tua += tu[i];
            tra += tr[i];
        }

        tua /= 32.f; tra /= 32.f;

        LOG_INF("up %.2fus, ren %.2fus, tot %.2fus for %.1f%% overhead", tua, tra, tua + tra, 100.f * ((tua + tra) / (_dt * (SECONDS/MICROSECONDS))));
    }
#endif /* BENCHMARK */

    window.update();

    _dt = delta_timer.stop_reset_start();
    if (std::isinf(_dt) || std::isnan(_dt)){
        _dt = 1.f / 80.f; //TODO: hate this...
    }
}

void GameDriver::start() {
    _launch_timer.start();
    user_create();
    LOG_INF("created, loop:");
    while (!_close && !window.should_close()) {
       loop();
    }
    LOG_INF("out of loop, exiting...");
    exit();
}

void GameDriver::close() {
	_close = true;
}
