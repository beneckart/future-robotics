float MAN_LAT = 40.786400;
float MAN_LON = -119.206500;
float DEG_PER_RAD = (180. / 3.1415926535);
float METERS_PER_DEGREE = (40030230. / 360.);

float NORTH = 10.5;
float ROTATE_THETA = -NORTH/12.0*2.0*PI; //3.14159265;


float[] playa_to_latlon(int hour, int minute, int dist)
{
    float clock_hours = hour + ((float)minute)/60.0;
    
    float bearing = (((clock_hours - NORTH)/12.)*360.);
    
    float m_dy = dist * cos(bearing/DEG_PER_RAD); 
    float m_dx = dist * sin(bearing/DEG_PER_RAD);
    
    float dlon = m_dx/(METERS_PER_DEGREE*cos(MAN_LAT / DEG_PER_RAD));
    float dlat = m_dy/METERS_PER_DEGREE;
    
    float lat = dlat + MAN_LAT;
    float lon = dlon + MAN_LON;

    float[] latlon = new float[2];

    latlon[0] = lat;
    latlon[1] = lon;
    return latlon;
}

float[] latlon_to_meters(float lat, float lon)
{
   float lat_m = lat*METERS_PER_DEGREE;
   float lon_m = lon*METERS_PER_DEGREE * cos(MAN_LAT / DEG_PER_RAD);
   float latR = rotateX(lat_m, lon_m, ROTATE_THETA);
   float lonR = rotateY(lat_m, lon_m, ROTATE_THETA);
   float[] latlon = new float[2];
   latlon[0] = latR;
   latlon[1] = lonR;
   return latlon;
   
}

int[] to_pixel_coord(float lat, float lon, float scale, float minLat, float minLon, boolean reverse)
{
    float lat_normalized = scale*(lat - minLat);
    float lon_normalized = scale*(lon - minLon);
    
    int pixel_coord[] = new int[2];
    //pixel_coord[0] = min(int((lat_normalized*RESOLUTION)+0.5), RESOLUTION-1); //
    if(reverse)
      pixel_coord[0] = RESOLUTION-min(int((lat_normalized*RESOLUTION)+0.5), RESOLUTION);
    else
      pixel_coord[0] = min(int((lat_normalized*RESOLUTION)+0.5), RESOLUTION-1); //
    pixel_coord[1] = min(RESOLUTION - min(int((lon_normalized*RESOLUTION)+0.5), RESOLUTION), RESOLUTION-1);
    //pixel_coord[1] = min(int((lon_normalized*RESOLUTION)+0.5), RESOLUTION-1);
    return pixel_coord;
}

float rotateX(float x, float y, float theta)
{
    float xr = x*cos(theta) - y*sin(theta);
    //yr = x*sin(theta) + y*cos(theta)
    return xr;
}

float rotateY(float x, float y, float theta)
{
    //xr = x*cos(theta) - y*sin(theta)
    float yr = x*sin(theta) + y*cos(theta);
    return yr;
}
