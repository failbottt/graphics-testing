#define MAX(a,b) (((a)>(b))?(a):(b))

typedef int8_t		 S8;
typedef int16_t		 S16;
typedef int32_t		 S32;
typedef int64_t		 S64;
typedef uint8_t		 U8;
typedef uint16_t	 U16;
typedef uint32_t	 U32;
typedef uint64_t	 U64;
typedef float		 F32;
typedef double		 F64;

typedef struct RGBA {
	F32 r;
	F32 g;
	F32 b;
	F32 a;
} RGBA;

typedef struct {
	F32 x;
	F32 y;
} VEC2;

typedef struct {
	F32 x;
	F32 y;
	F32 z;
} VEC3;

typedef struct Character{
	int xoffset; // x offset in texture
	int yoffset; // y offset in texture

	F32 ax;	// advance.x
	F32 ay;	// advance.y

	F32 bw;	// bitmap.width;
	F32 bh;	// bitmap.height;

	F32 bl;	// bitmap_left;
	F32 bt;	// bitmap_top;

	F32 tx;	// x offset of glyph in texture coordinates
	F32 ty;	// y offset of glyph in texture coordinates
} Character;

typedef struct UI_Button {
	U8  state;
	F32 width;
	F32 height;
	F32 x;
	F32 y;
	RGBA color;
	VEC2 top_left;
	VEC2 top_right;
	VEC2 bot_left;
	VEC2 bot_right;
} UI_Button;

UI_Button buttons[2];
