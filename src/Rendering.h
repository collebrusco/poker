/** 
 * Rendering.h 
 * poker
 * created 03/18/25 by frank collebrusco
 */
#ifndef RENDERING_H
#define RENDERING_H
#include "Deck.h"
#include <flgl.h>
#include <flgl/tools.h>

class CardRenderer {
    static Shader card_shader;
    static VertexArray card_vao;
    static VertexBuffer<Vt_2Dclassic> card_vbo;
    static ElementBuffer card_ibo;
    static Texture card_sheet;
public:
    static void init();
    static void unbind();
    static void destroy();

    static void sync_to_camera(Camera& cam);
    static void sync_to_card(Card const& card);

    static void draw_at(float x, float y, float r = 0.f);
    static void draw_card_at(Card const& card, float x, float y, float r = 0.f);

};

#endif /* RENDERING_H */
