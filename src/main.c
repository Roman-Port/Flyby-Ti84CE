#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <graphx.h>

//Include cs
#include "perlin_noise.c"

int cameraOffsetX=50;//-200;
int cameraOffsetY=370;//0;

int terrainOffsetX = 0;
int terrainOffsetY = 0;

int nodeCount = 16;

int drawCount = 0; //DEbugging

const float PERLIN_FREQ = 0.35;
const int PERLIN_DEPTH = 1;
const float PERLIN_MULT = -150;

const int GRID_TILE_SIZE = 100/2;
const int GRID_SIZE_X=9*2;
const int GRID_SIZE_Y=12*2;
const int MAX_NODE_COUNT = GRID_SIZE_X*GRID_SIZE_Y*4;

double SIN_T;
double COS_T;

int nodes[GRID_SIZE_X*GRID_SIZE_Y*4][3] = {
    {-30, 30, -30},
    {-30, 30,  30},
    { 30, 30, -30},
    { 30, 30,  30}
};

int regions[GRID_SIZE_X*GRID_SIZE_Y]; //Holds the landscape types.

void drawRotatedCanvas(float thetaX, float thetaY, float thetaZ);
void clearScreen();
void insertSquareIntoArray(float p1[], float p2[], float p3[], float p4[], int nodeIndex, int regionIndex);
void insertSquareWithHeights(float h1, float h2, float h3, float h4, int gridX, int gridY, int arrayGridX, int arrayGridY);

/* External protos */
extern float perlin2d(float x, float y, float freq, int depth);

/* Place main code in main */
void main(void) {
    float t = 0;
    float heightBuffer[GRID_SIZE_Y][2]; //This holds a buffer of the last heights. Used for map gen. Right first.
    int x =0;
    int y =0;
    /* Start the graphics routines */
    gfx_Begin();

    /* Calculate */
    SIN_T = sin(13);
    COS_T = cos(13);

    /* Write some text */
    gfx_SetTextScale(1,1);
    gfx_PrintStringXY("Flyover - Created by RomanPort",5,5);
    gfx_PrintStringXY("Generating Terrain...",90,150);
    
    /* Do first time map gen */
    for(x=0;x<GRID_SIZE_X;x = x+1) {
        for(y=0;y<GRID_SIZE_Y;y = y+1) {
            float progress = t / (float)(GRID_SIZE_X*GRID_SIZE_Y);
            float bottom_right;
            float bottom_left;
            float top_right;
            float top_left;
            //Generate perlin noise. Remember, top left is closest to 0.
            bottom_right = perlin2d(x+1 + terrainOffsetX, y+1 + terrainOffsetY, PERLIN_FREQ, PERLIN_DEPTH)*PERLIN_MULT;
            bottom_left = perlin2d(x + terrainOffsetX, y+1 + terrainOffsetY, PERLIN_FREQ, PERLIN_DEPTH)*PERLIN_MULT;
            top_right = perlin2d(x+1 + terrainOffsetX, y + terrainOffsetY, PERLIN_FREQ, PERLIN_DEPTH)*PERLIN_MULT;
            top_left = perlin2d(x + terrainOffsetX, y + terrainOffsetY, PERLIN_FREQ, PERLIN_DEPTH)*PERLIN_MULT;
            insertSquareWithHeights(/*Bottom left*/top_left,/*Top-left*/bottom_left,/*Bottom-right*/top_right,/*top-right*/bottom_right,x,y,x,y);
            //If this is the last one on X, write to the height buffer.
            if(x+1==GRID_SIZE_X) {
                heightBuffer[y][0]=bottom_right;
                heightBuffer[y][1]=top_right;
            }
            //Write progress
            t+=1;
            gfx_Rectangle(0,230,320*progress,10);
        };
    };
    terrainOffsetX = GRID_SIZE_X;
    t=0;
    nodeCount = GRID_SIZE_X*GRID_SIZE_Y*4;
    gfx_SetDrawBuffer();

    /* Loop until a key is pressed */
    while (!os_GetCSC()) {
        /* Defs */
        int y=0;
        /* Create */
        

        /*Clear and draw */
        clearScreen();
        drawRotatedCanvas(13,13,0);
        
        
        /* Generate the terrain for next time */
        for(y=0;y<GRID_SIZE_Y;y = y+1) {
            //Draw in 3d space ahead, but replace the last but in array space.
            float bottom_right;
            float top_right;
            int xArrayPos = (terrainOffsetX - GRID_SIZE_X) % GRID_SIZE_X;
            //Also generate the new heights
            bottom_right = perlin2d(x + terrainOffsetX, y+1 + terrainOffsetY, PERLIN_FREQ, PERLIN_DEPTH)*PERLIN_MULT;
            top_right = perlin2d(x + terrainOffsetX, y + terrainOffsetY, PERLIN_FREQ, PERLIN_DEPTH)*PERLIN_MULT;
            //Create the square using the last heights in the buffer
            insertSquareWithHeights(heightBuffer[y][1],heightBuffer[y][0],top_right,bottom_right,terrainOffsetX,y,xArrayPos,y);
            //Fill in the buffer
            heightBuffer[y][0]=bottom_right;
            heightBuffer[y][1]=top_right;
        };
        terrainOffsetX = terrainOffsetX + 1;

        /* Move the camera for next time */
        cameraOffsetX -= cos(13)*GRID_TILE_SIZE;
        cameraOffsetY += (sin(13)*GRID_TILE_SIZE)/2;

        /* Swap the buffer with the screen */
        gfx_SwapDraw();
        

    }

    /* End the graphics */
    gfx_End();
}

