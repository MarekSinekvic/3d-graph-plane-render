#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <math.h>
#include <vector>
#include <fstream>
#include <thread>

using namespace sf;
using namespace std;

const float PI = 3.14159265358979323846f;
int WIDTH = 1200;
int HEIGHT = 720;
Font defaultFont;
int frame = 0;

#include "../../Libraries/Library.h"
Camera3d defaultCam = Camera3d(Vector3ff(0, 2, -5));
;
Vector3ff lightDirection = Vector3ff(60, -100, 60);

CircleShape sunLightDirection;

float sinEq(float x, float y)
{
    // return powf(2.718281f, x * y);
    // return 1 / (1 + x * x);
    return sinf(sqrtf(x * x + y * y + frame * 5));
    return 2 / ((1 + powf(x, 2)) * (1 + powf(y, 2)));
}
float eqsxy(float x, float y)
{
    return sin(x) * sin(y);
}
float eqcxy(float x, float y)
{
    return cos(x) * cos(y);
}
float eqcxsy(float x, float y)
{
    return cos(x) * sin(y);
}
float eqsxcy(float x, float y)
{
    return sin(x) * cos(y);
}
float eqcxsy_plus_sxcy(float x, float y)
{
    return cos(x) * sin(y) + sin(x) * cos(y);
}
float angularSpace(float x, float y)
{
    return sqrtf(1.0f - powf(cosf(x) * y, 2.0f) - powf(sinf(x) * y, 2.0f));
}
string slice(string s, int from, int to)
{
    int j = 0;
    for (int i = from; i <= to; ++i)
        s[j++] = s[i];
    s[j] = 0;
    return s;
};
class DepthBuffer
{
public:
    vector<vector<float>> depth;
    DepthBuffer()
    {
    }
    void clear()
    {
        for (int y = 0; y < this->depth.size(); y++)
        {
            for (int x = 0; x < this->depth[y].size(); x++)
            {
                depth[y][x] = 0;
            }
        }
    }
};
DepthBuffer depths = DepthBuffer();
class Equation
{
public:
    float x, y;
    float (*fcnPtr)(float, float);
    Equation(float (*fcnPtr)(float, float))
    {
        this->fcnPtr = fcnPtr;
    }
    float operator()(float x, float y)
    {
        return (*fcnPtr)(x, y);
    }
};
class Graph3D
{
private:
    Vertex xLine[2], yLine[2], zLine[2];
    vector<vector<Vertex>> separatorLines;

    Text xScaleDisplay, yScaleDisplay, zScaleDisplay;

    Camera3d *renderCam;
    RenderWindow *window;

