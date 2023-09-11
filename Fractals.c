#include <fxcg/display.h>
#include <fxcg/keyboard.h>

/***************************************************************
 *  Copyright (c) 2023 George Newman
 *
 *  Permission is hereby granted, free of charge, to any person
 *  obtaining a copy of this software and associated documentation
 *  files (the "Software"), to deal in the Software without
 *  restriction, including without limitation the rights to use,
 *  copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom
 *  the Software is furnished to do so, subject to the following
 *  conditions:
 *
 *  The above copyright notice and this permission notice shall
 *  be included in all copies or substantial portions of the Software.
 *
 *  Attribution to the original author, George Newman, must be
 *  provided by any persons or entities distributing or modifying
 *  the Software.
 *
 *  The references to the author's GitHub repository, as provided on
 *  the information page of the Software, must not be removed
 *  from the Software or any derivative works.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 ***************************************************************/


// Define constants
const int WIDTH = 383; // Screen width in pixels
const int HEIGHT = 215; // Screen height in pixels
const float X_OFFSET = 250; // Center renders in screen
const float Y_OFFSET = HEIGHT / 2.0 + 5; // Center renders in screen
const float ZOOM = 95.0; // Zoom into renders

// Define global variables
int MAX_ITERATIONS = 20; // 20 reccomended, 10 min, 100 max
int TRACE = 1; // 0 = off, 1 = on
int LIVE_RENDER = 1; // 0 = off, 1 = on
int AXIS = 0; // 0 = off, 1 = on
int ADVANCED_COLOUR = 1; // 0 = off, 1 = on

int key; // Used in almost all functions to get keypresses

// Define structure for a complex number
typedef struct
{
    double re;
    double im;
} complex;

// Function declarations
void main();
void editSettings();
void getInfo();
void drawAxis();

// Functions to do with rendering the Mandlebrot set
void renderMandlebrot();
void mandlebrotPixel(unsigned int x, unsigned int y);
int getColor(int iterations, int maxIterations);
double squaredAbs(complex z);

// Functions to do with the trace setting
void setTrace();
void drawTrace(unsigned int x, unsigned int y);

// Functions used by the trace and axis settings above
void Bdisp_DrawLine_VRAM(int x1, int y1, int x2, int y2, unsigned short color);
void Bdisp_EraseLine_VRAM(int x1, int y1, int x2, int y2);


// Calculate the absolute value of a complex number, without sqrt as this is a slow function and we can square the other side instead
double squaredAbs(complex z)
{
    return (z.re * z.re) + (z.im * z.im);
}

// Calcualte the remainder when a number is divided by a denominator
int modulo(int number, int denominator)
{
    return number - (denominator * (number/denominator));
}

// Use iteration ratio to return an appropriate RGB 565 value
int getColor(int iterations, int maxIterations)
{
    if (iterations == maxIterations)
    {
        // Points that reach max iterations are black
        return 0x0000;
    }
    else
    {
        // Map low iteration counts to shades of blue
        int blueShade = (iterations * 0x001F / maxIterations) << 5; 
        return 0x001F | (blueShade << 1); 
    }
}

