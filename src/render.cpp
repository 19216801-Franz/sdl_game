#include "../include/render.hpp"

#define Z_PERSPECTIVE 0.01 
#define Z_PERSPECTIVE_COLOR 6

bool sortbyz(Polygon &lhs, Polygon &rhs)
{
    return lhs.getlastz() < rhs.getlastz();
}

int max(int a, int b, int c)
{
    return a > b ? (a > c ? a : c) : (b > c ? b : c);
}

int sgn(int x)
{
    return (x > 0) ? 1 : (x < 0) ? -1
                                 : 0;
}

void fast_bresenham(Windowmanager &wm, Vector3d &colorstart, Vector3d &colorend, int height, int xstart, int ystart, int zstart, int xend, int yend, int zend)
{

    int x, y, t, dx, dy, incx, incy, pdx, pdy, ddx, ddy, deltaslowdir, deltafastdir, err;
    short color;
    int zcolors = zstart, zcolore = zend;
    ColorLut &lut = ColorLut::getInstance();

    if (zstart > 0)
    {
        zcolors = 0;
    }
    else
    {
        zcolors *= Z_PERSPECTIVE_COLOR;
    }
    if (zend > 0)
    {
        zcolore = 0;
    }
    else
    {
        zcolore *= Z_PERSPECTIVE_COLOR;
    }

    // calc distances
    dx = xend - xstart;
    dy = yend - ystart;

    // check sign
    incx = sgn(dx);
    incy = sgn(dy);
    if (dx < 0)
        dx = -dx;
    if (dy < 0)
        dy = -dy;

    // find the fast direction
    if (dx > dy)
    {
        // x is fast
        pdx = incx;
        pdy = 0; //pd is parralel step and dd is diagonal step
        ddx = incx;
        ddy = incy;
        deltaslowdir = dy;
        deltafastdir = dx;
    }
    else
    {
        // y is fast
        pdx = 0;
        pdy = incy;
        ddx = incx;
        ddy = incy;
        deltaslowdir = dx;
        deltafastdir = dy;
    }

    // last initialisations
    x = xstart;
    y = ystart;
    err = deltafastdir / 2;

    color = lut.getInstance().rgb_to_8bit(colorstart.x + zcolors, colorstart.y + zcolors, colorstart.z + zcolors);
    wm.printxyc(2* x, y, color, color, true," ");
    wm.printxyc((2* x) +1, y, color, color, true," ");

    for (t = 0; t < deltafastdir; ++t)
    {

        // adjusting the error term
        err -= deltaslowdir;

        if (err < 0)
        {
            // we have to go a step in the slow direction
            err += deltafastdir;
            x += ddx;
            y += ddy;
        }
        else
        {
            // fast step
            x += pdx;
            y += pdy;
        }

        float fast = static_cast<float>(deltafastdir);
        float red = (((fast - t)/fast) * (colorstart.x + zcolors)) + ((t/fast) * colorend.x + zcolore);
        float green = (((fast - t)/fast) * (colorstart.y + zcolors)) + ((t/fast) * colorend.y + zcolore);
        float blue = (((fast - t)/fast) * (colorstart.z + zcolors)) + ((t/fast) * colorend.z + zcolore);
        color = lut.getInstance().rgb_to_8bit(red, green, blue);

        // rasterise the pixel
        wm.printxyc(2* x, y, color, color, true," ");
        wm.printxyc((2* x) +1, y, color, color, true," ");
    }
}