    void DrawAxisLines()
    {
        xScaleDisplay.setCharacterSize(60.0f / (renderCam->position - Vector3ff(1, 0, 0)).magnitude());
        xScaleDisplay.setString(to_string(floorf(this->scale.x * 100) / 100));
        xScaleDisplay.setPosition(renderCam->ProjectToCanvas(Vector3ff(1, 0, 0)));

        yScaleDisplay.setCharacterSize(60.0f / (renderCam->position - Vector3ff(0, 1, 0)).magnitude());
        yScaleDisplay.setString(to_string(floorf(this->scale.y * 100) / 100));
        yScaleDisplay.setPosition(renderCam->ProjectToCanvas(Vector3ff(0, 1, 0)));

        zScaleDisplay.setCharacterSize(60.0f / (renderCam->position - Vector3ff(0, 0, 1)).magnitude());
        zScaleDisplay.setString(to_string(floorf(this->scale.z * 100) / 100));
        zScaleDisplay.setPosition(renderCam->ProjectToCanvas(Vector3ff(0, 0, 1)));

        xLine[0] = Vertex(this->renderCam->ProjectToCanvas(Vector3ff(1, 0, 0)), Color::Red);
        xLine[1] = Vertex(this->renderCam->ProjectToCanvas(Vector3ff(-1, 0, 0)), Color::Red);
        (*window).draw(xLine, 2, Lines);

        yLine[0] = Vertex(this->renderCam->ProjectToCanvas(Vector3ff(0, 1, 0)), Color::Green);
        yLine[1] = Vertex(this->renderCam->ProjectToCanvas(Vector3ff(0, -1, 0)), Color::Green);
        (*window).draw(yLine, 2, Lines);

        zLine[0] = Vertex(this->renderCam->ProjectToCanvas(Vector3ff(0, 0, 1)), Color::Blue);
        zLine[1] = Vertex(this->renderCam->ProjectToCanvas(Vector3ff(0, 0, -1)), Color::Blue);
        (*window).draw(zLine, 2, Lines);

        window->draw(xScaleDisplay);
        window->draw(yScaleDisplay);
        window->draw(zScaleDisplay);
    }
    void DrawSeparatorLines()
    {
        separatorLines.clear();

        int xSeparatorsCount = (this->scale.x < 20 ? this->scale.x : 20);
        int ySeparatorsCount = (this->scale.y < 20 ? this->scale.y : 20);
        int zSeparatorsCount = (this->scale.z < 20 ? this->scale.z : 20);
        for (int i = 0; i < xSeparatorsCount; i++)
        {
            float t = 1.0f / (float)xSeparatorsCount * i;

            Vector2f up = this->renderCam->ProjectToCanvas(this->position + Vector3ff(t, 0.03, 0));
            Vector2f down = this->renderCam->ProjectToCanvas(this->position + Vector3ff(t, -0.03, 0));
            vector<Vertex> v = {Vertex(up, Color::Red), Vertex(down, Color::Red)};
            this->separatorLines.push_back(v);

            up = this->renderCam->ProjectToCanvas(this->position + Vector3ff(-t, 0.03, 0));
            down = this->renderCam->ProjectToCanvas(this->position + Vector3ff(-t, -0.03, 0));
            v = {Vertex(up, Color::Red), Vertex(down, Color::Red)};
            this->separatorLines.push_back(v);
        }
        for (int i = 0; i < ySeparatorsCount; i++)
        {
            float t = 1.0f / (float)ySeparatorsCount * i;

            Vector2f up = this->renderCam->ProjectToCanvas(this->position + Vector3ff(0.03, t, 0));
            Vector2f down = this->renderCam->ProjectToCanvas(this->position + Vector3ff(-0.03, t, 0));
            vector<Vertex> v = {Vertex(up, Color::Green), Vertex(down, Color::Green)};
            this->separatorLines.push_back(v);

            up = this->renderCam->ProjectToCanvas(this->position + Vector3ff(0.03, -t, 0));
            down = this->renderCam->ProjectToCanvas(this->position + Vector3ff(-0.03, -t, 0));
            v = {Vertex(up, Color::Green), Vertex(down, Color::Green)};
            this->separatorLines.push_back(v);
        }
        for (int i = 0; i < zSeparatorsCount; i++)
        {
            float t = 1.0f / (float)zSeparatorsCount * i;

            Vector2f up = this->renderCam->ProjectToCanvas(this->position + Vector3ff(0, 0.03, t));
            Vector2f down = this->renderCam->ProjectToCanvas(this->position + Vector3ff(0, -0.03, t));
            vector<Vertex> v = {Vertex(up, Color::Blue), Vertex(down, Color::Blue)};
            this->separatorLines.push_back(v);

            up = this->renderCam->ProjectToCanvas(this->position + Vector3ff(0, 0.03, -t));
            down = this->renderCam->ProjectToCanvas(this->position + Vector3ff(0, -0.03, -t));
            v = {Vertex(up, Color::Blue), Vertex(down, Color::Blue)};
            this->separatorLines.push_back(v);
        }

        for (int i = 0; i < this->separatorLines.size(); i++)
        {
            Vertex l[2];
            copy(this->separatorLines[i].begin(), this->separatorLines[i].end(), l);
            window->draw(l, 2, Lines);
        }
    }
    void DrawEquationsAsLines()
    {
        float step = 0.035;
        for (int i = 0; i < this->equations.size(); i++)
        {
            for (float z = -this->scale.z; z < this->scale.z; z += step * this->scale.y)
            {
                vector<Vertex> lines;
                Vector2f pastPos;
                for (float x = -this->scale.x; x < this->scale.x; x += step * this->scale.z)
                {
                    Vector3ff position = Vector3ff(x / this->scale.x, this->equations[i](x, z) / this->scale.y, z / this->scale.z);
                    Vector2f viewPos = (renderCam->ProjectToCanvas(position));

                    lines.push_back(Vertex(viewPos, Color::White));
                    if (x != -this->scale.x)
                        lines.push_back(Vertex(pastPos, Color::White));
                    pastPos = viewPos;
                }
                Vertex arr_lines[lines.size()];
                copy(lines.begin(), lines.end(), arr_lines);
                window->draw(arr_lines, lines.size(), Lines);
            }
            for (float z = -this->scale.z; z < this->scale.z; z += step * this->scale.y)
            {
                vector<Vertex> lines;
                Vector2f pastPos;
                for (float x = -this->scale.x; x < this->scale.x; x += step * this->scale.z)
                {
                    Vector3ff position = Vector3ff(z / this->scale.x, this->equations[i](z, x) / this->scale.y, x / this->scale.z);
                    Vector2f viewPos = (renderCam->ProjectToCanvas(position));

                    lines.push_back(Vertex(viewPos, Color::White));
                    if (x != -this->scale.x)
                        lines.push_back(Vertex(pastPos, Color::White));
                    pastPos = viewPos;
                }
                Vertex arr_lines[lines.size()];
                copy(lines.begin(), lines.end(), arr_lines);
                window->draw(arr_lines, lines.size(), Lines);
            }
        }
    }
    // thread thread1;
    vector<Vector3ff> getGridLine(int *i, float *z, float stepX)
    {
        vector<Vector3ff> eqGridLine;
        for (float x = -this->scale.x; x < this->scale.x; x += stepX)
        {
            Vector3ff position = Vector3ff(x / this->scale.x, this->equations[*i](x, *z) / this->scale.y, *z / this->scale.z);

            eqGridLine.push_back(position);
        }
        return eqGridLine;
        // return eqGridLine;
    }
    void DrawEquationsAsPlane()
    {
        int i;
        float z;

        float stepZ = 0.025 * this->scale.y;
        float stepX = 0.025 * this->scale.x;
        for (i = 0; i < this->equations.size(); i++)
        {
            Clock t = Clock();
            vector<vector<Vector3ff>> eqGrid;
            for (z = -this->scale.z; z < this->scale.z; z += stepZ)
            {
                vector<Vector3ff> eqGridLine;
                // for (float x = -this->scale.x; x < this->scale.x; x += stepX)
                // {
                //     Vector3ff position = Vector3ff(x / this->scale.x, this->equations[i](x, z) / this->scale.y, z / this->scale.z);

                //     eqGridLine.push_back(position);
                // }

                eqGridLine = this->getGridLine(&i, &z, stepX);
                eqGrid.push_back(eqGridLine);
            }
            // cout << to_string(t.getElapsedTime().asMilliseconds()) << " to memorize" << endl;
            t.restart();
            int vertsCount = (eqGrid.size() - 1) * (eqGrid[0].size() - 1) * 3 * 2;
            VertexArray triangles(Triangles, vertsCount);
            for (int z = 0, i = 0; z < eqGrid.size() - 1; z++)
            {
                for (int x = 0; x < eqGrid[z].size() - 1; x++, i++)
                {
                    Vector2f viewPos11 = renderCam->ProjectToCanvas(eqGrid[z][x]);
                    Vector2f viewPos12 = renderCam->ProjectToCanvas(eqGrid[z][x + 1]);
                    Vector2f viewPos13 = renderCam->ProjectToCanvas(eqGrid[z + 1][x]);
                    if ((viewPos11.x >= 0 && viewPos11.x <= WIDTH && viewPos11.y >= 0 && viewPos11.y <= HEIGHT) ||
                        (viewPos12.x >= 0 && viewPos12.x <= WIDTH && viewPos12.y >= 0 && viewPos12.y <= HEIGHT) ||
                        (viewPos13.x >= 0 && viewPos13.x <= WIDTH && viewPos13.y >= 0 && viewPos13.y <= HEIGHT))
                    {
                        Vector3ff normal1 = cross(eqGrid[z + 1][x] - eqGrid[z][x], eqGrid[z][x + 1] - eqGrid[z][x]).normalize();
                        float v1 = normal1.dot(-lightDirection);
                        if (v1 < 0)
                            v1 = 0;
                        triangles.append(Vertex(viewPos11, Color(v1 * 255, v1 * 255, v1 * 255, 255)));
                        triangles.append(Vertex(viewPos12, Color(v1 * 255, v1 * 255, v1 * 255, 255)));
                        triangles.append(Vertex(viewPos13, Color(v1 * 255, v1 * 255, v1 * 255, 255)));
                    }

                    Vector2f viewPos21 = renderCam->ProjectToCanvas(eqGrid[z + 1][x]);
                    Vector2f viewPos22 = renderCam->ProjectToCanvas(eqGrid[z + 1][x + 1]);
                    Vector2f viewPos23 = renderCam->ProjectToCanvas(eqGrid[z][x + 1]);
                    if ((viewPos11.x >= 0 && viewPos11.x <= WIDTH && viewPos11.y >= 0 && viewPos11.y <= HEIGHT) ||
                        (viewPos12.x >= 0 && viewPos12.x <= WIDTH && viewPos12.y >= 0 && viewPos12.y <= HEIGHT) ||
                        (viewPos13.x >= 0 && viewPos13.x <= WIDTH && viewPos13.y >= 0 && viewPos13.y <= HEIGHT))
                    {

                        Vector3ff normal2 = (cross(eqGrid[z][x + 1] - eqGrid[z + 1][x + 1], eqGrid[z + 1][x] - eqGrid[z + 1][x + 1])).normalize();
                        float v2 = normal2.dot(-lightDirection);
                        if (v2 < 0)
                            v2 = 0;
                        triangles.append(Vertex(viewPos21, Color(v2 * 255, v2 * 255, v2 * 255, 255)));
                        triangles.append(Vertex(viewPos22, Color(v2 * 255, v2 * 255, v2 * 255, 255)));
                        triangles.append(Vertex(viewPos23, Color(v2 * 255, v2 * 255, v2 * 255, 255)));
                    }
                }
            }
            window->draw(triangles);
            // cout << to_string(t.getElapsedTime().asMilliseconds()) << " to render, " << vertsCount << endl;
        }
    }

public:
    Vector3ff position, scale;
    vector<Equation> equations;

