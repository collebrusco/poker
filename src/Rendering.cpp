#include "Rendering.h"
using namespace glm;

Shader CardRenderer::card_shader;
VertexArray CardRenderer::card_vao;
VertexBuffer<Vt_2Dclassic> CardRenderer::card_vbo;
ElementBuffer CardRenderer::card_ibo;
Texture CardRenderer::card_sheet;

void CardRenderer::init() {
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

void CardRenderer::unbind() {
    card_shader.unbind();
    card_vao.unbind();
    card_vbo.unbind();
    card_ibo.unbind();
    card_sheet.unbind();
}

void CardRenderer::destroy() {
    card_shader.destroy();
    card_vao.destroy();
    card_vbo.destroy();
    card_ibo.destroy();
    card_sheet.destroy();
}

void CardRenderer::sync_to_camera(Camera& cam) {
    card_shader.uMat4("uView", cam.view());
    card_shader.uMat4("uProj", cam.proj());
}

void CardRenderer::sync_to_card(Card const& card) {
    int y = 0;
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

void CardRenderer::draw_at(float x, float y, float r) {
    card_shader.bind();
    card_shader.uMat4("uModel", genModelMat2d(vec2(x,y), r, vec2(1.)));
    card_sheet.bind();
    card_vao.bind();
    gl.draw_vao_ibo(card_ibo);
    unbind();
}

void CardRenderer::draw_card_at(Card const& card, float x, float y, float r) {
    sync_to_card(card); draw_at(x, y, r);
}

void DeckRenderer::draw(Deck const &deck, float x, float y, float spread, bool facedown) {
    size_t i = 0;
    for (auto card : deck) {
        CardRenderer::draw_card_at(card, x - (spread/2) + (((float)i * spread) / (float)deck.size()), y, 0.);
        i++;
    }
}
