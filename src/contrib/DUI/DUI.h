/*
Copyright 2020 Stephen Lane-Walsh

Permission is hereby granted, free of charge, to any person 
obtaining a copy of this software and associated documentation 
files (the "Software"), to deal in the Software without 
restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or 
sell copies of the Software, and to permit persons to whom the 
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be 
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef DUI_H
#define DUI_H

#include <SDL.h>
#include <stdint.h>
#include <stdbool.h>

#define DUI_CHAR_SIZE (8)

typedef struct
{
    int LineHeight;             // = 12

    int PanelPadding;           // = 8

    int ButtonPadding;          // = 4
    int ButtonMargin;           // = 8

    int TabPadding;             // = 8
    int TabMargin;              // = 8

    uint8_t ColorBackground[4]; // = 0xEE
    uint8_t ColorBorder[4];
    uint8_t ColorHover[4];
    uint8_t ColorDefault[4];

} DUI_Style;

/* Initialize the Debug UI.
 *
 * @param window: The SDL window to draw to. This will be used
 *   to get the SDL_Renderer for drawing, and the WindowID for
 *   determining which events are relevant. 
 */
void DUI_Init(SDL_Window * window);

/* Terminate the Debug UI.
 */
void DUI_Term();

/* Update DUI state, call once per frame.
 */
void DUI_Update();

/* Set the style.
 *
 * @param style: The DUI_Style to use
 */
void DUI_SetStyle(DUI_Style style);

/* Get a mutable reference to the current style.
 *
 * @return: A mutable reference to the current style.
 */
DUI_Style * DUI_GetStyle();

/* Handle a SDL Event.
 *
 * @param evt: The SDL_Event to process. The WindowID will be 
 *   checked against the window given to DUI_Init to limit what
 *   events are processed.
 */
void DUI_HandleEvent(SDL_Event * evt);

/* Move the DUI cursor.
 *
 * @param x: The new x coordinate. This will be used as the start
 *   of future lines when DUI_Newline is called.
 *
 * @param y: The new y coordinate.
 */
void DUI_MoveCursor(int x, int y);

/* Move the DUI cursor relative to its current position.
 *
 * @param dx: The value to add to the x coordinate. The resulting
 *   value will be used as the start of future lines when
 *   DUI_Newline is called.
 *
 * @param dy: The value to add to the y coordinate.
 */
void DUI_MoveCursorRelative(int dx, int dy);

/* Get the current cursor position.
 *
 * @param x: A pointer to the int to put the x coordinate in.
 * 
 * @param y: A pointer to the int to put the y coordinate in.
 */
void DUI_GetCursor(int * x, int * y);

/* Move the cursor to a new line.
 *
 * Reset the x coordinate to the beginning of the line, and
 *   increment the y by the LineHeight, set in the style.
 */
void DUI_Newline();

/* Print formatted text at the current cursor
 * The cursor will be moved to the end of the printed text.
 * Newlines ('\n') will call DUI_Newline.
 *
 * @param format: The format string to pass to vsnprintf.
 */
void DUI_Print(const char * format, ...);

/* Shortcut for calling both DUI_Print and DUI_Newline
 */