    vector<Thread> threads;

    Graph3D() {}
    Graph3D(RenderWindow *windowToDraw, Camera3d *camToRender = &defaultCam, Vector3ff StartPosition = Vector3ff(0, 0, 0), Vector3ff Scale = Vector3ff(3, 3, 3))
    {
        this->position = StartPosition;
        this->scale = Scale;

        this->renderCam = camToRender;
        this->window = windowToDraw;

        xLine[0] = Vertex(this->renderCam->ProjectToCanvas(Vector3ff(1, 0, 0)));
        xLine[1] = Vertex(this->renderCam->ProjectToCanvas(Vector3ff(-1, 0, 0)));

        this->xScaleDisplay.setFont(defaultFont);
        this->yScaleDisplay.setFont(defaultFont);
        this->zScaleDisplay.setFont(defaultFont);
        this->xScaleDisplay.setFillColor(Color::Red);
        this->yScaleDisplay.setFillColor(Color::Green);
        this->zScaleDisplay.setFillColor(Color::Blue);

        this->equations.push_back(Equation(sinEq));

        // threads.push_back(&Graph3D::getGridLine);
        // threads.push_back(&Graph3D::getGridLine);
    }
    void Draw()
    {
        this->DrawAxisLines();
        this->DrawSeparatorLines();
        this->DrawEquationsAsPlane();
        // this->DrawEquationsAsLines();
    }
};

