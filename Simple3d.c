// kompilacja:
// m68k-amigaos-gcc wl.c -o wl -Os -Wno-incompatible-pointer-types

// -O0 -g3 -Wall -c -fmessage-length=0

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
//#include <clib/exec_protos.h>
#include <clib/timer_protos.h>
#include <clib/utility_protos.h>
//#include <dos/dos.h>
//#include <proto/dos.h>

//#include <math.h>
//#define PI 3.1415
//#include <libraries/mathffp.h>
//#include <clib/mathffp_protos.h>
//#include <clib/mathtrans_protos.h>


// time
struct Device *TimerBase;
struct Library *UtilityBase;

/* characteristics of the screen */

#define SCR_WIDTH  (640)
#define SCR_HEIGHT (400)
#define SCR_DEPTH    (3)
//
/* Prototypes for our functions */

VOID runDBuff(struct Screen*, struct BitMap**);
struct BitMap** setupBitMaps(LONG, LONG, LONG);
VOID freeBitMaps(struct BitMap**, LONG, LONG, LONG);
LONG setupPlanes(struct BitMap*, LONG, LONG, LONG);
VOID freePlanes(struct BitMap*, LONG, LONG, LONG);
struct Library *IntuitionBase = NULL;
struct Library *GfxBase = NULL;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Simple pre-calculated sin and cos implementation

float sintab[91] = {0.000000,0.017452,0.034899,0.052336,0.069756,0.087156,0.104528,0.121869,0.139173,0.156434,0.173648,0.190809,0.207912,0.224951,0.241922,0.258819,0.275637,0.292372,0.309017,0.325568,0.342020,0.358368,0.374607,0.390731,0.406737,0.422618,0.438371,0.453990,0.469472,0.484810,0.500000,0.515038,0.529919,0.544639,0.559193,0.573576,0.587785,0.601815,0.615661,0.629320,0.642788,0.656059,0.669131,0.681998,0.694658,0.707107,0.719340,0.731354,0.743145,0.754710,0.766044,0.777146,0.788011,0.798636,0.809017,0.819152,0.829038,0.838671,0.848048,0.857167,0.866025,0.874620,0.882948,0.891007,0.898794,0.906308,0.913545,0.920505,0.927184,0.933580,0.939693,0.945519,0.951057,0.956305,0.961262,0.965926,0.970296,0.974370,0.978148,0.981627,0.984808,0.987688,0.990268,0.992546,0.994522,0.996195,0.997564,0.998630,0.999391,0.999848,1.0};

// input: angle in degrees
float ksin(int x) {
	if (x<0) x=(-x+180);
	if (x>=360) x%=360;
	if (x<=90) {
		return sintab[x];
	} else if (x<=180) {
		return sintab[180-x];
	} else if (x<=270) {
		return -sintab[x-180];
	} else {
		return -sintab[360-x];
	}
}

// input: angle in degrees
float kcos(int x) {
	return ksin(x+90);
}
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------


/*
 ** Main routine.  Setup for using the double buffered screen.
 ** Clean up all resources when done or on any error.
 */

VOID main(int argc, char **argv) {
	struct BitMap **myBitMaps;
	struct Screen *screen;
	struct NewScreen myNewScreen;
	struct AreaInfo ainfo = {0};


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
			myNewScreen.ViewModes = HIRES | LACE;
			myNewScreen.Type = CUSTOMSCREEN | CUSTOMBITMAP | SCREENQUIET;
			myNewScreen.Font = NULL;
			myNewScreen.DefaultTitle = "Simple 3D";
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

/*
 ** setupBitMaps(): allocate the bit maps for a double buffered screen.
 */
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

		if (0 != setupPlanes(myBitMaps[0], depth, width, height)) {
			if (0 != setupPlanes(myBitMaps[1], depth, width, height))
				return (myBitMaps);

			freePlanes(myBitMaps[0], depth, width, height);
		}
		FreeMem(myBitMaps[1], (LONG) sizeof(struct BitMap));
	}
	FreeMem(myBitMaps[0], (LONG) sizeof(struct BitMap));
}
return (NULL);
}

/*
 ** runDBuff(): loop through a number of iterations of drawing into
 ** alternate frames of the double-buffered screen.  Note that the
 ** object is drawn in color 1.
 */
