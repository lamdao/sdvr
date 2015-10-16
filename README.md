## SDVR - Simple Direct Volume Rendering
-----------------------------------------

### Showcase

<img src="http://lamdao.github.io/sdvr/sdvr.gif"/>

### Descriptions

This is a just-for-fun (JFF) project to demonstrate a Direct Volume Rendering
algorithm using CPU with SSE enabled. This pure Win32 application (no OpenGL
and/or DirectX needed) has a fairly complex rendering engine using MS Windows
multi-thread API. It also introduces a simple algorithm to draw bounding box
after rendering, and a simple trackball algorithm for natural volume rotation.
This project can be used for study or test Volume Rendering algorithm and
multi-thread tasks scheduling. A true real-time Direct Volume Rendering engine
using GPU will be posted in another project that has more advances in
visualization and simulation.

### Usage

SDVR has a simple GUI controlled mostly by hotkeys:

    F3      -- Open file dialog to load volume in 8-bit raw format
    + / -   -- Zoom In/Out
    < / >   -- Rotate the view to left / right
    ^ / v   -- Bring up / down background threshold
    0 / 1   -- Reset zoom scale to default
    2 - 4   -- Zoom In 2x, 3x, 4x times
    b / B   -- Change brightness
    f       -- Toggle fixed light source
    p       -- Switch between perspective and parallel view mode
    q / Q   -- Increase/decrease sampling step
    r       -- Reset view back to default
    s       -- Toggle shading
    x       -- Toggle bounding box

Some special functions must be activated by mouse:

    - Rotate view freely by hold down left mouse button and move
    - Zoom in/out with mouse wheel

### Build

This project can be compiled using Visual Studio 13 or higher. It also has
Makefile for cross compiling with mingw32/64 in Linux system (tested on
Fedora 22 with ming64-gcc 5.1.0). The Makefile can be modified easily to
use with MinGW on M$ Windows as well.

### JFF
Happy hacking and learning!!!
