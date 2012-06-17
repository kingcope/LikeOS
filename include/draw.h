class Color {
	public:
	
	Color(unsigned char R, unsigned char G, unsigned char B);	
	unsigned char getR();	
	unsigned char getG();
	unsigned char getB();
	void setR(unsigned char R);
	void setG(unsigned char G);
	void setB(unsigned char B);
	
	private:
	 unsigned char r,g,b;
};

class Draw {
	public:
	 Draw();
	 void SetOutputBuffer(unsigned char outputbuffer);
	 inline void DrawPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);
	 void DrawLine(int fromx, int tox, int y, Color *col);
	 private:
	 unsigned char OutputBuffer;
	 unsigned long *xbuffer;
};
