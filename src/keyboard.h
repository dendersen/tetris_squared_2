//stty
//https://github.com/bart9h/2n/blob/master/rawkb.c

void enableKeyPull();
//void disableKeyPull();
int getNextKeys(uint8_t* buffer, int size);

#define keysPerOperation 2
enum customKeyEncoding{
  a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,space,
  dash,comma,period,slash,semicolon,quote,pipe,backslash,
  bracketOpen,bracketClose,curlyBracketOpen,curlyBracketClose,squareBracketOpen,squareBracketClose,
  up,down,left,right,
  esc,tab,
  NONE_key
};
enum tetrisOperateions{
  moveLeft,moveRight,moveDown,dropPiece,
  rotateClockwise,rotateCounterClockwise,
  holdPiece,
  NONE_operation
};
