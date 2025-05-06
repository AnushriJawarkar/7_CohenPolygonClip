#include <GL/glut.h>
#include <vector>
#include <iostream>

using namespace std;

const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000

int xMin = 100, yMin = 100, xMax = 400, yMax = 300;

struct Point {
    int x, y;
};

vector<Point> polygon;
bool clipRequested = false;

int computeCode(int x, int y) {
    int code = INSIDE;
    if (x < xMin) code |= LEFT;
    else if (x > xMax) code |= RIGHT;
    if (y < yMin) code |= BOTTOM;
    else if (y > yMax) code |= TOP;
    return code;
}

bool cohenSutherlandClip(int& x1, int& y1, int& x2, int& y2) {
    int code1 = computeCode(x1, y1);
    int code2 = computeCode(x2, y2);
    bool accept = false;

    while (true) {
        if ((code1 | code2) == 0) {
            accept = true;
            break;
        } else if (code1 & code2) {
            break;
        } else {
            int codeOut = code1 ? code1 : code2;
            int x, y;

            if (codeOut & TOP) {
                x = x1 + (x2 - x1) * (yMax - y1) / (y2 - y1);
                y = yMax;
            } else if (codeOut & BOTTOM) {
                x = x1 + (x2 - x1) * (yMin - y1) / (y2 - y1);
                y = yMin;
            } else if (codeOut & RIGHT) {
                y = y1 + (y2 - y1) * (xMax - x1) / (x2 - x1);
                x = xMax;
            } else {
                y = y1 + (y2 - y1) * (xMin - x1) / (x2 - x1);
                x = xMin;
            }

            if (codeOut == code1) {
                x1 = x; y1 = y;
                code1 = computeCode(x1, y1);
            } else {
                x2 = x; y2 = y;
                code2 = computeCode(x2, y2);
            }
        }
    }
    return accept;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw clipping window
    glColor3f(0, 1, 0);
    glBegin(GL_LINE_LOOP);
    glVertex2i(xMin, yMin);
    glVertex2i(xMax, yMin);
    glVertex2i(xMax, yMax);
    glVertex2i(xMin, yMax);
    glEnd();

    // If not clipped, draw original polygon
    if (!clipRequested) {
        glColor3f(1, 0, 0); // Red
        glBegin(GL_LINE_LOOP);
        for (const auto& pt : polygon) {
            glVertex2i(pt.x, pt.y);
        }
        glEnd();
    }

    // Draw clipped polygon (using Cohen-Sutherland on edges)
    if (clipRequested && polygon.size() > 1) {
        glColor3f(0, 0, 1); // Blue
        for (size_t i = 0; i < polygon.size(); i++) {
            int x1 = polygon[i].x, y1 = polygon[i].y;
            int x2 = polygon[(i + 1) % polygon.size()].x;
            int y2 = polygon[(i + 1) % polygon.size()].y;

            if (cohenSutherlandClip(x1, y1, x2, y2)) {
                glBegin(GL_LINES);
                glVertex2i(x1, y1);
                glVertex2i(x2, y2);
                glEnd();
            }
        }
    }

    glFlush();
}

void mouse(int button, int state, int x, int y) {
    y = 480 - y; // Convert window coordinates to OpenGL
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        polygon.push_back({x, y});
    }
    glutPostRedisplay();
}

void keyboard(unsigned char key, int, int) {
    if (key == 'c' || key == 'C') {
        clipRequested = true;
        glutPostRedisplay();
    } else if (key == 'r' || key == 'R') {
        polygon.clear();
        clipRequested = false;
        glutPostRedisplay();
    }
}

void init() {
    glClearColor(1, 1, 1, 1);
    gluOrtho2D(0, 640, 0, 480);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(640, 480);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Cohen-Sutherland Line Clipping for Polygon");
    init();
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
