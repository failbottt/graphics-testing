#define MAX(a,b) (((a)>(b))?(a):(b))

typedef uint8_t		 U8;
typedef uint16_t	 U16;
typedef uint32_t	 U32;
typedef uint64_t	 U64;
typedef int8_t		 S8;
typedef int16_t		 S16;
typedef int32_t		 S32;
typedef int64_t		 S64;
typedef float		 F32;
typedef double		 F64;
typedef const char   C8;
typedef unsigned int US64;

typedef struct RGBA {
	float r;
	float g;
	float b;
	float a;
} RGBA;

typedef struct {
	float x;
	float y;
} VEC2;

typedef struct {
	float x;
	float y;
	float z;
} VEC3;

typedef struct {
	int xoffset; // x offset in texture
	int yoffset; // y offset in texture

	float ax;	// advance.x
	float ay;	// advance.y

	float bw;	// bitmap.width;
	float bh;	// bitmap.height;

	float bl;	// bitmap_left;
	float bt;	// bitmap_top;

	float tx;	// x offset of glyph in texture coordinates
	float ty;	// y offset of glyph in texture coordinates
} Character;