float maxFPS = 60;
float deltaTime = 0;
Clock frameStartTime;
Text FPSDisplay;
Clock fpsDisplayUpdate;

Vector2i oldMousePosition;
bool isMouseClicked;

bool isKeyboardClicked;
Keyboard::Key keyClicked;

Graph3D graph1;

int main()
{
    cout << "Start";
    ContextSettings settings;
    settings.antialiasingLevel = 1;
    RenderWindow window(VideoMode(WIDTH, HEIGHT), "SFML window", Style::Default, settings);
    window.setFramerateLimit(60);

    defaultFont = Font();
    defaultFont.loadFromFile("E:\\!Coding\\c++\\Projects\\SFML\\Default\\sansation.ttf");

    FPSDisplay.setFillColor(Color::Green);
    FPSDisplay.setFont(defaultFont);
    FPSDisplay.setCharacterSize(16);
    fpsDisplayUpdate = Clock();

    frameStartTime = Clock();

    graph1 = Graph3D(&window, &defaultCam);
    sunLightDirection = CircleShape(10);
    sunLightDirection.setPosition(defaultCam.ProjectToCanvas(-lightDirection));
    sunLightDirection.setFillColor(Color::Yellow);

    cout << Vector3ff(1, 2, 3) + Vector3ff(3, 2, 1) << endl;

    ofstream fpsDebug("fpsDebug.txt", ios::app);
    fpsDebug << "----------------" << endl;

    while (window.isOpen())
    {
        WIDTH = window.getSize().x;
        HEIGHT = window.getSize().y;
        deltaTime = frameStartTime.getElapsedTime().asSeconds();
        // if (deltaTime < 1 / maxFPS)
        //     continue;
        frameStartTime.restart();

        if (fpsDisplayUpdate.getElapsedTime().asSeconds() > 0.12f)
        {
            FPSDisplay.setString(to_string((int)round(1 / deltaTime)));
            fpsDebug << to_string((int)round(1 / deltaTime)) << endl;
            fpsDisplayUpdate.restart();
        }

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
            if (event.type == Event::MouseButtonPressed)
            {

                isMouseClicked = true;
            }
            if (event.type == Event::MouseButtonReleased)
            {
                isMouseClicked = false;
            }
            if (event.type == Event::MouseMoved)
            {
                Vector2i mousePosition = Mouse::getPosition();
                if (isMouseClicked)
                {
                    float sensivity = 0.1f * deltaTime;
                    defaultCam.Rotate(Vector3ff(defaultCam.rotation.x + (mousePosition.x - oldMousePosition.x) * sensivity, defaultCam.rotation.y - (mousePosition.y - oldMousePosition.y) * sensivity, defaultCam.rotation.z));
                }
                oldMousePosition = mousePosition;
            }
            if (event.type == Event::KeyPressed)
            {
                keyClicked = event.key.code;
                isKeyboardClicked = true;
            }
            if (event.type == Event::KeyReleased)
            {
                isKeyboardClicked = false;
            }
            if (event.type == Event::MouseWheelScrolled)
            {
                graph1.scale += Vector3ff(graph1.scale.x, graph1.scale.y, graph1.scale.z) * (float)event.mouseWheelScroll.delta * 0.1f;
                if (graph1.scale.x < 0.01)
                    graph1.scale.x = 0.01;
                if (graph1.scale.y < 0.01)
                    graph1.scale.y = 0.01;
                if (graph1.scale.z < 0.01)
                    graph1.scale.z = 0.01;
            }
        }
        if (isKeyboardClicked)
        {
            float speed = 10 * deltaTime;
            if (keyClicked == Keyboard::A)
            {
                defaultCam.position += defaultCam.right * -1.0f * speed;
            }
            if (keyClicked == Keyboard::W)
            {
                defaultCam.position += defaultCam.forward * speed;
            }
            if (keyClicked == Keyboard::S)
            {
                defaultCam.position += defaultCam.forward * -1.0f * speed;
            }
            if (keyClicked == Keyboard::D)
            {
                defaultCam.position += defaultCam.right * speed;
            }
            if (keyClicked == Keyboard::R)
            {
                defaultCam.position += defaultCam.up * speed;
            }
            if (keyClicked == Keyboard::F)
            {
                defaultCam.position += defaultCam.up * -1.0f * speed;
            }
        }
        if (isKeyboardClicked)
        {
            if (keyClicked == Keyboard::Add)
            {
                defaultCam.fov += 0.1;
            }
            if (keyClicked == Keyboard::Subtract)
            {
                defaultCam.fov += -0.1;
            }
        }

        graph1.Draw();

        // lightDirection.x = cosf(frame * 0.1);
        // lightDirection.z = sinf(frame * 0.1);

        lightDirection = lightDirection.normalize();

        sunLightDirection.setPosition(defaultCam.ProjectToCanvas(-lightDirection * 2.0f));
        window.draw(sunLightDirection);

        window.draw(FPSDisplay);
        window.display();
        window.clear();
        frame++;
    }
    fpsDebug.close();
    return 0;
}