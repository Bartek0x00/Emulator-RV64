#include "terminal.hpp"

using namespace Emulator;

void reset(uint32_t _rows, uint32_t _cols, TTF_Font *_font)
{
    if (vterm) {
        vterm_free(vterm);
        vterm = nullptr;
    }
    invalidate_texture();

    if (surface) {
        SDL_FreeSurface(surface);
        surface = nullptr;
    }

    matrix = Matrix<unsigned char>(_rows, _cols);
    font = _font;
    font_height = TTF_FontHeight(font);

    vterm = vterm_new(_rows, _cols);
    vterm_set_utf8(vterm, 1);
    vterm_output_set_callback(vterm, [](const char *str, size_t len, void *user) {}, nullptr);

    vterm = vterm_obtain_screen(vterm);
    vterm_screen_set_callbacks(screen, &screen_callbacks, this);
    vterm_screen_reset(vterm, 1);

    matrix.fill(0);
    TTF_SizeUTF8(font, "X", &font_width, nullptr);
    surface = SDL_CreateRGBSurfaceWithFormat(
        0, font_width * _cols, font_height * _rows,
        32, SDL_PIXELFORMAT_RGBA32
    );
}

void render(SDL_Renderer *renderer, const SDL_Rect& window_rect)
{
    if (texture)
        goto cursor;

    for (uint32_t row = 0; row < matrix.rows; row++) {
        for (uint32_t col = 0; col < matrix.cols; col++) {
            if (!matrix(row, col))
                continue;
            
            VTermPos pos = {.row = row, .col = col};
            VTermScreenCell cell;
            vterm_screen_get_cell(screen, pos, &cell);

            if (cell.chars[0] == 0xFFFFFFFF)
                continue;

            icu::UnicodeString ustr;
            for (int i = 0; cell.chars[i] != 0 && i < VTERM_MAX_CHARS_PER_CELL; i++)
                ustr.append(static_cast<UChar32>(cell.chars[i]));

            SDL_Color fg_color = {.r = 128, .g = 128, .b = 128};
            SDL_Color bg_color = {0};

            if (VTERM_COLOR_IS_INDEXED(&cell.fg))
                vterm_screen_convert_color_to_rgb(screen, &cell.fg);

            if (VTERM_COLOR_IS_RGB(&cell.fg)) {
                fg_color.r = cell.fg.rgb.red;
                fg_color.g = cell.fg.rgb.green;
                fg_color.b = cell.fg.rgb.blue;
            }

            if (VTERM_COLOR_IS_INDEXED(&cell.bg))
                vterm_screen_convert_color_to_rgb(screen, &cell.bg);
                
            if (VTERM_COLOR_IS_RGB(&cell.bg)) {
                bg_color.r = cell.bg.rgb.red;
                bg_color.g = cell.bg.rgb.green;
                bg_color.b = cell.bg.rgb.blue;
            }

            
        }
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

cursor:
    SDL_RenderCopy(renderer, texture, nullptr, &window_rect);
    VTermScreenCell cell;
    vterm_screen_get_cell(screen, cursor_pos, &cell);

    SDL_Rect rect = {
        .x = window_rect.x + ((cursor_pos.col * font_width) * window_rect.w) / surface->w,
        .y = window_rect.y + ((cursor_pos.row * font_height) * window_rect.h) / surface->h,
        .w = ((font_width * window_rect.w) / surface->w) * cell_width,
        .h = (font_height * window_rect.h) / surface->h
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 96);
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &rect);

    if (!ringing)
        return;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 192);
    SDL_RenderFillRect(renderer, &window_rect);
    ringing = 0;
}

void process_event(const SDL_Event& event)
{
    if (event.type == SDL_TEXTINPUT) {
        const Uint8 *state = SDL_GetKeyboardState(nullptr);

        int mod = VTERM_MOD_NONE;

        if (state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL])
            mod |= VTERM_MOD_CTRL;

        if (state[SDL_SCANCODE_LALT] || state[SDL_SCANCODE_RALT])
            mod |= VTERM_MOD_ALT;

        if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT])
            mod |= VTERM_MOD_SHIFT;

        for (char ch : event.text.text)
            vterm_keyboard_unichar(vterm, ch, static_cast<VTermModifier>(mod));
            
        return;
    }

    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            vterm_keyboard_key(vterm, VTERM_KEY_ENTER, VTERM_MOD_NONE);
            break;

        case SDLK_BACKSPACE:
            vterm_keyboard_key(vterm, VTERM_KEY_BACKSPACE, VTERM_MOD_NONE);
            break;
        
        case SDLK_ESCAPE:
            vterm_keyboard_key(vterm, VTERM_KEY_ESCAPE, VTERM_MOD_NONE);
            break;
        
        case SDLK_TAB:
            vterm_keyboard_key(vterm, VTERM_KEY_TAB, VTERM_MOD_NONE);
            break;

        case SDLK_UP:
            vterm_keyboard_key(vterm, VTERM_KEY_UP, VTERM_MOD_NONE);
            break;
        
        case SDLK_DOWN:
            vterm_keyboard_key(vterm, VTERM_KEY_DOWN, VTERM_MOD_NONE);
            break;

        case SDLK_LEFT:
            vterm_keyboard_key(vterm, VTERM_KEY_LEFT, VTERM_MOD_NONE);
            break;

        case SDLK_RIGHT:
            vterm_keyboard_key(vterm, VTERM_KEY_RIGHT, VTERM_MOD_NONE);
            break;
        
        case SDLK_PAGEUP:
            vterm_keyboard_key(vterm, VTERM_KEY_PAGEUP, VTERM_MOD_NONE);
            break;
        
        case SDLK_PAGEDOWN:
            vterm_keyboard_key(vterm, VTERM_KEY_PAGEDOWN, VTERM_MOD_NONE);
            break;
        
        case SDLK_HOME:
            vterm_keyboard_key(vterm, VTERM_KEY_HOME, VTERM_MOD_NONE);
            break;
        
        case SDLK_END:
            vterm_keyboard_key(vterm, VTERM_KEY_END, VTERM_MOD_NONE);
            break;
        
        default:
            if (event.key.keysym.mod & KMOD_CTRL && event.key.keysym.sym < 127)
                vterm_keyboard_unichar(vterm, event.key.keysym.sym, VTERM_MOD_CTRL);
            break;
        }
    }
}

int Terminal::damage(VTermRect rect, void *user)
{
    user = reinterpret_cast<Terminal*>(user);
    user->invalidate_texture();
    user->matrix.fill(1);
    return 0;
}

int Terminal::movecursor(VTermPos pos, VTermPos old_pos, int visible, void *user)
{
    reinterpret_cast<Terminal*>(user)->cursor_pos = pos;
    return 0;
}

int Terminal::bell(void *user)
{
    reinterpret_cast<Terminal*>(user)->ringing = true;
    return 0;
}

int Terminal::resize(int rows, int cols, void *user)
{
    user = reinterpret_cast<Terminal*>(user);

    user->vterm_set_size(user->vterm, rows, cols);
    user->matrix = Matrix<unsigned char>(rows, cols);
    user->matrix.fill(1);
    user->vterm_screen_reset(user->screen, 1);
    user->invalidate_texture();

    return 0;
}

int Terminal::moverect(VTermRect dest, VTermRect src, void *user) {return 0;}
int Terminal::settermprop(VTermProp prop, VTermValue *value, void *user) {return 0;}
int Terminal::sb_pushline(int cols, const VTermScreenCell *cells, void *user) {return 0;}
int Terminal::sb_popline(int cols, VTermScreenCell *cells, void *user) {return 0;}