#include "../include/main.hpp"

#define FPS_REFRESH 1.0
#define FPS_PRINTLEN 9 // 8 chars + '\0+ '\0'
#define X_ORIGIN 30
#define Y_ORIGIN 30
#define MOVE_AMOUNT 10
#define ROTATE_AMOUNT 1
#define FLIGHT_MODE_MOVE 0.0009
#define IDLE_MOVEMENT (ROTATE_AMOUNT / 50)
#define COLOR_THRESHOLD 95
#define COLOR_MAX 255
#define FLIPPED_COORDINATES -1

using namespace std;

void move3dobject(Vector3d direction, std::vector<Polygon> &polygons)
{

    for (std::vector<Polygon>::iterator i = polygons.begin(); i != polygons.end(); ++i)
    {

        (*i).a += direction;
        (*i).b += direction;
        (*i).c += direction;
    }
}

Vector3d getCenter(std::vector<Polygon> &polygons)
{

    std::vector<double> xs, ys, zs;
    Vector3d center;
    int j = 0;

    //sum all vertex nodes up
    for (std::vector<Polygon>::iterator i = polygons.begin(); i != polygons.end(); ++i)
    {

        xs.push_back((*i).a.x);
        xs.push_back((*i).b.x);
        xs.push_back((*i).c.x);

        ys.push_back((*i).a.y);
        ys.push_back((*i).b.y);
        ys.push_back((*i).c.y);

        zs.push_back((*i).a.z);
        zs.push_back((*i).b.z);
        zs.push_back((*i).c.z);

        j++;
    }

    //calc median value to turn the objekt in-place and from a central point
    for (int k = 0; k < j; k++)
    {

        center.x += xs.back();
        center.y += ys.back();
        center.z += zs.back();
    }

    center.x /= j;
    center.y /= j;
    center.z /= j;

    return center;
}

void turn3dobject(Vector3d axis, double alpha, std::vector<Polygon> &polygons)
{

    Vector3d ba, ca, center;

    center = getCenter(polygons);

    //move the object to (0,0,0), apply turning matrix and then move back to original coordinates
    for (std::vector<Polygon>::iterator i = polygons.begin(); i != polygons.end(); ++i)
    {

        (*i).a -= center;
        (*i).b -= center;
        (*i).c -= center;

        (*i).a.turnonaxis(axis, alpha);
        (*i).b.turnonaxis(axis, alpha);
        (*i).c.turnonaxis(axis, alpha);

        (*i).a += center;
        (*i).b += center;
        (*i).c += center;

        //update n
        ba = (*i).a - (*i).b;
        ca = (*i).a - (*i).c;

        (*i).n = ba.cross_product(ca);
        (*i).n = (*i).n.normalise();
    }
}