void fast_bresenham3d(Windowmanager &wm, Vector3d &start, Vector3d &end, Vector3d &colorstart, Vector3d &colorend){

    ColorLut &lut = ColorLut::getInstance();
/*

    int zcolors = start.z, zcolore = end.z;

    if (zcolors > 0)
    {
        zcolors = 0;
    }
    else
    {
        zcolors *= 5;
    }
    if (zcolore > 0)
    {
        zcolore = 0;
    }
    else
    {
        zcolore *= 5;
    }
*/


    int x0 = static_cast<int>(start.x), x1 = static_cast<int>(end.x), y0 = static_cast<int>(start.y), y1 = static_cast<int>(end.y), z0 = static_cast<int>(start.z), z1 = static_cast<int>(end.z);
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int dz = abs(z1 - z0), sz = z0 < z1 ? 1 : -1;
    int dm = max(dx,dy,dz), i = dm;
    x1 = y1 = z1 = dm/2;
    short color;

    for(;;){
/*
        float fast = static_cast<float>(dm);
        float red = ((i/fast) * (colorstart.x + zcolors)) + (((dm - i)/fast) * colorend.x + zcolore);
        float green = ((i/fast) * (colorstart.y + zcolors)) + (((dm - i)/fast) * colorend.y + zcolore);
        float blue = ((i/fast) * (colorstart.z + zcolors)) + (((dm - i)/fast) * colorend.z + zcolore);
        color = lut.getInstance().rgb_to_8bit(red, green, blue); */

        float fast = static_cast<float>(dm);
        float red = ((i/fast) * (colorstart.x + z1)) + (((dm - i)/fast) * colorend.x + z1);
        float green = ((i/fast) * (colorstart.y + z1)) + (((dm - i)/fast) * colorend.y + z1);
        float blue = ((i/fast) * (colorstart.z + z1)) + (((dm - i)/fast) * colorend.z + z1);
        color = lut.getInstance().rgb_to_8bit(red, green, blue); 

        // rasterise the pixel
        wm.printxyc(x1, y1, color, color, true," ");
        
        if(i-- == 0) break;

        x1 -= dx; if(x1 < 0){ x1 += dm; x0 += sx;}
        y1 -= dy; if(y1 < 0){ y1 += dm; y0 += sy;}
        z1 -= dz; if(z1 < 0){ z1 += dm; z0 += sz;}

    }

}

void render(Windowmanager &wm, Polygon poly)
{
    // rasterise the polygon from a-b, b-c and c-a
    fast_bresenham(wm, poly.a_color, poly.b_color, wm.screenheight, poly.a.x, poly.a.y, poly.a.z, poly.b.x, poly.b.y, poly.b.z);
    fast_bresenham(wm, poly.b_color, poly.c_color, wm.screenheight, poly.b.x, poly.b.y, poly.b.z, poly.c.x, poly.c.y, poly.c.z);
    fast_bresenham(wm, poly.c_color, poly.a_color, wm.screenheight, poly.c.x, poly.c.y, poly.c.z, poly.a.x, poly.a.y, poly.a.z);
    
//    fast_bresenham3d(wm, poly.a, poly.b, poly.a_color, poly.b_color);
 //   fast_bresenham3d(wm, poly.b, poly.c, poly.b_color, poly.c_color);
  //  fast_bresenham3d(wm, poly.c, poly.a, poly.c_color, poly.a_color);

    return;
}

void render_object(Windowmanager &wm, std::vector<Polygon> polygons)
{

    float angle, n_len, cosphi;
    static Vector3d camera_angle(0, 0, 1);
    static float camera_len = camera_angle.lenght();


    wm.clearscreen();
    // Sorting the polygons to start rendering from the back.
    std::sort(polygons.begin(), polygons.end(), sortbyz);

    for (std::vector<Polygon>::iterator i = polygons.begin(); i != polygons.end(); ++i)
    {
        //if the polygon's normale is facing away from us, it is a backside polygon, and we do not need to render it.
        n_len = (*i).n.lenght();
        cosphi = camera_angle.scalar_product((*i).n) / (camera_len * n_len);
        angle = acos(cosphi) * 180.0 / PI;
        if(angle >= 90){
            continue;
        }


        //if polygon is out of bounds, we can not draw it and skip.
        if (((*i).a.x < 0 && (*i).a.x > wm.screenwidth) && ((*i).b.x < 0 && (*i).b.x > wm.screenwidth) && ((*i).c.x < 0 && (*i).c.x > wm.screenwidth))
            continue;

        if (((*i).a.y < 0 && (*i).a.y > wm.screenheight) && ((*i).b.y < 0 && (*i).b.y > wm.screenheight) && ((*i).c.y < 0 && (*i).c.y > wm.screenheight))
            continue;

        render(wm, *i);
    }
    return;
}
