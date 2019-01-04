
String fmtPlayaStr(double lat, double lon) {
  if (lat == 0 && lon == 0) {
    return "I'm lost";
  } else {
    return playaStr(lat, lon);
  }
}

// 0=man, 1=espl, 2=A, 3=B, ...
float ringRadius(int n) {
  if (n == 0) {
    return 0;
  } else if (n == 1) {
    return ESPLANADE_RADIUS;
  } else if (n == 2) {
    return ESPLANADE_RADIUS + FIRST_BLOCK_DEPTH;
  } else {
    return ESPLANADE_RADIUS + FIRST_BLOCK_DEPTH + (n - 2) * BLOCK_DEPTH;
  }
}

// Distance inward from ring 'n' to show distance relative to n vs. n-1
float ringInnerBuffer(int n) {
  if (n == 0) {
    return 0;
  } else if (n == 1) {
    return ESPLANADE_INNER_BUFFER;
  } else if (n == 2) {
    return .5 * FIRST_BLOCK_DEPTH;
  } else {
    return .5 * BLOCK_DEPTH;
  }
}

int getReferenceRing(float dist) {
  for (int n = NUM_RINGS; n > 0; n--) {
    //Serial.println(n + ":" + String(ringRadius(n)) + " " + String(ringInnerBuffer(n)));
    if (ringRadius(n) - ringInnerBuffer(n) <= dist) {
      return n;
    }
  }
  return 0;
}

String getRefDisp(int n) {
  if (n == 0) {
    return ")(";
  } else if (n == 1) {
    return "Espl";
  } else {
    return String(char(int('A') + n - 2));
  }
}


String playaStr(double lat, double lon) {

  bool accurate = true; // later add functionality for looking at gps accuracy
  
  float dlat = (float)lat - MAN_LAT;
  float dlon = (float)lon - MAN_LON;

  float m_dx = dlon * METERS_PER_DEGREE * cos(MAN_LAT / DEG_PER_RAD);
  float m_dy = dlat * METERS_PER_DEGREE;

  float dist = SCALE * sqrt(m_dx * m_dx + m_dy * m_dy);
  float bearing = (DEG_PER_RAD * atan2(m_dx, m_dy));
  if(bearing >= 360) bearing -= 360;

  float clock_hours = (bearing / 360. * 12. + NORTH);
  if(clock_hours >= 12) clock_hours -= 12;
  int clock_minutes = (int)(clock_hours * 60 + .5);
  // Force into the range [0, CLOCK_MINUTES)
  //clock_minutes = ((clock_minutes % CLOCK_MINUTES) + CLOCK_MINUTES) % CLOCK_MINUTES;

  uint8_t hour = clock_minutes / 60;
  uint8_t minute = clock_minutes % 60;
  
  String clock_disp = String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute);

  int refRing;
  if (6 - abs(clock_minutes/60. - 6) < RADIAL_GAP - RADIAL_BUFFER) {
    refRing = 0;
  } else {
    refRing = getReferenceRing(dist);
  }
  float refDelta = dist - ringRadius(refRing);
  long refDeltaRounded = (long)(refDelta + .5);

  return clock_disp + " & " + getRefDisp(refRing) + (refDeltaRounded >= 0 ? "+" : "-") + String(refDeltaRounded < 0 ? -refDeltaRounded : refDeltaRounded) + "m" + (accurate ? "" : "-ish");
}

String playaStrFromPkt(uint8_t hour, uint8_t minute, uint16_t dist)
{
  String clock_disp = String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute);
  int refRing = getReferenceRing((float)dist);
  float refDelta = (float)dist - ringRadius(refRing);
  long refDeltaRounded = (long)(refDelta + .5);
  return clock_disp + " & " + getRefDisp(refRing) + (refDeltaRounded >= 0 ? "+" : "-") + String(refDeltaRounded < 0 ? -refDeltaRounded : refDeltaRounded) + "m"; 
}
