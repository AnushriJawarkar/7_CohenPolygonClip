#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

int xmin = 100, ymin = 100, xmax = 400, ymax = 300;
vector<pair<int, int>> polyPoints;
bool finishPoly = false;

// Region codes
const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000

int getCode(int x, int y) {
    int code = INSIDE;
    if (x < xmin) code |= LEFT;
    else if (x > xmax) code |= RIGHT;
    if (y < ymin) code |= BOTTOM;
    else if (y > ymax) code |= TOP;
    return code;
}

void clipLine(int x1, int y1, int x2, int y2, vector<pair<int, int>>& result) {
    int code1 = getCode(x1, y1);
    int code2 = getCode(x2, y2);
    bool accept = false;
    
    while (true) {
        if (!(code1 | code2)) { // Both inside
            accept = true;
            break;
        } else if (code1 & code2) { // Both outside same region
            break;
        } else {
            int code = code1 ? code1 : code2;
            int x, y;
            
            if (code & TOP) {
                x = x1 + (x2 - x1) * (ymax - y1) / (y2 - y1);
                y = ymax;
            } else if (code & BOTTOM) {
                x = x1 + (x2 - x1) * (ymin - y1) / (y2 - y1);
                y = ymin;
            } else if (code & RIGHT) {
                y = y1 + (y2 - y1) * (xmax - x1) / (x2 - x1);
                x = xmax;
            } else if (code & LEFT) {
                y = y1 + (y2 - y1) * (xmin - x1) / (x2 - x1);
                x = xmin;
            }
            
            if (code == code1) {
                x1 = x; y1 = y;
                code1 = getCode(x1, y1);
            } else {
                x2 = x; y2 = y;
                code2 = getCode(x2, y2);
            }
        }
    }
    
    if (accept) {
        result.push_back({x1, y1});
        result.push_back({x2, y2});
    }
}

void clipPolygon() {
    if (polyPoints.size() < 3) return;
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Draw window
    glColor3f(0, 0, 0);
    glBegin(GL_LINE_LOOP);
    glVertex2i(xmin, ymin);
    glVertex2i(xmax, ymin);
    glVertex2i(xmax, ymax);
    glVertex2i(xmin, ymax);
    glEnd();
    
    // Draw original polygon in blue
    glColor3f(0, 0, 1);
    glBegin(GL_LINE_LOOP);
    for (auto& p : polyPoints) {
        glVertex2i(p.first, p.second);
    }
    glEnd();
    
    // Clip and draw clipped polygon in red
    vector<pair<int, int>> clippedPoints;
    for (size_t i = 0; i < polyPoints.size(); i++) {
        int x1 = polyPoints[i].first;
        int y1 = polyPoints[i].second;
        int x2 = polyPoints[(i + 1) % polyPoints.size()].first;
        int y2 = polyPoints[(i + 1) % polyPoints.size()].second;
        
        clipLine(x1, y1, x2, y2, clippedPoints);
    }
    
    if (!clippedPoints.empty()) {
        glColor3f(1, 0, 0);
        glBegin(GL_LINES);
        for (size_t i = 0; i < clippedPoints.size(); i += 2) {
            glVertex2i(clippedPoints[i].first, clippedPoints[i].second);
            glVertex2i(clippedPoints[i+1].first, clippedPoints[i+1].second);
        }
        glEnd();
    }
    
    glFlush();
}

void display() {
    glClearColor(1, 1, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Draw window
    glColor3f(0, 0, 0);
    glBegin(GL_LINE_LOOP);
    glVertex2i(xmin, ymin);
    glVertex2i(xmax, ymin);
    glVertex2i(xmax, ymax);
    glVertex2i(xmin, ymax);
    glEnd();
    
    // Draw current polygon points
    if (!polyPoints.empty()) {
        glColor3f(0, 0, 1);
        glBegin(GL_LINE_STRIP);
        for (auto& p : polyPoints) {
            glVertex2i(p.first, p.second);
        }
        glEnd();
        
        glPointSize(5);
        glBegin(GL_POINTS);
        for (auto& p : polyPoints) {
            glVertex2i(p.first, p.second);
        }
        glEnd();
    }
    
    glFlush();
}

void mouse(int button, int state, int x, int y) {
    y = 480 - y; // Invert y coordinate
    
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (!finishPoly) {
            polyPoints.push_back({x, y});
            display();
        }
    } else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        finishPoly = true;
        clipPolygon();
    }
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 'c' || key == 'C') {
        polyPoints.clear();
        finishPoly = false;
        glClear(GL_COLOR_BUFFER_BIT);
        display();
    } else if (key == 'f' || key == 'F') {
        finishPoly = true;
        clipPolygon();
    } else if (key == 27) { // ESC
        exit(0);
    }
}

void init() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 640, 0, 480);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(640, 480);
    glutCreateWindow("Cohen-Sutherland Polygon Clipping");
    
    init();
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    
    glutMainLoop();
    return 0;
}