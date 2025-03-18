/** 
 * PokerDriver.h 
 * poker
 * created 03/17/25 by frank collebrusco
 */
#ifndef POKER_DRIVER_H
#define POKER_DRIVER_H
#include "Driver.h"
#include "Deck.h"
#include <flgl/tools.h>
#include <flgl/logger.h>
LOG_MODULE(poker);
using namespace glm;

class CardRenderer {
    static Shader card_shader;
    static VertexArray card_vao;
    static VertexBuffer<Vt_2Dclassic> card_vbo;
    static ElementBuffer card_ibo;
    static Texture card_sheet;
public:
    static void init() {
        card_shader = Shader::from_source("vert_cards", "frag_cards");
        card_sheet = Texture::from_file("cards", true);
        card_vao.create_bind();
        card_vbo.create_bind();
        card_vao.attach(card_vbo);
        const Vt_2Dclassic card_verts[] = {
            {{-0.5, -0.7}, { 0., 0.}},
            {{-0.5,  0.7}, { 0., 1.}},
            {{ 0.5,  0.7}, { 1., 1.}},
            {{ 0.5, -0.7}, { 1., 0.}}
        };
        card_vbo.buffer_data(4, card_verts);
        card_ibo.create_bind();
        const uint32_t card_elems[] = {
            0, 1, 2,
            0, 3, 2
        };
        card_ibo.buffer_data(6, card_elems);
        unbind();
    }

    static void unbind() {
        // card_shader.unbind();
        card_vao.unbind();
        card_vbo.unbind();
        card_ibo.unbind();
        // card_sheet.unbind();
    }

    static void destroy() {
        card_shader.destroy();
        card_vao.destroy();
        card_vbo.destroy();
        card_ibo.destroy();
        card_sheet.destroy();
    }

    static void sync_to_camera(Camera& cam) {
        card_shader.uMat4("uView", cam.view());
        card_shader.uMat4("uProj", cam.proj());
    }

    static void sync_to_card(Card const& card) {
        int y;
        switch(card.suit) {
        case SUIT_CLUBS:
            y = 4; break;
        case SUIT_DIAMONDS:
            y = 2; break;
        case SUIT_HEARTS:
            y = 1; break;
        case SUIT_SPADES:
            y = 3; break;
        default:
            break;
        }
        ivec2 sp = {
            ((int)card.rank + 1) % 13,
            y
        };
        card_shader.uIVec2("uSheetPos", sp);
    }

    static void draw_at(float x, float y, float r = 0.f) {
        card_shader.bind();
        card_shader.uMat4("uModel", genModelMat2d(vec2(x,y), r, vec2(1.)));
        card_sheet.bind();
        card_vao.bind();
        gl.draw_vao_ibo(card_ibo);
        unbind();
    }

    static void draw_card_at(Card const& card, float x, float y) {
        sync_to_card(card); draw_at(x, y);
    }

};
Shader CardRenderer::card_shader;
VertexArray CardRenderer::card_vao;
VertexBuffer<Vt_2Dclassic> CardRenderer::card_vbo;
ElementBuffer CardRenderer::card_ibo;
Texture CardRenderer::card_sheet;

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
    camera = OrthoCamera(vec3(0.,0.,-1), vec3(0,0.,1.), vec3(0.,1.,0.), 1e-6, 1e6, 6);
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
    CardRenderer::draw_card_at(card.card, card.pos.x, card.pos.y);
}

void PokerDriver::user_destroy() {
    CardRenderer::destroy();
    gl.destroy();
}




#endif /* POKER_DRIVER_H */
