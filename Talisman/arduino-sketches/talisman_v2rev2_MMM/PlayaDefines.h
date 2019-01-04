/*
 * Location-related constants
 */

#ifdef PRODUCTION
#define MAN_LAT 40.786400
#define MAN_LON -119.206500
#define PLAYA_ELEV 1190.  // m
#define SCALE 1.
#else
//#define MAN_LAT 40.42425
//#define MAN_LON -79.982274
//#define PLAYA_ELEV 272.  // m
//#define SCALE 15.
#define MAN_LAT 37.75687
#define MAN_LON -122.4149
#define PLAYA_ELEV -17.
#define SCALE 15.
//Time      : 7:0:57 when really 12:03am

#endif

///// PLAYA COORDINATES CODE /////

#define DEG_PER_RAD (180. / 3.1415926535)
#define CLOCK_MINUTES (12 * 60)
#define METERS_PER_DEGREE (40030230. / 360.)
// Direction of north in clock units
#define NORTH 10.5  // hours
#define NUM_RINGS 27  // Esplanade through Z 
#define ESPLANADE_RADIUS (2500 * .3048)  // m
#define FIRST_BLOCK_DEPTH (440 * .3048)  // m
#define BLOCK_DEPTH (240 * .3048)  // m
// How far in from Esplanade to show distance relative to Esplanade rather than the man
#define ESPLANADE_INNER_BUFFER (250 * .3048)  // m
// Radial size on either side of 12 w/ no city streets
#define RADIAL_GAP 2.  // hours
// How far radially from edge of city to show distance relative to city streets
#define RADIAL_BUFFER .25  // hours

