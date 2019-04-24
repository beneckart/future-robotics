void stroketext(String txt, int r, int c, int strokewidth)
{
  fill(200,200,200);
  for(int x = -strokewidth+1; x < strokewidth; x++){
    text(txt, r+x,c);
    text(txt, r,c+x);
  }
  //fill(255);
  //text(txt, r,c); 
}