int parse_file(const char *filename, std::vector<Polygon> &result, bool colorful)
{

    std::vector<Polygon> tmp;
    std::ifstream infile(filename);
    std::string str, garbage1, garbage2;

    //Debugging strings
    std::string facet("facet");
    std::string normal("normal");
    std::string vertex("vertex");
    std::string outer("outer");
    std::string loop("loop");
    std::string endloop("endloop");
    std::string endfacet("endfacet");
    std::string endsolid("endsolid");

    Vector3d n, a, b, c, ba, ca, offset, color = Vector3d(COLOR_MAX, 0, 0);
    double x, minx, maxx, y, miny, maxy, z, minz, maxz;
    long currentColor = 0;

    if (!infile)
    {
        return -1;
    }

    //initial "solid"
    getline(infile, str);

    /*
    * Now we will parse all polygons and add them to our container.
    * The file format is the following:
    * facet normal [x_n] [y_n] [z_n]
    *  outer loop
    *   vertex [v1_x] [v1_y] [v1_z]
    *   vertex [v2_x] [v2_y] [v2_z]
    *   vertex [v3_x] [v3_y] [v3_z]
    *  endloop
    * endfacet
    * 
    */

    while (getline(infile, str))
    {
        std::istringstream firstLine(str);
        firstLine >> garbage1 >> garbage2 >> x >> y >> z;

        //if garbage1 == 'endsolid' we are nicely done
        if (garbage1.compare(endsolid) == 0)
        {
            break;
        }

        //check if we parsed a line with 'facet' and 'normal'
        if ((garbage1.compare(facet) != 0) && garbage2.compare(normal) != 0)
        {
            return -1;
        }
        /*
        n.x = x;
        n.y = y;
        n.z = z;*/

        //get next line
        getline(infile, str);
        std::istringstream secondLine(str);
        secondLine >> garbage1 >> garbage2;

        //check if we parsed the line with 'outer loop'
        if ((garbage1.compare(outer) != 0) && garbage2.compare(loop) != 0)
        {
            return -1;
        }

        //get first vertex line
        getline(infile, str);
        std::istringstream thirdLine(str);
        thirdLine >> garbage1 >> x >> y >> z;

        //check if we parsed a line with 'vertex x y z'
        if ((garbage1.compare(vertex)) != 0)
        {
            return -1;
        }

        a.x = x;
        a.y = y;
        a.z = z;

        if (x < minx)
            minx = x;
        if (x > maxx)
            maxx = x;
        if (y < miny)
            miny = y;
        if (y > maxy)
            maxy = y;
        if (z < minz)
            minz = z;
        if (z > maxz)
            maxz = z;

        //get second vertex line
        getline(infile, str);
        std::istringstream fourthLine(str);
        fourthLine >> garbage1 >> x >> y >> z;

        //check if we parsed a line with 'vertex x y z'
        if ((garbage1.compare(vertex)) != 0)
        {
            return -1;
        }

        b.x = x;
        b.y = y;
        b.z = z;

        if (x < minx)
            minx = x;
        if (x > maxx)
            maxx = x;
        if (y < miny)
            miny = y;
        if (y > maxy)
            maxy = y;
        if (z < minz)
            minz = z;
        if (z > maxz)
            maxz = z;

        //get third vertex line
        getline(infile, str);
        std::istringstream fifthLine(str);
        fifthLine >> garbage1 >> x >> y >> z;

        //check if we parsed a line with 'vertex x y z'
        if ((garbage1.compare(vertex)) != 0)
        {
            return -1;
        }

        c.x = x;
        c.y = y;
        c.z = z;

        if (x < minx)
            minx = x;
        if (x > maxx)
            maxx = x;
        if (y < miny)
            miny = y;
        if (y > maxy)
            maxy = y;
        if (z < minz)
            minz = z;
        if (z > maxz)
            maxz = z;

        //get line with 'endloop'
        getline(infile, str);
        std::istringstream sixthLine(str);
        sixthLine >> garbage1;

        //check if we parsed a line with 'endloop'
        if ((garbage1.compare(endloop)) != 0)
        {
            return -1;
        }

        //get line with 'endfacet'
        getline(infile, str);
        std::istringstream seventhLine(str);
        seventhLine >> garbage1;

        //check if we parsed a line with 'endfacet'
        if ((garbage1.compare(endfacet)) != 0)
        {
            return -1;
        }

        //Since some models off the internet are faulty, we will calculate our normal vectors by ourselves.
        //update n
        ba = a - b;
        ca = a - c;

        n = ba.cross_product(ca);
        if (n.lenght() == 0)
        {
            cout << "Found a polygon with a normal vector of lenght 0. Skipping it.\n";
            continue;
        }
        n = n.normalise();

        //we succesfully parsed a whole Polygon. Lets append it on our container.
        if (colorful)
        {
            if (currentColor == 0)
            {
                // red colors
                color.x -= 1;
                if (color.x < COLOR_THRESHOLD)
                {
                    color.x = 0;
                    color.y = COLOR_MAX;
                    currentColor = 1;
                }
            }
            else if (currentColor == 1)
            {
                // green colors
                color.y -= 1;
                if (color.y < COLOR_THRESHOLD)
                {
                    color.y = 0;
                    color.z = COLOR_MAX;
                    currentColor = 2;
                }
            }
            else
            {
                // blue colors
                color.z -= 1;
                if (color.z < COLOR_THRESHOLD)
                {
                    color.z = 0;
                    color.x = COLOR_MAX;
                    currentColor = 0;
                }
            }
            tmp.push_back(Polygon(a, b, c, n, color, color, color));
        }
        else
        {
            tmp.push_back(Polygon(a, b, c, n));
        }
    }

    offset = Vector3d((X_ORIGIN - minx), (Y_ORIGIN - miny), (-1) * ((abs(maxz) - abs(minz)) / 2));

    for (Polygon &poly : tmp)
    {
        poly.a += offset;
        poly.b += offset;
        poly.c += offset;
    }

    result = tmp;

    return 1;
}