void getInfo()
{
    Bdisp_AllClr_VRAM();

    // Setup header
    char color1 = TEXT_COLOR_WHITE;
    char color2 = TEXT_COLOR_WHITE;
    char msg[12] = "Information";
    DefineStatusMessage(&msg[0], 0, TEXT_COLOR_BLACK, 0);
    DefineStatusAreaFlags(4, SAF_BATTERY | SAF_TEXT | SAF_ALPHA_SHIFT, &color1, &color2);
    EnableDisplayHeader(2, 2);

    DefineStatusAreaFlags(4, SAF_BATTERY | SAF_TEXT | SAF_ALPHA_SHIFT, &color1, &color2);
    PrintXY(1, 1, "  GitHub Name:", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    PrintXY(1, 2, "  GeorgeNewman27", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    PrintXY(1, 4, "  GitHub Repo:", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    PrintXY(1, 5, "  FXCG50_Fractals", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    PrintXY(1, 6, "    >readme.md<  ", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    PrintXY(1, 8, "  EXIT: <- Menu     ", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    
    while(1)
    {
        GetKey(&key);
        // If key is EXIT  return to menu
        if (key == 0x7532)
        {
            main();
        }

        // If key is  left arrow, return to settings
        else if (key == 0x7544)
        {
            editSettings();
        }
    }  
}

// Open the settings page to adjust settings with F keys
void editSettings()
{
    Bdisp_AllClr_VRAM();
    
    // Setup header
    char color1 = TEXT_COLOR_WHITE;
    char color2 = TEXT_COLOR_WHITE;
    char msg[9] = "Settings";
    DefineStatusMessage(&msg[0], 0, TEXT_COLOR_BLACK, 0);
    DefineStatusAreaFlags(4, SAF_BATTERY | SAF_TEXT | SAF_ALPHA_SHIFT, &color1, &color2);
    EnableDisplayHeader(2, 2);

    
    // Prints appropriate trace status
    EnableDisplayHeader(2, 2);
    if (TRACE == 0)
    {
        PrintXY(1, 1, "  F1: Trace = OFF", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    }
    else if (TRACE == 1)
    {
        PrintXY(1, 1, "  F1: Trace = On ", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    }
    
    // Prints appropriate iteration status
    PrintXY(1, 2, "  F2: Max Iters =   ", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    char s[3];
    s[0] = 0x30 + (MAX_ITERATIONS - modulo(MAX_ITERATIONS, 10)) / 10;
    s[1] = 0x30 + modulo(MAX_ITERATIONS, 10);
    s[2] = '\0';
    locate_OS(17, 2);
    Print_OS(s, 0, 0);

    // Prints appropriate live render status
    if (LIVE_RENDER == 0)
    {
        PrintXY(1, 3, "  F3: Live Rdr = OFF", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    }
    else if (LIVE_RENDER == 1)
    {
        PrintXY(1, 3, "  F3: Live Rdr = ON ", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    }

    // Prints appropriate axis status
    if (AXIS == 0)
    {
        PrintXY(1, 4, "  F4: Axies = OFF", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    }
    else if (AXIS == 1)
    {
        PrintXY(1, 4, "  F4: Axies = ON ", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    }

    // Prints appropriate advanced colour status
    if (ADVANCED_COLOUR == 0)
    {
        PrintXY(1, 5, "  F5: Adv Col = OFF", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    }
    else if (ADVANCED_COLOUR == 1)
    {
        PrintXY(1, 5, "  F5: Adv Col = ON ", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    }

    PrintXY(1, 6, "  F6: Info ->", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    PrintXY(1, 8, "  EXIT: <- Menu     ", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);

    while(1)
    {
        key = 0;
        GetKey(&key);
        
        // If key is F1
        if (key == 0x7539 && TRACE == 0)
        {
            TRACE = 1;
            PrintXY(1, 1, "  F1: Trace = ON ", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
        }
        else if (key == 0x7539 && TRACE == 1)
        {
            TRACE = 0;
            PrintXY(1, 1, "  F1: Trace = OFF", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
        }


        // If key is F2
        else if (key == 0x753A)
        {
            PrintXY(17, 2, "    ", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
            locate_OS(17, 2);
            Cursor_SetFlashOn(8);
            int i = 0;
            int ret = 0;
            while(i != 2)
            {
                GetKey(&key);
                if (key >= 0x030 && key <= 0x039)
                {
                    ret = ret * 10 + key - 0x0030;
                    char c = key;
                    Print_OS(&c, 0, 0);
                    PrintXY(18 + i, 2, "      ", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
                    locate_OS(18 + i, 2);
                    i++;
                }
                else if (key == KEY_CTRL_EXE)
                {
                    i = 2;
                    break;
                }
            }

            if (ret > 9 && ret < 100)
            {
                MAX_ITERATIONS = ret;
            }
            else
            {
                PrintXY(1, 2, "  F2: Max Iters = 20", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
                MAX_ITERATIONS = 20;
            }

            Cursor_SetFlashOff();
        }

        // If key is F3
        else if (key == 0x753B && LIVE_RENDER == 0)
        {
            LIVE_RENDER = 1;
            PrintXY(1, 3, "  F3: Live Rdr = ON ", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
        }
        else if (key == 0x753B && LIVE_RENDER == 1)
        {
            LIVE_RENDER = 0;
            PrintXY(1, 3, "  F3: Live Rdr = OFF", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
        }

        // If key is F4
        else if (key == 0x753C && AXIS == 0)
        {
            AXIS = 1;
            PrintXY(1, 4, "  F4: Axies = ON ", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
        }
        else if (key == 0x753C && AXIS == 1)
        {
            AXIS = 0;
            PrintXY(1, 4, "  F4: Axies = OFF", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
        }

        // If key is F5
        else if (key == 0x753D && ADVANCED_COLOUR == 0)
        {
            ADVANCED_COLOUR = 1;
            Bdisp_EnableColor(ADVANCED_COLOUR);
            PrintXY(1, 5, "  F5: Adv Col = ON ", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
        }
        else if (key == 0x753D && ADVANCED_COLOUR == 1)
        {
            ADVANCED_COLOUR = 0;
            Bdisp_EnableColor(ADVANCED_COLOUR);
            PrintXY(1, 5, "  F5: Adv Col = OFF", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
        }

        // If key if F6
        else if (key == 0x753E || key == 0x7545)
        {
            getInfo();
        }

        // If key is EXIT or left arrow, return to menu
        else if (key == 0x7532 || key == 0x7544)
        {
            main();
        }
    }
}

// Render the mandlebrot set
void renderMandlebrot()
{
    Bdisp_EnableColor(ADVANCED_COLOUR);

    //Clear VRAM ready to write to
    Bdisp_AllClr_VRAM();

    // Setup header
    char color1 = TEXT_COLOR_WHITE;
    char color2 = TEXT_COLOR_WHITE;
    char msg[11] = "Mandelbrot";
    DefineStatusMessage(&msg[0], 0, TEXT_COLOR_BLACK, 0);
    DefineStatusAreaFlags(4, SAF_BATTERY | SAF_TEXT | SAF_ALPHA_SHIFT, &color1, &color2);
    DisplayStatusArea();

    if (LIVE_RENDER == 0)
    {
        PrintXY(7, 4, "  Rendering...", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    }

    Bdisp_PutDisp_DD();

    // Iterate over each pixel on screen
    for (unsigned int y = 24; y <= HEIGHT; y++)
    {
        for (unsigned int x = 0; x <= WIDTH; x++)
        {
            mandlebrotPixel(x, y);
        }

        // Make the loading icon move once
        HourGlass();

        if (LIVE_RENDER == 1)
        {
            // Force display VRAM strip
            Bdisp_PutDisp_DD_stripe(y, y);
        }
    }

    if (LIVE_RENDER == 0)
    {
        Bdisp_PutDisp_DD();
    }

    // Draw axies
    if (AXIS == 1)
    {   
        drawAxis();
    }
    
    // Prepare cursor system for trace
    if (TRACE == 1)
    {
        setTrace();
    }

    // Exit back to main menu, clearing VRAM in proscess
    while(1)
    {
        GetKey(&key);
        if (key == 0x7532)
        {
            main();
        }
    }
}

// Makes cursor that you can move around the screen and make a trace from
void setTrace()
{
    int cx = X_OFFSET;
    int cy = Y_OFFSET;
    unsigned short tmpcol = Bdisp_GetPoint_VRAM(cx, cy);

    // Draw cursor at origin
    Bdisp_SetPoint_VRAM(cx, cy, 0xf800);

    while(1)
    {
        GetKey(&key);

        // If up, move cursor up
        if (key == 0x7542)
        {
            Bdisp_SetPoint_VRAM(cx, cy, tmpcol);
            cy--;
            tmpcol = Bdisp_GetPoint_VRAM(cx, cy);
            Bdisp_SetPoint_VRAM(cx, cy, 0xf800);
            Bdisp_PutDisp_DD_stripe(cy, cy + 1);
        }

        // If right move cursor right
        else if (key == 0x7545)
        {
            Bdisp_SetPoint_VRAM(cx, cy, tmpcol);
            cx++;
            tmpcol = Bdisp_GetPoint_VRAM(cx, cy);
            Bdisp_SetPoint_VRAM(cx, cy, 0xf800);
            Bdisp_PutDisp_DD_stripe(cy, cy);
        }

        // If down move cursor down
        else if (key == 0x7547)
        {
            Bdisp_SetPoint_VRAM(cx, cy, tmpcol);
            cy++;
            tmpcol = Bdisp_GetPoint_VRAM(cx, cy);
            Bdisp_SetPoint_VRAM(cx, cy, 0xf800);
            Bdisp_PutDisp_DD_stripe(cy - 1, cy);
        }

        // If left move cursor left
        else if (key == 0x7544)
        {
            Bdisp_SetPoint_VRAM(cx, cy, tmpcol);
            cx--;
            tmpcol = Bdisp_GetPoint_VRAM(cx, cy);
            Bdisp_SetPoint_VRAM(cx, cy, 0xf800);
            Bdisp_PutDisp_DD_stripe(cy, cy);
        }

        // If exe, draw trace
        else if (key == KEY_CTRL_EXE)
        {
            drawTrace(cx, cy);
        }

        // If key is exit, return to menu
        else if (key == 0x7532)
        {
            main();
        }
    }
}

// Draws trace from point selected by cursor
void drawTrace(unsigned int x, unsigned int y)
{
    complex z1;
    complex z2;
    complex c;
    int iterations = 0;

    // Set Z to origin 
    z1.re = 0; 
    z1.im = 0;

    c.re = (x - X_OFFSET) / ZOOM;
    c.im = (y - Y_OFFSET) / ZOOM;

    while (iterations < MAX_ITERATIONS && squaredAbs(z1) <= 4)
    {
        z2.re = (z1.re * z1.re) - (z1.im * z1.im) + c.re;
        z2.im = (2 * z1.re * z1.im) + c.im;

        Bdisp_DrawLine_VRAM(z1.re * ZOOM + X_OFFSET, z1.im * ZOOM + Y_OFFSET, z2.re * ZOOM + X_OFFSET, z2.im * ZOOM + Y_OFFSET, 0x07e0);
        Bdisp_SetPoint_VRAM(z1.re * ZOOM + X_OFFSET, z1.im * ZOOM + Y_OFFSET, 0xf800);
        Bdisp_SetPoint_VRAM(z2.re * ZOOM + X_OFFSET, z2.im * ZOOM + Y_OFFSET, 0xf800);

        z1 = z2;
        iterations++;
    }

    while(1)
    {
        GetKey(&key);

        if ((key > 0x7542 && key < 0x7547) || key == KEY_CTRL_EXE)
        {
            z1.re = 0; 
            z1.im = 0;
            iterations = 0;

            c.re = (x - X_OFFSET) / ZOOM;
            c.im = (y - Y_OFFSET) / ZOOM;

            while (iterations < MAX_ITERATIONS && squaredAbs(z1) <= 4)
            {
                z2.re = (z1.re * z1.re) - (z1.im * z1.im) + c.re;
                z2.im = (2 * z1.re * z1.im) + c.im;

                Bdisp_EraseLine_VRAM(z1.re * ZOOM + X_OFFSET, z1.im * ZOOM + Y_OFFSET, z2.re * ZOOM + X_OFFSET, z2.im * ZOOM + Y_OFFSET);

                z1 = z2;
                iterations++;
            }

            Bdisp_SetPoint_VRAM(x, y, 0xf800);

            return;
        }

        else if (key == 0x7532)
        {
            main();
        }
    }
}

// Draws fully scaled axies over the top of a render
void drawAxis()
{
    // Draw Axis Lines
    Bdisp_DrawLine_VRAM(0, Y_OFFSET, WIDTH, Y_OFFSET, 0xFFFF);
    Bdisp_DrawLine_VRAM(X_OFFSET, 0, X_OFFSET, HEIGHT, 0xFFFF);

    // Draw Origin
    Bdisp_DrawLine_VRAM(X_OFFSET - 1, Y_OFFSET + 1, X_OFFSET + 1, Y_OFFSET + 1, 0xFFFF);
    Bdisp_DrawLine_VRAM(X_OFFSET - 1, Y_OFFSET - 1, X_OFFSET + 1, Y_OFFSET - 1, 0xFFFF);

    // Draw unit increments
    for (int i = -1; i <= 1; i++)
    {
        Bdisp_DrawLine_VRAM(X_OFFSET - 3, Y_OFFSET + i * ZOOM, X_OFFSET + 3, Y_OFFSET + i * ZOOM, 0xFFFF);
        Bdisp_DrawLine_VRAM(X_OFFSET + i * ZOOM, Y_OFFSET - 3, X_OFFSET + i * ZOOM, Y_OFFSET + 3, 0xFFFF);
        Bdisp_DrawLine_VRAM(X_OFFSET - 2, Y_OFFSET + i * ZOOM - 0.5 * ZOOM, X_OFFSET + 2, Y_OFFSET + i * ZOOM - 0.5 * ZOOM, 0xFFFF);
        Bdisp_DrawLine_VRAM(X_OFFSET + i * ZOOM - 0.5 * ZOOM, Y_OFFSET - 2, X_OFFSET + i * ZOOM - 0.5 * ZOOM, Y_OFFSET + 2, 0xFFFF);
    }
    Bdisp_DrawLine_VRAM(X_OFFSET - 2 * ZOOM, Y_OFFSET - 3, X_OFFSET - 2 * ZOOM, Y_OFFSET + 3, 0xFFFF);
    Bdisp_DrawLine_VRAM(X_OFFSET - 2 * ZOOM - 0.5 * ZOOM, Y_OFFSET - 2, X_OFFSET - 2 * ZOOM - 0.5 * ZOOM, Y_OFFSET + 2, 0xFFFF);
}

// Draw the correct colour of a pixel of the mandlebrot set given pixel coords
void mandlebrotPixel(unsigned int x, unsigned int y)
{
    complex z;
    complex c;
    complex tmp;
    int iterations = 0;   

    // Set Z to origin 
    z.re = 0; 
    z.im = 0;

    // Set parameter to pixel
    c.re = (x - X_OFFSET) / ZOOM;
    c.im = (y - Y_OFFSET) / ZOOM;

    while (iterations < MAX_ITERATIONS && squaredAbs(z) <= 4)
    {
        //Perform Z_(n+1) = Z_(n)^2 + c
        tmp.re = (z.re * z.re) - (z.im * z.im) + c.re;
        tmp.im = (2 * z.re * z.im) + c.im;

        //Update Z_(n+1) to equal z_(n)
        z = tmp;
        iterations++;
    }

    //Colour each pixel
    Bdisp_SetPoint_VRAM(x, y, getColor(iterations, MAX_ITERATIONS));

    return;
}

void main(void) 
{
    //Clear VRAM ready to write to
    Bdisp_AllClr_VRAM();

    // Bypass 3 bit 8 colour mode
    Bdisp_EnableColor(ADVANCED_COLOUR);

    // Setup header
    char color1 = TEXT_COLOR_WHITE;
    char color2 = TEXT_COLOR_WHITE;
    char msg[5] = "Menu";
    DefineStatusMessage(&msg[0], 0, TEXT_COLOR_BLACK, 0);
    DefineStatusAreaFlags(4, SAF_BATTERY | SAF_TEXT | SAF_ALPHA_SHIFT, &color1, &color2);
    EnableDisplayHeader(2, 2);

    PrintXY(1, 1, "  F1: Mandelbrot", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    PrintXY(1, 8, "  F6: Settings ->", TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);

    while(1)
    {
        GetKey(&key);

        // If key is F1, render mandelbrot
        if (key == 0x7539)
        {
            renderMandlebrot();
        }

        // If key is F6, render mandlebrot
        else if (key == 0x753E|| key == 0x7545)
        {
            editSettings();
        }

        // This should never be pressed, but if they do then this will force quit
        else if (key == KEY_CTRL_EXE) 
        {
            break;
        }
    }
    return; // Main can never return, so this call should never be reached
}

// A function I modified from here https://prizm.cemetech.net/Useful_Routines/DrawLine/ by Christopher Mitchell
void Bdisp_DrawLine_VRAM(int x1, int y1, int x2, int y2, unsigned short color) 
{
    signed char ix;
    signed char iy;

    int delta_x = (x2 > x1?(ix = 1, x2 - x1):(ix = -1, x1 - x2)) << 1;
    int delta_y = (y2 > y1?(iy = 1, y2 - y1):(iy = -1, y1 - y2)) << 1;
 
    Bdisp_SetPoint_VRAM(x1, y1, color); 
    if (delta_x >= delta_y) 
    {
        int error = delta_y - (delta_x >> 1);        
        while (x1 != x2) 
        {
            if (error >= 0) 
            {
                if (error || (ix > 0)) 
                {
                    y1 += iy;
                    error -= delta_x;
                }                           
            }
                                          
        x1 += ix;
        error += delta_y;
        Bdisp_SetPoint_VRAM(x1, y1, color);   
        }
    } 
    else 
    {
        int error = delta_x - (delta_y >> 1);      
        while (y1 != y2) 
        {
            if (error >= 0) 
            {
                if (error || (iy > 0)) 
                {
                    x1 += ix;
                    error -= delta_y;
                }                           
            }                              
        y1 += iy;
        error += delta_x;  
        Bdisp_SetPoint_VRAM(x1, y1, color);
        }
    }
}

// A function I modified from here https://prizm.cemetech.net/Useful_Routines/DrawLine/ by Christopher Mitchell
void Bdisp_EraseLine_VRAM(int x1, int y1, int x2, int y2) 
{
    signed char ix;
    signed char iy;

    int delta_x = (x2 > x1?(ix = 1, x2 - x1):(ix = -1, x1 - x2)) << 1;
    int delta_y = (y2 > y1?(iy = 1, y2 - y1):(iy = -1, y1 - y2)) << 1;
 
    mandlebrotPixel(x1 , y1);
    if (delta_x >= delta_y) 
    {
        int error = delta_y - (delta_x >> 1);        
        while (x1 != x2) 
        {
            if (error >= 0) 
            {
                if (error || (ix > 0)) 
                {
                    y1 += iy;
                    error -= delta_x;
                }                          
            }
                                        
        x1 += ix;
        error += delta_y;
        mandlebrotPixel(x1 , y1);   
        }
    } 
    else 
    {
        int error = delta_x - (delta_y >> 1);      
        while (y1 != y2) 
        {
            if (error >= 0) 
            {
                if (error || (iy > 0)) 
                {
                    x1 += ix;
                    error -= delta_y;
                }                           
            }                              
        y1 += iy;
        error += delta_x;  
        mandlebrotPixel(x1 , y1);
        }
    }

    if (AXIS == 1)
    {
        drawAxis();
    }

    return;
}