#define DUI_Println(format, ...)            \
    do {                                    \
        DUI_Print(format, ##__VA_ARGS__);   \
        DUI_Newline();                      \
    } while (0)

/* Print formatted text at the given location.
 *
 * The cursor will not be updated.
 *
 * @param x: The x coordinate.
 *
 * @param y: The y coordinate.
 *
 * @param format: The format string to pass to vsnprintf.
 */
#define DUI_PrintAt(x, y, format, ...)      \
    do {                                    \
        int tmpX, tmpY;                     \
        DUI_GetCursor(&tmpX, &tmpY);        \
        DUI_MoveCursor(x, y);               \
        DUI_Print(format, ##__VA_ARGS__);   \
        DUI_MoveCursor(tmpX, tmpY);         \
    } while (0)

/* Draw a bordered box, and move the cursor inside it.
 *
 * The border will be drawn with ColorBorder.
 * The background will be filled with ColorBackground.
 * The PanelPadding value will be added to the cursor's
 *   x and y coordinates.
 *
 * @param width: The width of the panel to draw.
 *
 * @param height: The height of the panel to draw.
 */
void DUI_Panel(int width, int height);

/* Draw a button with the specified text.
 *
 * The border will be drawn with ColorBorder.
 * The background will be filled with ColorHover if the
 *   mouse is over the button, or ColorDefault otherwise.
 * The ButtonPadding value will be added to the cursor's
 *   x and y coordinates before drawing text.
 * The ButtonMargin value will be added to the cursor's
 *   x coordinate after the button is drawn.
 *
 * @param text: The text to draw in the button.
 *
 * @return: True if the button is being clicked.
 */
bool DUI_Button(const char * text);

/* Draw a button at a specified location, with the given text.
 *
 * The cursor will not be updated.
 * 
 * @param x: The x coordinate.
 *
 * @param y: The y coordinate.
 *
 * @param text: The text to draw in the button.
 *
 * @return: True if the button is being clicked.
 */
bool DUI_ButtonAt(int x, int y, const char * text);

/* Draw a checkbox with the specified text.
 *
 * The border will be drawn with ColorBorder.
 * The background will be filled with ColorHover if the
 *   mouse is over the checkbox, or ColorDefault otherwise.
 * The ButtonPadding value will be added to the cursor's
 *   x and y coordinates before drawing text.
 * The ButtonMargin value will be added to the cursor's
 *   x coordinate after the checkbox is drawn.
 *
 * @param text: The text to draw in the checkbox.
 * 
 * @param checked: A pointer to a boolean to store the 
 *   state of the checkbox.
 *
 * @return: True if the checkbox is being clicked.
 */
bool DUI_Checkbox(const char * text, bool * checked);

/* Draw a checkbox at a specified location, with the given text.
 *
 * The cursor will not be updated.
 * 
 * @param x: The x coordinate.
 *
 * @param y: The y coordinate.
 *
 * @param text: The text to draw in the checkbox.
 * 
 * @param checked: A pointer to a boolean to store the 
 *   state of the checkbox.
 *
 * @return: True if the checkbox is checked.
 */
bool DUI_CheckboxAt(int x, int y, const char * text, bool * checked);

/* Draw a radio button with the specified text.
 *
 * The border will be drawn with ColorBorder.
 * The background will be filled with ColorHover if the
 *   mouse is over the radio button, or ColorDefault otherwise.
 * The ButtonPadding value will be added to the cursor's
 *   x and y coordinates before drawing text.
 * The ButtonMargin value will be added to the cursor's
 *   x coordinate after the radio button is drawn.
 *
 * @param text: The text to draw in the radio button.
 * 
 * @param index: The index of this radio button.
 * 
 * @param currentIndex: A pointer to the current index of this
 *   radio button group. If this radio button is clicked, the 
 *   currentIndex will be set to this radio button's index.
 *
 * @return: True if the radio button is selected.
 */
bool DUI_Radio(const char * text, int index, int * currentIndex);

/* Draw a radio button with the specified text.
 *
 * The cursor will not be updated.
 * 
 * @param x: The x coordinate.
 *
 * @param y: The y coordinate.
 *
 * @param text: The text to draw in the checkbox.
 * 
 * @param index: The index of this radio button.
 * 
 * @param currentIndex: A pointer to the current index of this
 *   radio button group. If this radio button is clicked, the 
 *   currentIndex will be set to this radio button's index.
 *
 * @return: True if the radio button is selected.
 */
bool DUI_RadioAt(int x, int y, const char * text, int index, int * currentIndex);

/* Prepares for tabs to be rendered.
 *
 * Saves the position of the next tab to be rendered, so that 
 *   render calls trigged by the tab being selected don't break it.
 */
void DUI_BeginTabBar();

/* Draw a tab with the specified text.
 *
 * The border will be drawn with ColorBorder.
 * The background will be filled with ColorHover if the
 *   mouse is over the tab, or ColorDefault otherwise.
 * The TabPadding value will be added to the cursor's
 *   x and y coordinates before drawing text.
 * The TabMargin value will be added to the cursor's
 *   x coordinate after the tab is drawn.
 *
 * @param text: The text to draw in the tab.
 * 
 * @param index: The index of this tab.
 * 
 * @param currentIndex: A pointer to the current index of this
 *   tab group. If this tab is clicked, the currentIndex will 
 *   be set to this tab's index.
 *
 * @return: True if the tab is selected.
 */
bool DUI_Tab(const char * text, int index, int * currentIndex);

#endif // DUI_H

#if defined(DUI_IMPLEMENTATION)

#include <DUI/DUI_font.h>

SDL_Window *   _duiWindow      = NULL;
SDL_Renderer * _duiRenderer    = NULL;
SDL_Texture *  _duiFontTexture = NULL;

int _duiWindowID = 0;

DUI_Style _duiStyle = {
    .LineHeight = 12,

    .PanelPadding = 8,

    .ButtonPadding = 4,
    .ButtonMargin  = 8,

    .TabPadding = 8,
    .TabMargin  = 8,

    .ColorBackground = { 0xEE, 0xEE, 0xEE, 0xFF },
    .ColorBorder     = { 0x00, 0x00, 0x00, 0xFF },
    .ColorHover      = { 0xEE, 0xEE, 0xEE, 0xEE },
    .ColorDefault    = { 0xAA, 0xAA, 0xAA, 0xAA },
};

int _duiLineStart = 0;

SDL_Point _duiMouse     = { 0, 0 };
SDL_Point _duiCursor    = { 0, 0 };
SDL_Point _duiTabCursor = { 0, 0 };

bool _duiMouseDown = false;
bool _duiClicked = false;

void DUI_setColorBackground()
{
    SDL_SetRenderDrawColor(_duiRenderer, 
        _duiStyle.ColorBackground[0],
        _duiStyle.ColorBackground[1],
        _duiStyle.ColorBackground[2],
        _duiStyle.ColorBackground[3]);
}

void DUI_setColorBorder()
{
    SDL_SetRenderDrawColor(_duiRenderer, 
        _duiStyle.ColorBorder[0],
        _duiStyle.ColorBorder[1],
        _duiStyle.ColorBorder[2],
        _duiStyle.ColorBorder[3]);
}

void DUI_setColorHover()
{
    SDL_SetRenderDrawColor(_duiRenderer, 
        _duiStyle.ColorHover[0],
        _duiStyle.ColorHover[1],
        _duiStyle.ColorHover[2],
        _duiStyle.ColorHover[3]);
}

void DUI_setColorDefault()
{
    SDL_SetRenderDrawColor(_duiRenderer, 
        _duiStyle.ColorDefault[0],
        _duiStyle.ColorDefault[1],
        _duiStyle.ColorDefault[2],
        _duiStyle.ColorDefault[3]);
}

void DUI_Init(SDL_Window * window)
{
    _duiWindow = window;
    _duiRenderer = SDL_GetRenderer(window);
    _duiWindowID = SDL_GetWindowID(window);

    SDL_RWops * fontMem = SDL_RWFromConstMem(DUI_FONT_BMP, sizeof(DUI_FONT_BMP));
    SDL_Surface * fontSurface = SDL_LoadBMP_RW(fontMem, 1);

    _duiFontTexture = SDL_CreateTextureFromSurface(_duiRenderer, fontSurface);
    SDL_FreeSurface(fontSurface);
}

void DUI_Term()
{
    SDL_DestroyTexture(_duiFontTexture);
}

void DUI_Update()
{
    int state = SDL_GetMouseState(&_duiMouse.x, &_duiMouse.y);
    bool pressed = (state & SDL_BUTTON(SDL_BUTTON_LEFT));
    _duiClicked = (pressed && !_duiMouseDown);
    _duiMouseDown = pressed;
}

void DUI_SetStyle(DUI_Style style)
{
    _duiStyle = style;
}

DUI_Style * DUI_GetStyle()
{
    return &_duiStyle;
}

void DUI_MoveCursor(int x, int y)
{
    _duiCursor.x = x;
    _duiCursor.y = y;
    _duiLineStart = _duiCursor.x;
}

void DUI_MoveCursorRelative(int dx, int dy)
{
    _duiCursor.x += dx;
    _duiCursor.y += dy;
    _duiLineStart = _duiCursor.x;
}

void DUI_GetCursor(int * x, int * y)
{
    *x = _duiCursor.x;
    *y = _duiCursor.y;
}

void DUI_Newline()
{
    _duiCursor.y += _duiStyle.LineHeight;
    _duiCursor.x = _duiLineStart;
}

void DUI_Print(const char * format, ...)
{
    static char buffer[1024];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    size_t length = strlen(buffer);

    SDL_Rect src = { 
        .x = 0,
        .y = 0,
        .w = DUI_CHAR_SIZE,
        .h = DUI_CHAR_SIZE,
    };

    SDL_Rect dst = { 
        .x = _duiCursor.x,
        .y = _duiCursor.y,
        .w = DUI_CHAR_SIZE,
        .h = DUI_CHAR_SIZE,
    };

    char * questionMark = strchr(DUI_FONT_MAP, '?');

    for (size_t i = 0; i < length; ++i) {
        if (buffer[i] == ' ') {
            dst.x += DUI_CHAR_SIZE;
            continue;
        }

        if (buffer[i] == '\n') {
            DUI_Newline();
            dst.x = _duiCursor.x;
            dst.y = _duiCursor.y;
        }

        char * index = strchr(DUI_FONT_MAP, toupper(buffer[i]));

        if (index == NULL) {
            index = questionMark;
        }

        size_t offset = index - DUI_FONT_MAP;

        src.x = (offset % DUI_CHAR_SIZE) * DUI_CHAR_SIZE;
        src.y = (offset / DUI_CHAR_SIZE) * DUI_CHAR_SIZE;
        SDL_RenderCopy(_duiRenderer, _duiFontTexture, &src, &dst);

        dst.x += DUI_CHAR_SIZE;
    }
}

void DUI_Panel(int width, int height)
{
    SDL_Rect bounds = {
        .x = _duiCursor.x,
        .y = _duiCursor.y,
        .w = width,
        .h = height,
    };

    DUI_setColorBackground();
    SDL_RenderFillRect(_duiRenderer, &bounds);

    DUI_setColorBorder();
    SDL_RenderDrawRect(_duiRenderer, &bounds);

    DUI_MoveCursorRelative(_duiStyle.PanelPadding, _duiStyle.PanelPadding);
}

bool DUI_Button(const char * text)
{
    int width = (strlen(text) * DUI_CHAR_SIZE) 
        + (_duiStyle.ButtonPadding * 2);

    int height = DUI_CHAR_SIZE 
        + (_duiStyle.ButtonPadding * 2);

    SDL_Rect bounds = {
        .x = _duiCursor.x,
        .y = _duiCursor.y,
        .w = width,
        .h = height,
    };

    bool hover = SDL_PointInRect(&_duiMouse, &bounds);
    bool clicked = (hover && _duiClicked);

    if (hover) {
        DUI_setColorHover();
    }
    else {
        DUI_setColorDefault();
    }

    SDL_RenderFillRect(_duiRenderer, &bounds);

    DUI_setColorBorder();
    SDL_RenderDrawRect(_duiRenderer, &bounds);

    _duiCursor.x += _duiStyle.ButtonPadding;
    _duiCursor.y += _duiStyle.ButtonPadding;

    DUI_Print("%s", text);

    _duiCursor.x = bounds.x + bounds.w + _duiStyle.ButtonMargin;
    _duiCursor.y = bounds.y;

    return clicked;
}

bool DUI_ButtonAt(int x, int y, const char * text)
{
    SDL_Point tmp = _duiCursor;
    DUI_MoveCursor(x, y);

    bool clicked = DUI_Button(text);

    DUI_MoveCursor(tmp.x, tmp.y);

    return clicked;
}

bool DUI_Checkbox(const char * text, bool * checked)
{
    int width = (strlen(text) * DUI_CHAR_SIZE) 
        + (DUI_CHAR_SIZE * 2)
        + (_duiStyle.ButtonPadding * 2);

    int height = DUI_CHAR_SIZE 
        + (_duiStyle.ButtonPadding * 2);

    SDL_Rect bounds = {
        .x = _duiCursor.x,
        .y = _duiCursor.y,
        .w = width,
        .h = height,
    };

    bool hover = SDL_PointInRect(&_duiMouse, &bounds);
    bool clicked = (hover && _duiClicked);

    if (hover) {
        DUI_setColorHover();
    }
    else {
        DUI_setColorDefault();
    }

    SDL_RenderFillRect(_duiRenderer, &bounds);

    DUI_setColorBorder();
    SDL_RenderDrawRect(_duiRenderer, &bounds);

    SDL_Rect mark = { 
        .x = bounds.x + DUI_CHAR_SIZE,
        .y = bounds.y + (DUI_CHAR_SIZE / 2),
        .w = DUI_CHAR_SIZE,
        .h = DUI_CHAR_SIZE,
    };

    SDL_RenderDrawRect(_duiRenderer, &mark);

    if (clicked) {
        *checked ^= true;
    }

    if (*checked) {
        ++mark.x;
        ++mark.y;
        mark.w -= 2;
        mark.h -= 2;

        SDL_RenderFillRect(_duiRenderer, &mark);
    }

    _duiCursor.x += _duiStyle.ButtonPadding + (DUI_CHAR_SIZE * 2);
    _duiCursor.y += _duiStyle.ButtonPadding;

    DUI_Print("%s", text);

    _duiCursor.x = bounds.x + bounds.w + _duiStyle.ButtonMargin;
    _duiCursor.y = bounds.y;

    return *checked;
}

bool DUI_CheckboxAt(int x, int y, const char * text, bool * checked)
{
    SDL_Point tmp = _duiCursor;
    DUI_MoveCursor(x, y);

    bool selected = DUI_Checkbox(text, checked);

    DUI_MoveCursor(tmp.x, tmp.y);

    return selected;
}

bool DUI_Radio(const char * text, int index, int * currentIndex)
{
    int width = (strlen(text) * DUI_CHAR_SIZE) 
        + (DUI_CHAR_SIZE * 2)
        + (_duiStyle.ButtonPadding * 2);

    int height = DUI_CHAR_SIZE 
        + (_duiStyle.ButtonPadding * 2);

    SDL_Rect bounds = {
        .x = _duiCursor.x,
        .y = _duiCursor.y,
        .w = width,
        .h = height,
    };

    bool hover = SDL_PointInRect(&_duiMouse, &bounds);
    bool clicked = (hover && _duiClicked);

    if (clicked) {
        *currentIndex = index;
    }

    bool active = (*currentIndex == index);

    if (hover) {
        DUI_setColorHover();
    }
    else {
        DUI_setColorDefault();
    }

    SDL_RenderFillRect(_duiRenderer, &bounds);

    DUI_setColorBorder();
    SDL_RenderDrawRect(_duiRenderer, &bounds);

    SDL_Rect mark = { 
        .x = bounds.x + DUI_CHAR_SIZE,
        .y = bounds.y + (DUI_CHAR_SIZE / 2),
        .w = DUI_CHAR_SIZE, 
        .h = DUI_CHAR_SIZE
    };

    SDL_RenderDrawRect(_duiRenderer, &mark);

    if (active) {
        ++mark.x;
        ++mark.y;
        mark.w -= 2;
        mark.h -= 2;

        SDL_RenderFillRect(_duiRenderer, &mark);
    }

    _duiCursor.x += _duiStyle.ButtonPadding + (DUI_CHAR_SIZE * 2);
    _duiCursor.y += _duiStyle.ButtonPadding;
    
    DUI_Print("%s", text);

    _duiCursor.x = bounds.x + bounds.w + _duiStyle.ButtonMargin;
    _duiCursor.y = bounds.y;

    return active;
}

bool DUI_RadioAt(int x, int y, const char * text, int index, int * currentIndex)
{
    SDL_Point tmp = _duiCursor;
    DUI_MoveCursor(x, y);

    bool selected = DUI_Radio(text, index, currentIndex);

    DUI_MoveCursor(tmp.x, tmp.y);

    return selected;
}

void DUI_BeginTabBar()
{
    _duiTabCursor = _duiCursor;
}

bool DUI_Tab(const char * text, int index, int * currentIndex)
{
    _duiCursor = _duiTabCursor;

    int width = (strlen(text) * DUI_CHAR_SIZE) 
        + (_duiStyle.TabPadding * 2);

    int height = DUI_CHAR_SIZE 
        + (_duiStyle.TabPadding * 2);

    SDL_Rect bounds = {
        .x = _duiCursor.x,
        .y = _duiCursor.y,
        .w = width,
        .h = height,
    };

    bool hover = SDL_PointInRect(&_duiMouse, &bounds);
    bool clicked = (hover && _duiClicked);

    if (clicked) {
        *currentIndex = index;
    }

    bool active = (*currentIndex == index);

    if (hover || active) {
        DUI_setColorHover();
    }
    else {
        DUI_setColorDefault();
    }

    SDL_RenderFillRect(_duiRenderer, &bounds);

    DUI_setColorBorder();
    SDL_RenderDrawRect(_duiRenderer, &bounds);

    _duiCursor.x += _duiStyle.TabPadding;
    _duiCursor.y += _duiStyle.TabPadding;
    
    DUI_Print("%s", text);

    _duiCursor.x = bounds.x + bounds.w + _duiStyle.TabMargin;
    _duiCursor.y = bounds.y;

    _duiTabCursor = _duiCursor;

    return active;
}

#endif