int main(int argc, char **argv)
{

    SDL_Window *window = NULL;
    SDL_Surface *screenSurface = NULL;
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "Error initialising SDL Library! SDL_ERROR: " << SDL_GetError() << endl;
        return 1;
    }
    else
    {
        //Create window
        window = SDL_CreateWindow("SDL Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN | /* SDL_WINDOW_HIDDEN |*/ SDL_WINDOW_INPUT_FOCUS);
        if (window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        }
        else
        {
            //Get window surface
            screenSurface = SDL_GetWindowSurface(window);

            //Fill the surface white
            SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));

            //Update the surface
            SDL_UpdateWindowSurface(window);

            std::cout << "Hiding window." << endl;
            //SDL_Delay(1000);
        }
    }

    // Parsed object vector
    std::vector<Polygon> polys;

    if (!(argc == 2 || (argc == 3 && (std::string(argv[2]).compare("-colorful") == 0))))
    {
        cout << "Please specifiy file to inspect. Usage: './render file.stl'" << endl;
        return 1;
    }

    if (std::string(argv[1]).compare("-help") == 0)
    {
        cout << "Usage: './render file.stl'. If you would like to randomly color polygons use './render file.stl -colorful'" << endl;
        return 0;
    }
    else if (parse_file(argv[1], polys, (argc == 3)) == -1)
    {
        cout << "Parsing of the file failed.\nPlease make sure file is in directory, and in ASCII STL format." << endl;
        return 1;
    }
    else
    {
        cout << "Parsed file succesfully! :-)" << endl;
    }

    // Initialise ncurses library and windows
    Windowmanager wm(SCREENHEIGHT, SCREENWIDTH, 10, 10);
    setlocale(LC_ALL, "");

    // Local vars
    int fps = 0, oldfps = 0;
    bool flight_mode = true;
    char buf[255];
    std::array<int, KEY_MAX> keyhits;
    keyhits.fill(0);
    auto start = std::chrono::system_clock::now();
    auto last = start;
    double deltaTime = 1, velocity = 1;
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    Vector3d planex = Vector3d(0, 1, 0), planey = Vector3d(0, 0, -1), planez = Vector3d(1, 0, 0);
    SDL_Event event;

    float movex_d = 0, movey_d = 0;
    Vector3d center_d = Vector3d(0, 0, 1);

    // Main loop
    while (true)
    {

        //refresh screen and get events
        refresh();
        SDL_PumpEvents();

        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                keyhits['q'] = 1;
                break;
            }
        }

        if (keys[SDL_SCANCODE_ESCAPE] == 1)
        {
            break;
        }
        if (keys[SDL_SCANCODE_F] == 1)
        {
            flight_mode = !flight_mode;
        }
        if (keys[SDL_SCANCODE_I])
        {

            double alpha = ROTATE_AMOUNT * deltaTime;
            turn3dobject(planez, alpha, polys);
            planex.turnonaxis(planez, alpha);
            planey.turnonaxis(planez, alpha);
        }
        if (keys[SDL_SCANCODE_K])
        {

            double alpha = -ROTATE_AMOUNT * deltaTime;
            turn3dobject(planez, alpha, polys);
            planex.turnonaxis(planez, alpha);
            planey.turnonaxis(planez, alpha);
        }
        if (keys[SDL_SCANCODE_U])
        {

            double alpha = ROTATE_AMOUNT * deltaTime;
            turn3dobject(planex, alpha, polys);
            planey.turnonaxis(planex, alpha);
            planez.turnonaxis(planex, alpha);
        }
        if (keys[SDL_SCANCODE_O])
        {

            double alpha = -ROTATE_AMOUNT * deltaTime;
            turn3dobject(planex, alpha, polys);
            planey.turnonaxis(planex, alpha);
            planez.turnonaxis(planex, alpha);
        }
        if (keys[SDL_SCANCODE_J])
        {

            double alpha = ROTATE_AMOUNT * deltaTime;
            turn3dobject(planey, alpha, polys);
            planex.turnonaxis(planey, alpha);
            planez.turnonaxis(planey, alpha);
        }
        if (keys[SDL_SCANCODE_L])
        {

            double alpha = -ROTATE_AMOUNT * deltaTime;
            turn3dobject(planey, alpha, polys);
            planex.turnonaxis(planey, alpha);
            planez.turnonaxis(planey, alpha);
        }
        if (keys[SDL_SCANCODE_W] == 1)
        {
            //move3dobject(Vector3d(0, -1, 0) * MOVE_AMOUNT * deltaTime, polys);
            velocity += 0.1;
        }
        /*if (keys[SDL_SCANCODE_A] == 1)
        {
            move3dobject(Vector3d(-1, 0, 0) * MOVE_AMOUNT * deltaTime, polys);
        }*/
        if (keys[SDL_SCANCODE_S] == 1)
        {
            //move3dobject(Vector3d(0, 1, 0) * MOVE_AMOUNT * deltaTime, polys);
            if(velocity > 1){
                velocity -= 0.1;
            }
        }/*
        if (keys[SDL_SCANCODE_D] == 1)
        {
            move3dobject(Vector3d(1, 0, 0) * MOVE_AMOUNT * deltaTime, polys);
        }*/
        if (flight_mode)
        {
            float direction_len, cosphi, angle;
            static Vector3d camera_angle(1, 0, 0), center;
            static float camera_len = camera_angle.lenght(), movex, movey;

            direction_len = planex.lenght();
            cosphi = camera_angle.scalar_product(planey) / (camera_len * direction_len);
            angle = acos(cosphi) * 180.0 / PI;

            movex = FLIPPED_COORDINATES * planex.x * FLIGHT_MODE_MOVE * angle * velocity;
            movey = FLIPPED_COORDINATES * planex.y * FLIGHT_MODE_MOVE * angle * velocity;
            center = getCenter(polys);

            if (((center.x < 0) && (movex < 0)) || ((center.x > SCREENWIDTH) && (movex > 0)))
                movex = 0;
            if (((center.y < 0) && (movey < 0)) || ((center.y > SCREENHEIGHT) && (movey > 0)))
                movey = 0;

            move3dobject(Vector3d(movex, movey, 0), polys);

            center_d = center;
            movex_d = movex;
            movey_d = movey;
        }

        // render current iteration
        render_object(wm, polys);

        // calc fps
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        deltaTime = std::chrono::duration<double>(end - last).count();
        last = end;

        if (elapsed.count() > FPS_REFRESH)
        {
            start = std::chrono::system_clock::now();
            oldfps = fps;
            fps = 0;
        }

        snprintf(buf, FPS_PRINTLEN, "fps: %3i", oldfps);
        wm.printxyc(0, 0, ColorLut::getInstance().rgb_to_8bit(0, 255, 0), COLOR_BLACK, true, buf);

        std::ostringstream stream;
        long distance = std::distance(polys.begin(), polys.end());
        stream << "Currently rendering " << distance << " Polygons per iteration. Center_d: " << center_d.to_string() << " movex: " << movex_d << " movey: " << movey_d << "\n";
        wm.printxyc(0, 1, ColorLut::getInstance().rgb_to_8bit(255, 0, 0), COLOR_BLACK, true, stream.str().c_str());

        ++fps;
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
