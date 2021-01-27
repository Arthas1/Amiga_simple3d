#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <stdio.h>
#include <stdlib.h>
#include <devices/timer.h>
#include <clib/exec_protos.h>
#include <clib/timer_protos.h>
#include <clib/utility_protos.h>
#include <dos/dos.h>
#include <proto/dos.h>
// #include "Data3d.h"

#include <libraries/mathffp.h>
#include <clib/mathffp_protos.h>
#include <clib/mathtrans_protos.h>

struct Library *MathTransBase;

// time
struct Library *TimerBase;
struct Library *UtilityBase;

/* characteristics of the screen */
#define SCR_WIDTH  (640)
#define SCR_HEIGHT (200)
#define SCR_DEPTH    (4)   //
/* Prototypes for our functions */

VOID runDBuff(struct Screen*, struct BitMap**);
struct BitMap** setupBitMaps(LONG, LONG, LONG);
VOID freeBitMaps(struct BitMap**, LONG, LONG, LONG);
LONG setupPlanes(struct BitMap*, LONG, LONG, LONG);
VOID freePlanes(struct BitMap*, LONG, LONG, LONG);
struct Library *IntuitionBase = NULL;
struct Library *GfxBase = NULL;

/*
 ** Main routine.  Setup for using the double buffered screen.
 ** Clean up all resources when done or on any error.
 */

VOID main(int argc, char **argv) {
	struct BitMap **myBitMaps;
	struct Screen *screen;
	struct NewScreen myNewScreen;

IntuitionBase = OpenLibrary("intuition.library", 33L);
if (IntuitionBase != NULL) {
	GfxBase = OpenLibrary("graphics.library", 33L);
	if (GfxBase != NULL) {
		myBitMaps = setupBitMaps(SCR_DEPTH, SCR_WIDTH, SCR_HEIGHT);
		if (myBitMaps != NULL) {
			/* Open a simple quiet screen that is using the first
			 ** of the two bitmaps.
			 */
			myNewScreen.LeftEdge = 0;
			myNewScreen.TopEdge = 0;
			myNewScreen.Width = SCR_WIDTH;
			myNewScreen.Height = SCR_HEIGHT;
			myNewScreen.Depth = SCR_DEPTH;
			myNewScreen.DetailPen = 0;
			myNewScreen.BlockPen = 1;
			myNewScreen.ViewModes = HIRES;
			myNewScreen.Type = CUSTOMSCREEN | CUSTOMBITMAP | SCREENQUIET;
			myNewScreen.Font = NULL;
			myNewScreen.DefaultTitle = NULL;
			myNewScreen.Gadgets = NULL;
			myNewScreen.CustomBitMap = myBitMaps[0];
			screen = OpenScreen(&myNewScreen);
			if (screen != NULL) {
				/* Indicate that the rastport is double buffered. */
				screen->RastPort.Flags = DBUFFER;

				runDBuff(screen, myBitMaps);

				CloseScreen(screen);
			}
			freeBitMaps(myBitMaps, SCR_DEPTH, SCR_WIDTH, SCR_HEIGHT);
		}
		CloseLibrary(GfxBase);
	}
	CloseLibrary(IntuitionBase);
}
}

// setupBitMaps(): allocate the bit maps for a double buffered screen.

struct BitMap** setupBitMaps(LONG depth, LONG width, LONG height) {
/* this must be static -- it cannot go away when the routine exits. */
static struct BitMap *myBitMaps[2];

myBitMaps[0] = (struct BitMap*) AllocMem((LONG) sizeof(struct BitMap),
		MEMF_CLEAR);
if (myBitMaps[0] != NULL) {
	myBitMaps[1] = (struct BitMap*) AllocMem((LONG) sizeof(struct BitMap),
			MEMF_CLEAR);
	if (myBitMaps[1] != NULL) {
		InitBitMap(myBitMaps[0], depth, width, height);
		InitBitMap(myBitMaps[1], depth, width, height);

		if (NULL != setupPlanes(myBitMaps[0], depth, width, height)) {
			if (NULL != setupPlanes(myBitMaps[1], depth, width, height))
				return (myBitMaps);

			freePlanes(myBitMaps[0], depth, width, height);
		}
		FreeMem(myBitMaps[1], (LONG) sizeof(struct BitMap));
	}
	FreeMem(myBitMaps[0], (LONG) sizeof(struct BitMap));
}
return (NULL);
}

