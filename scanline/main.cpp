#include <stdio.h>
#include <math.h>
#include <GL/glut.h>

typedef struct Edge {
    int yMax;
    float xYMin;
    float inverseM;
    struct Edge *next;
} Edge;

typedef struct EdgeTable {
    int yMin;
    Edge *edgeList;
    struct EdgeTable *next;
} EdgeTable;

typedef struct Point {
    int x;
    int y;
} Point;

void insertEdge(Edge **edgeList, Edge *edge) {
    if (*edgeList == NULL || (*edgeList)->xYMin < edge->xYMin) {
        edge->next = *edgeList;
        *edgeList = edge;
    } else {
        Edge *current = *edgeList;

        while (current->next != NULL && current->next->xYMin >= edge->xYMin) {
            current = current->next;
        }

        edge->next = current->next;
        current->next = edge;
    }
}

void removeExpiredEdges(EdgeTable **edgeTable, int y) {
    Edge **edgeList = &(*edgeTable)->edgeList;

    Edge *current = *edgeList;
    Edge *newList = NULL;

    //printf("\nREMOVE %d | ", y);

    while (current) {
        //printf("%d ", current->yMax);
        if (current->yMax != y) {
            Edge *edge = (Edge*) malloc(sizeof(Edge));
            edge->yMax = current->yMax;
            edge->xYMin = current->xYMin;
            edge->inverseM = current->inverseM;
            edge->next = NULL;

            insertEdge(&newList, edge);
        }

        current = current->next;
    }

    *edgeList = newList;
}

void insertEdgeTable(EdgeTable **edgeTable, Edge *edge, int y) {
    if ((*edgeTable) == NULL) {
        (*edgeTable) = (EdgeTable*) malloc(sizeof(EdgeTable));
        (*edgeTable)->yMin = y;
        (*edgeTable)->edgeList = edge;
        (*edgeTable)->next = NULL;
    } else if ((*edgeTable)->yMin == y) {
        insertEdge(&(*edgeTable)->edgeList, edge);
    } else if ((*edgeTable)->yMin < y) {
        insertEdgeTable(&((*edgeTable)->next), edge, y);
    } else if ((*edgeTable)->yMin > y) {
        EdgeTable *newEdgeTable = (EdgeTable*) malloc(sizeof(EdgeTable));
        newEdgeTable->yMin = y;
        newEdgeTable->edgeList = edge;
        newEdgeTable->next = *edgeTable;
        *edgeTable = newEdgeTable;
    }
}

void initEdgeTable(EdgeTable **edgeTable, Point *arr, int pointsCount) {
    int i;
    for (i = 0; i < pointsCount; i++) {
        Point p1 = arr[i];
        Point p2 = arr[(i + 1) % pointsCount];

        Edge *edge = (Edge*) malloc(sizeof(Edge));
        edge->yMax = fmax(p1.y, p2.y);
        edge->xYMin = p1.y < p2.y ? p1.x : p2.x;
        edge->inverseM = (float)(p2.x - p1.x) / (float)(p2.y - p1.y);
        edge->next = NULL;

        int y = fmin(p1.y, p2.y);

        insertEdgeTable(edgeTable, edge, y);
    }
}

void moveEdgesToActiveTable(EdgeTable **activeEdgeTable, Edge *edgeList) {
    while (edgeList) {
        Edge *edge = (Edge*) malloc(sizeof(Edge));
        edge->yMax = edgeList->yMax;
        edge->xYMin = edgeList->xYMin;
        edge->inverseM = edgeList->inverseM;
        edge->next = NULL;

        insertEdge(&(*activeEdgeTable)->edgeList, edge);
        edgeList = edgeList->next;
    }
}

void updateEdges(Edge *edgeList) {
    while (edgeList) {
        edgeList->xYMin += edgeList->inverseM;
        edgeList = edgeList->next;
    }
}

void printEdgeTable(EdgeTable *edgeTable) {
    if (edgeTable == NULL)
        printf("\nEmpty!");
    else
        while (edgeTable) {
            printf("\n%d -> | ", edgeTable->yMin);
            Edge *aux = edgeTable->edgeList;
            while (aux) {
                printf("%d %.2f %.2f | ", aux->yMax, aux->xYMin, aux->inverseM);
                aux = aux->next;
            }
            edgeTable = edgeTable->next;
        }
}

void printTables(EdgeTable *edgeTable, EdgeTable *activeEdgeTable) {
    printf("\n\nET");
    printEdgeTable(edgeTable);
    printf("\n\nAET");
    printEdgeTable(activeEdgeTable);
}

void setPixel(GLint x,GLint y) {
    glBegin(GL_POINTS);
    glVertex2i(x,y);
    glEnd();
}

void draw(Edge *edgeList, int y) {
    int printEnable = 1;
    while (edgeList->next) {
        int x = ceil(edgeList->xYMin);
        while (x < floor(edgeList->next->xYMin)) {
            if (printEnable) {
                setPixel(x, y);
                //DRAW
            }
            x++;
        }
        printEnable = !printEnable;
        edgeList = edgeList->next;
    }
}

void fillPolygon() {
    Point a;
    a.x = 3;
    a.y = 4;

    Point b;
    b.x = 5;
    b.y = 6;

    Point c;
    c.x = 9;
    c.y = 5;

    Point d;
    d.x = 12;
    d.y = 8;

    Point e;
    e.x = 5;
    e.y = 11;

    Point pointArray[5] = {a, b, c, d, e};

    EdgeTable *edgeTable = NULL;
    initEdgeTable(&edgeTable, pointArray, 5);

    printf("##ET##\n");
    printEdgeTable(edgeTable);
    printf("\n");

    EdgeTable *activeEdgeTable = (EdgeTable*) malloc(sizeof(EdgeTable));
    activeEdgeTable->yMin = edgeTable->yMin;
    activeEdgeTable->edgeList = NULL;

    moveEdgesToActiveTable(&activeEdgeTable, edgeTable->edgeList);

    edgeTable = edgeTable->next;
    int actualY = activeEdgeTable->yMin;

    printTables(edgeTable, activeEdgeTable);

    int x = 0;
    while (edgeTable || activeEdgeTable->edgeList) {
        if (edgeTable != NULL && edgeTable->yMin == actualY) {
            moveEdgesToActiveTable(&activeEdgeTable, edgeTable->edgeList);
            if (edgeTable)
                edgeTable = edgeTable->next;
        }

        actualY++;
        activeEdgeTable->yMin = actualY;

        printTables(edgeTable, activeEdgeTable);

        removeExpiredEdges(&activeEdgeTable, actualY);

        updateEdges(activeEdgeTable->edgeList);

        x++;
    }
}

int main(int argc, char **argv) {
    /*glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
    glutInitWindowPosition(0,0);
    glutInitWindowSize(500,500);
    glutCreateWindow("Xablau!");

    glClearColor(1.0,1.0,1.0,0.0);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0,400.0,0.0,400.0);

    glutDisplayFunc(fillPolygon);
    glutMainLoop();*/

    fillPolygon();

    printf("\n\nWithout segmentation fault!\n");
    return 0;
}