void clearScreen() {
    gfx_FillScreen(terrainOffsetX*8);
}

float cleanHeights(float h) {
    //This just changes the heights for the map gen
    return h;
}

void insertSquareWithHeights(float h1, float h2, float h3, float h4, int gridX, int gridY, int arrayGridX, int arrayGridY) {
    //Get index
    int index = ((GRID_SIZE_X*arrayGridY)+arrayGridX)*4;
    int regionIndex = ((GRID_SIZE_X*arrayGridY)+arrayGridX);
    //Get positions
    float xpos = gridX*GRID_TILE_SIZE;
    float ypos = gridY*GRID_TILE_SIZE;
    //Create vertices
    float p1[3] = {0,0,0};
    float p2[3] = {0,0,0};
    float p3[3] = {0,0,0};
    float p4[3] = {0,0,0};
    //Add
    p1[0]=xpos;
    p1[1]=cleanHeights(h1);
    p1[2]=ypos;
    
    p2[0]=xpos;
    p2[1]=cleanHeights(h2);
    p2[2]=ypos+GRID_TILE_SIZE;
    
    p3[0]=xpos+GRID_TILE_SIZE;
    p3[1]=cleanHeights(h3);
    p3[2]=ypos;
    
    p4[0]=xpos+GRID_TILE_SIZE;
    p4[1]=cleanHeights(h4);
    p4[2]=ypos+GRID_TILE_SIZE;
    
    //Insert
    insertSquareIntoArray(p1,p2,p3,p4,index,regionIndex);
}

void insertSquareIntoArray(float p1[], float p2[], float p3[], float p4[], int nodeIndex, int regionIndex) {
    int ni=0;
    int edgeIndex = nodeIndex;
    float averageHeight=0;
    int region=0;
    //Calculate all in 3d space
    //Generate the two lines.
    
    for(ni = 0; ni<4; ni = ni+1) {
        //Run this for each of the four vertices
        int x = p1[0];
        int y = p1[1];
        int z = p1[2];
        //This is gross and will need to be fixed if this works
        if(ni==1) {
            x=p2[0];
            y=p2[1];
            z=p2[2];
        } else if(ni==2) {
            x=p3[0];
            y=p3[1];
            z=p3[2];
        } else if(ni==3) {
            x=p4[0];
            y=p4[1];
            z=p4[2];
        }

        //Write current height to average. We'll divide this later.
        averageHeight = averageHeight + y;

        //If y is under the ocean line, make it flat.
        if(-y<60) {
            y=-70;
        }

        //X first
        x = x * COS_T - z * SIN_T;
        z = z * COS_T + x * SIN_T;
        //Now, Y
        y = y * COS_T - z * SIN_T;
        z = z * COS_T + y * SIN_T;
        //Apply
        nodes[nodeIndex+ni][0] = x;
        nodes[nodeIndex+ni][1] = y;
        nodes[nodeIndex+ni][2] = z;
        
        
    }

    //We'll decide the color now based on the average height
    //Get average 
    averageHeight = averageHeight / 4;
    //Flip it so it makes more sense
    averageHeight = -averageHeight;
    //Between 0 and 150
    region = 25; //Ocean color
    if(averageHeight>60) {
        region = 207;//Sand color
    }
    if(averageHeight>75) {
        region = 007;//Grass color
    }
    if(averageHeight>125) {
        region = 223;//Snow color
    }
    //Apply color.
    regions[regionIndex]=region;
}


void drawRotatedCanvas(float thetaX, float thetaY, float thetaZ) {
    int n;
    

    //Rotate all
    for (n = nodeCount-4; n >= 0; n = n - 4) {
        /* Get the average height so we can calculate the color. */
        float averageHeight = 0;
        
        //Get color from regions
        gfx_SetColor(regions[n/4]);

        //Draw triangles
        //gfx_SetColor(0);
        gfx_FillTriangle(nodes[n+0][0] + cameraOffsetX, nodes[n+0][1] + cameraOffsetY, nodes[n+1][0] + cameraOffsetX, nodes[n+1][1] + cameraOffsetY, nodes[n+2][0] + cameraOffsetX ,nodes[n+2][1] + cameraOffsetY);
        //gfx_SetColor(5);
        gfx_FillTriangle(nodes[n+1][0] + cameraOffsetX ,nodes[n+1][1] + cameraOffsetY, nodes[n+3][0] + cameraOffsetX ,nodes[n+3][1] + cameraOffsetY ,nodes[n+2][0] + cameraOffsetX ,nodes[n+2][1] + cameraOffsetY );
        //Wireframe (slows things down, use one or the other for production)
        gfx_SetColor(0);
        gfx_Line(nodes[n+0][0] + cameraOffsetX ,nodes[n+0][1] + cameraOffsetY ,nodes[n+1][0] + cameraOffsetX ,nodes[n+1][1] + cameraOffsetY );
        //gfx_Line(nodes[n+2][0] + cameraOffsetX ,nodes[n+2][1] + cameraOffsetY ,nodes[n+3][0] + cameraOffsetX ,nodes[n+3][1] + cameraOffsetY );
        //gfx_Line(nodes[n+0][0] + cameraOffsetX ,nodes[n+0][1] + cameraOffsetY ,nodes[n+2][0] + cameraOffsetX ,nodes[n+2][1] + cameraOffsetY );
        gfx_Line(nodes[n+1][0] + cameraOffsetX ,nodes[n+1][1] + cameraOffsetY ,nodes[n+3][0] + cameraOffsetX ,nodes[n+3][1] + cameraOffsetY );
        
    }
    
}