// runDBuff(): loop through a number of iterations of drawing into
// ** alternate frames of the double-buffered screen.  Note that the
// ** object is drawn in color 1.

VOID runDBuff(struct Screen *screen, struct BitMap **myBitMaps) {

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

	FLOAT x1, x2,x3, y1, y2, y3, z1, z2, z3;

	FLOAT x1Projected, y1Projected, z1Projected, x2Projected, y2Projected, z2Projected, x3Projected, y3Projected, z3Projected;

	FLOAT x1Rotated, x2Rotated, x3Rotated, y1Rotated, y2Rotated,y3Rotated, z1Rotated, z2Rotated, z3Rotated;

	FLOAT x1RotatedX, x2RotatedX, x3RotatedX, y1RotatedX, y2RotatedX, y3RotatedX, z1RotatedX, z2RotatedX, z3RotatedX;

	FLOAT fTheta;

	FLOAT matProj[4][4]={{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};

	FLOAT matRotZ[4][4]={{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};

	FLOAT matRotX[4][4]={{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};

	FLOAT mesh[12][9] = {

			// SOUTH
			{ 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },
			// EAST
			{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },
			// NORTH
			{ 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },
			// WEST
			{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },
			// TOP
			{ 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
			{ 0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },
			// BOTTOM
			{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f }
	};
	// Projection Matrix
	FLOAT fNear = 0.1f;
	FLOAT fFar = 1000.0f;
	FLOAT fFov = 90.0f;
	FLOAT fAspectRatio = 640 / 400;  										// screen size
	FLOAT fFovRad = 1.0f / tan(fFov * 0.5f / 180.0f * 3.14159f);

			matProj[0][0] = 	fAspectRatio * fFovRad;				//	0.1423882205;   // fAspectRatio * fFovRad;
			matProj[0][1] = 0.0f;
			matProj[0][2] = 0.0f;
			matProj[0][3] = 0.0f;
			matProj[1][0] = 0.0f;
			matProj[1][1] = 	fFovRad;							//	0.0444963189; //  fFovRad;
			matProj[1][2] = 0.0f;
			matProj[1][3] = 0.0f;
			matProj[2][0] = 0.0f;
			matProj[2][1] = 0.0f;
			matProj[2][2] = 	fFar / (fFar - fNear);				//	1.00010001; //fFar / (fFar - fNear);
			matProj[2][3] = 1.0f;
			matProj[3][0] = 0.0f;
			matProj[3][1] = 0.0f;
			matProj[3][2] = 	(-fFar * fNear) / (fFar - fNear); 	//	-0.100010001;//(-fFar * fNear) / (fFar - fNear);
			matProj[3][3] = 0.0f;

WORD ktr, xpos, ypos;
WORD toggleFrame;
struct Rastport *rport;
rport = &screen->RastPort;
int frameNo = 0;
DOUBLE fps;
struct ClockData *clockdata;
struct timerequest *tr;
struct timeval *tv;
LONG dilatationTime;
LONG startTime;
LONG endTime;

if (UtilityBase = OpenLibrary("utility.library", 37)) {
	if (tr = AllocMem(sizeof(struct timerequest), MEMF_CLEAR)) {
		if (tv = AllocMem(sizeof(struct timeval), MEMF_CLEAR)) {
			if (clockdata = AllocMem(sizeof(struct ClockData), MEMF_CLEAR)) {
				if (!(OpenDevice("timer.device", UNIT_VBLANK,
						(struct IORequest*) tr, 0))) {
					TimerBase = tr->tr_node.io_Device;

					GetSysTime(tv);                     // check initial time
					startTime = tv->tv_micro;
					toggleFrame = 0;
					SetAPen(&(screen->RastPort), 1);

					char str[123];
					char str1[123];
					char slot1[123];
					char slot2[123];
					char slot3[123];
					char slot4[123];
					char slot5[123];
					char slot6[123];
					char slot7[123];
					char slot8[123];
					char slot9[123];
					char slot10[123];
					char slot11[123];
					char slot12[123];

					/* MAIN DRAWING LOOP NA 2 EKRANY ----------------------------------------------------- */

					for (ktr = 1; ktr < 300; ktr++) {

// Read polygon coords in array

						 for (int k=0; k<12; k++) {

						        	x1 = mesh[k][0];
						        	y1 = mesh[k][1];
						        	z1 = mesh[k][2];
						        	x2 = mesh[k][3];
						        	y2 = mesh[k][4];
						        	z2 = mesh[k][5];
						        	x3 = mesh[k][6];
						           	y3 = mesh[k][7];
						          	z3 = mesh[k][8];

						        fTheta += 1.0f * ktr; // time for incrementation
// Transforms
						        // Rotation Z matrix

	        					matRotZ[0][0] = cos(fTheta);
	        					matRotZ[0][1] = sin(fTheta);
	        					matRotZ[1][0] = -sin(fTheta);
	        					matRotZ[1][1] = cos(fTheta);
	        					matRotZ[2][2] = 1.0f;
	        					matRotZ[3][3] = 1.0f;

	        					// Rotate Z

	        					x1Rotated = x1 * matRotZ[0][0] + y1 * matRotZ[1][0] ;
	        					y1Rotated = x1 * matRotZ[0][1] + y1 * matRotZ[1][1] ;
	        					z1Rotated = z1 * matRotZ[2][2];
	        					FLOAT w11 = matRotZ[3][3];
	        					if (w11 != 0.0f)		{		x1Rotated /= w11; y1Rotated /= w11; z1Rotated /= w11; }

	        					x2Rotated = x2 * matRotZ[0][0] + y2 * matRotZ[1][0];
	        					y2Rotated = x2 * matRotZ[0][1] + y2 * matRotZ[1][1];
	        					z2Rotated = z2 * matRotZ[2][2] ;
	        					FLOAT w12 = matRotZ[3][3];
	        					if (w12 != 0.0f)		{		x2Rotated /= w12; y2Rotated /= w12; z3Rotated /= w12; }

	        					x3Rotated = x3 * matRotZ[0][0] + y3 * matRotZ[1][0];
	        					y3Rotated = x3 * matRotZ[0][1] + y3 * matRotZ[1][1];
	        					z3Rotated = z3 * matRotZ[2][2] + matRotZ[3][2];
	        					FLOAT w13 = matRotZ[3][3];
	        					if (w13 != 0.0f)		{		x3Rotated /= w13; y3Rotated /= w13; z3Rotated /= w13; }

	        					// Rotation X matrix

	        					matRotX[0][0] = 1.0f;
	        					matRotX[1][1] = cos(fTheta * 0.5f);
	        					matRotX[1][2] = sin(fTheta * 0.5f);
	        					matRotX[2][1] = -sin(fTheta * 0.5f);
	        					matRotX[2][2] = cos(fTheta * 0.5f);
	        					matRotX[3][3] = 1.0f;

	        							// Rotate X

	        					x1RotatedX = x1Rotated * matRotX[0][0] + y1Rotated * matRotX[1][0] + z1Rotated * matRotX[2][0] + matRotX[3][0];
	        					y1RotatedX = x1Rotated * matRotX[0][1] + y1Rotated * matRotX[1][1] + z1Rotated * matRotX[2][1] + matRotX[3][1];
	        					z1RotatedX = x1Rotated * matRotX[0][2] + y1Rotated * matRotX[1][2] + z1Rotated * matRotX[2][2] + matRotX[3][2];
	        					FLOAT w111 = x1Rotated * matRotX[0][3] + y1Rotated * matRotX[1][3] + z1Rotated * matRotX[2][3] + matRotX[3][3];
	        					if (w111 != 0.0f)		{		x1RotatedX /= w111; y1RotatedX /= w111; z1RotatedX /= w111; }

	        					x2RotatedX = x2Rotated * matRotX[0][0] + y2Rotated * matRotX[1][0] + z2Rotated * matRotX[2][0] + matRotX[3][0];
	        					y2RotatedX = x2Rotated * matRotX[0][1] + y2Rotated * matRotX[1][1] + z2Rotated * matRotX[2][1] + matRotX[3][1];
	        					z2RotatedX = x2Rotated * matRotX[0][2] + y2Rotated * matRotX[1][2] + z2Rotated * matRotX[2][2] + matRotX[3][2];
	        					FLOAT w112 = x2Rotated * matRotX[0][3] + y2Rotated * matRotX[1][3] + z2Rotated * matRotX[2][3] + matRotX[3][3];
	        					if (w112 != 0.0f)		{		x2RotatedX /= w112; y2RotatedX /= w112; z3RotatedX /= w112; }

	        					x3RotatedX = x3Rotated * matRotX[0][0] + y3Rotated * matRotX[1][0] + z3Rotated * matRotX[2][0] + matRotX[3][0];
	        					y3RotatedX = x3Rotated * matRotX[0][1] + y3Rotated * matRotX[1][1] + z3Rotated * matRotX[2][1] + matRotX[3][1];
	        					z3RotatedX = x3Rotated * matRotX[0][2] + y3Rotated * matRotX[1][2] + z3Rotated * matRotX[2][2] + matRotX[3][2];
	        					FLOAT w113 = x3Rotated * matRotX[0][3] + y3Rotated * matRotX[1][3] + z3Rotated * matRotX[2][3] + matRotX[3][3];
	        					if (w113 != 0.0f)		{		x3RotatedX /= w113; y3RotatedX /= w113; z3RotatedX /= w113; }

// Offset into the screen

z1RotatedX+=3.0f;			// this should be correct
z2RotatedX+=3.0f;
z3RotatedX+=3.0f;

// Project triangles from 3D --> 2D

x1Projected = x1RotatedX * matProj[0][0];
y1Projected = y1RotatedX * matProj[1][1];
z1Projected = z1RotatedX * matProj[2][2] + matProj[3][2];
FLOAT w1 = z1RotatedX * matProj[2][3] + matProj[3][3];
if (w1 != 0.0f)		{		x1Projected /= w1; y1Projected /= w1; z1Projected /= w1; }

x2Projected = x2RotatedX * matProj[0][0];
y2Projected = y2RotatedX * matProj[1][1];
z2Projected = z2RotatedX * matProj[2][2] + matProj[3][2];
FLOAT w2 = z2RotatedX * matProj[2][3] + matProj[3][3];
if (w2 != 0.0f)		{		x2Projected /= w2; y2Projected /= w2; z2Projected /= w2; }

x3Projected =x3RotatedX * matProj[0][0];
y3Projected = y3RotatedX * matProj[1][1];
z3Projected = z3RotatedX * matProj[2][2] + matProj[3][2];
FLOAT w3 = z3RotatedX * matProj[2][3] + matProj[3][3];
if (w3 != 0.0f)		{		x3Projected /= w3; y3Projected /= w3; z3Projected /= w3; }

x1Projected += 1.0f; y1Projected += 1.0f;
x2Projected += 1.0f; y2Projected += 1.0f;
x3Projected += 1.0f; y3Projected += 1.0f;

x1Projected*= 0.4f * 640;
y1Projected*= 0.4f * 200;
x2Projected*= 0.4f * 640;
y2Projected*= 0.4f * 200;
x3Projected*= 0.4f * 640;
y3Projected*= 0.4f * 200;

// RASTERISE -------------------------------------------------
								SetAPen(&(screen->RastPort), 1);
								   Move(&(screen->RastPort), x1Projected, y1Projected);
								   Draw(&(screen->RastPort), x2Projected, y2Projected);
								   Draw(&(screen->RastPort), x3Projected, y3Projected);
								   Draw(&(screen->RastPort), x1Projected, y1Projected);
						        }

						frameNo++; // frame counter

						/* switch the bitmap so that we are drawing into the correct place */
						screen->RastPort.BitMap = myBitMaps[toggleFrame];
						screen->ViewPort.RasInfo->BitMap =
								myBitMaps[toggleFrame];

						/* Draw the objects --------------------------------------------------------------
						 ** Here we clear the old frame and draw a simple filled rectangle.
						 */
						 SetRast(&(screen->RastPort), 0);

						GetSysTime(tv);                    // check current time
						endTime = tv->tv_micro;
						dilatationTime = (endTime - startTime); // time of 1 frame
						fps = 1000000 / dilatationTime; // ? frames per second
//* User interface */
						SetAPen(rport, 0xA);
						SetDrMd(rport, JAM1);
						itoa(fTheta, str, 10);

						Move(rport, 10, 175);
						Text(rport, str, 3);         // wyswietlaj cos z odswiezaniem
				// fps
						Move(rport, 10, 190);
						Text(rport, "Frame rate: ", 11);
						itoa(fps, str1, 10);
						Move(rport, 90, 190);
						Text(rport, str1, 2);              // fps d

						int sxa=450, sya=10;

				// Slot 1
						Move(rport, sxa, sya);
						sprintf(slot1, "X1 rotated Z:%f", x1Rotated);
						Text(rport,slot1, 22);

				// slot 2
						Move(rport, sxa, sya+=12);
						sprintf(slot2, "Y1 rotated Z:%f", y1Rotated);
						Text(rport,slot2, 22);

				// slot 3
						Move(rport, sxa, sya+=12);
						sprintf(slot3, "Z2 rotated Z:%f", z2Rotated);
						Text(rport,slot3, 22);

				// slot 4
						Move(rport, sxa, sya+=12);
						sprintf(slot4, "X3 rotated X:%f", x3RotatedX);
						Text(rport,slot4, 22);

				// slot 5
						Move(rport, sxa, sya+=12);
						sprintf(slot5, "Y2 rotated X:%f", y2RotatedX);
						Text(rport,slot5, 22);

				// slot 6
						Move(rport, sxa, sya+=12);
						sprintf(slot6, "Z1 rotated X:%f", z1RotatedX);
						Text(rport,slot6, 22);

				// slot 7
						Move(rport, sxa, sya+=12);
						sprintf(slot7, "X2 projected:%f", x2Projected);
						Text(rport,slot7, 22);

				// slot 8
						Move(rport, sxa, sya+=12);
						sprintf(slot8, "Y2 projected:%f", y2Projected);
						Text(rport,slot8, 22);

				// slot 9
						Move(rport, sxa, sya+=12);
						sprintf(slot9, "Z1 projected:%f", z1Projected);
						Text(rport,slot9, 22);

				// slot 10
						Move(rport, sxa, sya+=12);
						sprintf(slot10, "X3 rotated Z:%f", x3Rotated);
						Text(rport,slot10, 22);

				// slot 11
						Move(rport, sxa, sya+=12);
						sprintf(slot11, "Y3 rotated Z:%f", y3Rotated);
						Text(rport,slot11, 22);

						/* update the physical display to match the newly drawn bitmap. */
						MakeScreen(screen); /* Tell intuition to do its stuff.          */
						RethinkDisplay(); /* Intuition compatible MrgCop & LoadView   */
						/*               it also does a WaitTOF().  */

						/* switch the frame number for next time through */
						startTime = endTime;
						toggleFrame ^= 1;
					}

					CloseDevice((struct IORequest*) tr);
				}
				FreeMem(clockdata, sizeof(struct ClockData));
			}
			FreeMem(tv, sizeof(struct timeval));
		}
		FreeMem(tr, sizeof(struct timerequest));
	}
	CloseLibrary(UtilityBase);
}
}

/*
 ** freeBitMaps(): free up the memory allocated by setupBitMaps().
 */
VOID freeBitMaps(struct BitMap **myBitMaps, LONG depth, LONG width, LONG height) {
freePlanes(myBitMaps[0], depth, width, height);
freePlanes(myBitMaps[1], depth, width, height);

FreeMem(myBitMaps[0], (LONG) sizeof(struct BitMap));
FreeMem(myBitMaps[1], (LONG) sizeof(struct BitMap));
}

/*
 ** setupPlanes(): allocate the bit planes for a screen bit map.
 */
LONG setupPlanes(struct BitMap *bitMap, LONG depth, LONG width, LONG height) {
SHORT plane_num;

for (plane_num = 0; plane_num < depth; plane_num++) {
	bitMap->Planes[plane_num] = (PLANEPTR) AllocRaster(width, height);
	if (bitMap->Planes[plane_num] != NULL)
		BltClear(bitMap->Planes[plane_num], (width / 8) * height, 1);
	else {
		freePlanes(bitMap, depth, width, height);
		return (NULL);
	}
}
return (TRUE);
}

/*
 ** freePlanes(): free up the memory allocated by setupPlanes().
 */
VOID freePlanes(struct BitMap *bitMap, LONG depth, LONG width, LONG height) {
SHORT plane_num;

for (plane_num = 0; plane_num < depth; plane_num++) {
	if (bitMap->Planes[plane_num] != NULL)
		FreeRaster(bitMap->Planes[plane_num], width, height);
}}
