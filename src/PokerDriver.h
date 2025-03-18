/** 
 * PokerDriver.h 
 * poker
 * created 03/17/25 by frank collebrusco
 */
#ifndef POKER_DRIVER_H
#define POKER_DRIVER_H
#include "Driver.h"
#include "Deck.h"
#include "Rendering.h"
#include <flgl/logger.h>
LOG_MODULE(poker);
using namespace glm;

struct CardInst {
    Card card;
    vec2 pos;
};

class PokerDriver : public GameDriver {
    virtual void user_create() override final;
    virtual void user_update(float dt, Keyboard const& kb, Mouse const& mouse) override final;
    virtual void user_render() override final;
    virtual void user_destroy() override final;

    vec2 m2w(vec2 mp) {
        mp /= vec2((float)window.frame.x, (float)window.frame.y);
        vec4 p = vec4((mp.x * 2.) - 1.,  -((mp.y * 2) - 1), 0., 1.);
        vec4 w = camera.iview() * (camera.iproj() * (p));
        return w.xy();
    }

    OrthoCamera camera;
    Deck deck;
    CardInst card;

};

void PokerDriver::user_create() {
    gl.init();
    window.create("poker", 480 * 1, 360 * 1);
    camera = OrthoCamera(vec3(0.,0.,-1.), vec3(0,0.,1.), vec3(0.,1.,0.), 1e-6, 1e6, 6);
    CardRenderer::init();

    deck = Deck::new_shuffled();
    card.card = deck.draw();
    card.pos = vec2(0.f);
}

void PokerDriver::user_update(float dt, Keyboard const& kb, Mouse const& mouse) {
    if (kb[GLFW_KEY_ESCAPE].down) this->close();
    camera.update();
    if (mouse.left.pressed) {
        card.card = deck.draw();
        LOG_INF("drew a %s of %s\n", rank_name(card.card.rank), suit_name(card.card.suit)); 
    }
    card.pos = m2w(mouse.pos);
}

void PokerDriver::user_render() {
    gl.clear();

    CardRenderer::sync_to_camera(camera);
    // CardRenderer::draw_card_at(card.card, card.pos.x, card.pos.y);
    DeckRenderer::draw(deck, card.pos.x, card.pos.y, 5);
}

void PokerDriver::user_destroy() {
    CardRenderer::destroy();
    gl.destroy();
}




#endif /* POKER_DRIVER_H */
