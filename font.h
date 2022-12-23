#define NUM_GLYPHS 128

U64 cell_size = 16;
U64 font_atlas_width = 416;
U64 font_atlas_height = 64;
GLuint texture_atlas_id;
Character c[128];
U64 w = 0;
U64 h = 0;

void init_ft_font()
{

	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		printf("ERROR::FREETYPE: Could not init FreeType Library\n");
		return -1;
	}

	// load font as face
	FT_Face face;
	if (FT_New_Face(ft, "Hack-Regular.ttf", 0, &face)) {
		printf("ERROR::FREETYPE: Failed to load font at ./external/fonts/Hack-Regular.ttf\n");
		return -1;
	}

	FT_Set_Pixel_Sizes(face, 20, 20);
	FT_GlyphSlot g = face->glyph;

	U64 rowh = 0;

	w = font_atlas_width;
	h = font_atlas_height;

	glCreateTextures(GL_TEXTURE_2D, 1, &texture_atlas_id);
	glBindTexture(GL_TEXTURE_2D, texture_atlas_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	U64 ox = 0;
	U64 oy = 0;
	rowh = 0;

	U64 txidx = 0;
	U64 tyidx = 0;

	for (U8 i = 32; i < NUM_GLYPHS ; i++)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
		{
			printf("ERROR::FREETYTPE: Failed to load Glyph\n");
			return -1;
		}

		if (ox + g->bitmap.width + 1 >= w) {
			tyidx++;
			txidx = 0;
			oy += rowh;
			ox = 0;
		}

		glTexSubImage2D(GL_TEXTURE_2D,
				0,
				ox,
				oy,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				GL_RED,
				GL_UNSIGNED_BYTE,
				g->bitmap.buffer
				);

		c[i].xoffset = txidx++;
		c[i].yoffset = tyidx;

		c[i].ax = g->advance.x >> 6;
		c[i].ay = g->advance.y >> 6;

		c[i].bw = g->bitmap.width;
		c[i].bh = g->bitmap.rows;

		c[i].bl = g->bitmap_left;
		c[i].bt = g->bitmap_top;

		c[i].tx = ox / (F32)w;
		c[i].ty = oy / (F32)h;


		F32 row_height = cell_size - MAX(rowh, g->bitmap.rows);
		rowh = MAX(rowh, g->bitmap.rows) + row_height;

		// column between characters
		F32 col_width = cell_size - g->bitmap.width;
		ox += g->bitmap.width + col_width;
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}