VOID runDBuff(struct Screen *screen, struct BitMap **myBitMaps) {


//------------------------------------------------------------------------------------------------------------------------------------------------------------------


	// InitArea(&ainfo, areabuf, 200 * 2 / 5);

	float x_orig, y_orig, z_orig, x1, x2,x3, y1, y2, y3, z1, z2, z3;

	const int SHOW_POLYGONS = 12;

	float mesh[12][9] = {

			// SOUTH
			{ 0.0f, 0.0f, 0.0f,    0.0f, 150.0f, 0.0f,    150.0f, 150.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f,    150.0f, 150.0f, 0.0f,    150.0f, 0.0f, 0.0f },
			// EAST
			{ 150.0f, 0.0f, 0.0f,    150.0f, 150.0f, 0.0f,    150.0f, 150.0f, 150.0f },
			{ 150.0f, 0.0f, 0.0f,   150.0f, 150.0f, 150.0f,    150.0f, 0.0f, 150.0f },
			// NORTH
			{ 150.0f, 0.0f, 150.0f,    150.0f, 150.0f, 150.0f,    0.0f, 150.0f, 150.0f },
			{ 150.0f, 0.0f, 150.0f,    0.0f, 150.0f, 150.0f,    0.0f, 0.0f, 150.0f },
			// WEST
			{ 0.0f, 0.0f, 150.0f,    0.0f, 150.0f, 150.0f,    0.0f, 150.0f, 0.0f },
			{ 0.0f, 0.0f, 150.0f,    0.0f, 150.0f, 0.0f,    0.0f, 0.0f, 0.0f },
			// TOP
			{ 0.0f, 150.0f, 0.0f,    0.0f, 150.0f, 150.0f,    150.0f, 150.0f, 150.0f },
			{ 0.0f, 150.0f, 0.0f,   150.0f, 150.0f, 150.0f,    150.0f, 150.0f, 0.0f },
			// BOTTOM
			{ 150.0f, 0.0f, 150.0f,    0.0f, 0.0f, 150.0f,    0.0f, 0.0f, 0.0f },
			{ 150.0f, 0.0f, 150.0f,    0.0f, 0.0f, 0.0f,    150.0f, 0.0f, 0.0f }


	};

	float result[12][9] = {0};


	// Projection Matrix

			float fNear = 0.1f;
			float fFar = 1000.0f;
			float fFov = 90.0f;
			float fAspectRatio = (SCR_WIDTH/640) / (SCR_HEIGHT/400);
			float fFovRad = 1.0f;

int ktr;
WORD toggleFrame;

struct RastPort *rport;
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

					char str[123], str1[123];

					char slot1[123], slot2[123], slot3[123], slot4[123], slot5[123], slot6[123], slot7[123], slot8[123] , slot9[123], slot10[123], slot11[123], slot12[123];

					float angles[3][2] = {{0,0.5}, {0,0.3}, {0,0.15}};  // for all planes: { current angle, rotation speed }
					int angle;

					/* MAIN DRAWING LOOP NA 2 EKRANY ----------------------------------------------------- */

					SetRast(&(screen->RastPort), 0);
					toggleFrame = 0;

					for (ktr = 1; ktr < 350; ktr++) {


						/* switch the bitmap so that we are drawing into the correct place */
						screen->RastPort.BitMap = myBitMaps[toggleFrame];
						screen->ViewPort.RasInfo->BitMap =	myBitMaps[toggleFrame];





						SetAPen(rport, 1);

						// najpierw obroc wszystkie trojkaty
						for (int k=0; k<SHOW_POLYGONS; k++) {




							// odczytaj po jednym wierzcholku
							for (int t=0; t<3; t++) {
								int w = t*3;
								x_orig = mesh[k][w];
								y_orig = mesh[k][w+1];
								z_orig = mesh[k][w+2];

// usunac potem: przesun bryle tak, zeby srodek obrotu byl w srodku a nie w jednym rogu
x_orig -= 75;
y_orig -= 75;
z_orig -= 35;

								// rotate in xy
								angle = angles[0][0];
								x1 = x_orig*kcos(angle) - y_orig*ksin(angle);
								y1 = x_orig*ksin(angle) + y_orig*kcos(angle);
								z1 = z_orig;
								// rotate in xz
								angle = angles[1][0];
								x2 = x1*kcos(angle) - z1*ksin(angle);
								y2 = y1;
								z2 = x1*ksin(angle) + z1*kcos(angle);
								// rotate in yz
								angle = angles[2][0];
								x3 = x2;
								y3 = y2*kcos(angle) - z2*ksin(angle);
								z3 = y2*ksin(angle) + z2*kcos(angle);

								// perspektywa - nieco uproszczona
								x3 = x3*(fFar-z3)/fFar;
								y3 = y3*(fFar-z3)/fFar;

								// wysrodkuj na ekranie i uwzglednij aspekt ratio i zapamietaj odwrocone trojkaty
								result[k][w] = x3 + SCR_WIDTH/2;
								result[k][w+1] = y3/fAspectRatio + SCR_HEIGHT/2;
								result[k][w+2] = z3;

						    }

							// zupdatuj wszystkie katy obrotu
							for (int t=0; t<3; t++) {
								angles[t][0] += angles[t][1];
							}

						}

						// posortuj tablice rezultatow po Z - TBD

						// narysuj wszystkie trojkaty z tablicy


						/* Draw the objects --------------------------------------------------------------
						 ** Here we clear the old frame and draw a simple triangle.
						 */

						SetRast(rport, 0);  // czyszczenie ekranu


						for (int k=0; k<SHOW_POLYGONS; k++) {


							// rysuj po jednym trojkacie

							Move(rport, result[k][0], result[k][1]);

							if (k==8 | k==9 | k==10 | k==11){ SetAPen(rport,2); } else { SetAPen(rport, 1); };


							Draw(rport, result[k][3], result[k][4]);
							Draw(rport, result[k][6], result[k][7]);
							Draw(rport, result[k][0], result[k][1]);


						}


						frameNo++; // frame counter



						GetSysTime(tv);                    // chec current time
						endTime = tv->tv_micro;
						dilatationTime = (endTime - startTime); // time of 1 frame
						fps = 1000000 / dilatationTime; // ? frames per second

//* User interface */

						SetAPen(rport, 0xA);

//						itoa(fTheta, str, 10);
//
//						//dtostrf(x3, 4, 3, str);
//
//						Move(rport, 10, 175);
//						Text(rport, str, 3);         // wyswietlaj cos

				// fps
						Move(rport, 10, 190);
						Text(rport, "Frame rate: ", 11);
						itoa(fps, str1, 10);
						Move(rport, 90, 190);
						Text(rport, str1, 2);              // fps d


				// Slot 1
						Move(rport, 470, 10);
						Text(rport, "X1: ", 4);
						itoa(result[0][0], slot1, 10);		// x
						Move(rport, 570, 10);
						Text(rport,slot1, 3);

				// slot 2
						Move(rport, 470, 22);
						Text(rport, "Y1: ", 4);
						itoa(result[0][1], slot2, 10);		// y
						Move(rport, 570, 22);
						Text(rport,slot2, 3);

				// slot 3
						Move(rport, 470, 36);
						Text(rport, "Z1: ", 4);
						itoa(result[0][2], slot3, 10);		// z
						Move(rport, 570, 36);
						Text(rport,slot3, 3);

				// slot 4
						Move(rport, 470, 48);
						Text(rport, "X2: ", 4);
						itoa(result[0][3], slot4, 10);		// x
						Move(rport, 570, 48);
						Text(rport,slot4, 3);

				// slot 5
						Move(rport, 470, 60);
						Text(rport, "Y2: ", 4);
						itoa(result[0][4], slot5, 10);		// y
						Move(rport, 570, 60);
						Text(rport,slot5, 3);

				// slot 6
						Move(rport, 470, 72);
						Text(rport, "Z2: ", 4);
						itoa(result[0][5], slot6, 10);		// z
						Move(rport, 570, 72);
						Text(rport,slot6, 3);

				// slot 7
						Move(rport, 470, 84);
						Text(rport, "X3: ", 4);
						itoa(result[0][6], slot7, 10);		// x
						Move(rport, 570, 84);
						Text(rport,slot7, 3);

				// slot 8
						Move(rport, 470, 96);
						Text(rport, "Y3: ", 4);
						itoa(result[0][7], slot8, 10);		// y
						Move(rport, 570, 96);
						Text(rport,slot8, 3);

				// slot 9
						Move(rport, 470, 108);
						Text(rport, "Z3: ", 4);
						itoa(result[0][8], slot9, 10);		// z
						Move(rport, 570, 108);
						Text(rport,slot9, 3);

//				// slot 10
//						Move(rport, 470, 120);
//						Text(rport, "X3 rotated Z:", 13);
//						itoa(x3Rotated, slot10, 10);
//						Move(rport, 570, 120);
//						Text(rport,slot10, 4);
//
//				// slot 11
//						Move(rport, 470, 132);
//						Text(rport, "Y3 rotated X:", 13);
//						itoa(y3RotatedX, slot11, 10);
//						Move(rport, 570, 132);
//						Text(rport,slot11, 4);





//
						/* update the physical display to match the newly drawn bitmap. */

						MakeScreen(screen); /* Tell intuition to do its stuff.          */
						RethinkDisplay(); /* Intuition compatible MrgCop & LoadView   */

						/*               it also does a WaitTOF().  */
						// WaitTOF();






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
		BltClear(bitMap->Planes[plane_num], (width/8) * height, 1);
	else {
		freePlanes(bitMap, depth, width, height);
		return (0);
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
}
}
