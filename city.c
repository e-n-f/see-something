#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <jpeglib.h>

double minlat, minlon, maxlat, maxlon;
char *fname;

struct point {
	double lat;
	double lon;

	// projected: lat ... M_PI, lon ... 1
};

#define HEIGHT 2048
#define WIDTH 2048
unsigned char red[WIDTH * HEIGHT] = { 0 };
unsigned char green[WIDTH * HEIGHT] = { 0 };
unsigned char blue[WIDTH * HEIGHT] = { 0 };


void spew() {
	unsigned char buf[3 * WIDTH];

	  struct jpeg_compress_struct cinfo;
	  struct jpeg_error_mgr jerr;
	  FILE* outfile;                /* target file */
	  JSAMPROW row_pointer[1];      /* pointer to JSAMPLE row[s] */
	  int row_stride;               /* physical row width in image buffer */

	  cinfo.err = jpeg_std_error(&jerr);
	  jpeg_create_compress(&cinfo);

	  if ((outfile = fopen(fname, "wb")) == NULL) {
	    fprintf(stderr, "can't open %s\n", fname);
	    exit(1);
	  }
	  jpeg_stdio_dest(&cinfo, outfile);

	  cinfo.image_width = WIDTH;    /* image width and height, in pixels */
	  cinfo.image_height = HEIGHT;
	  cinfo.input_components = 3;           /* # of color components per pixel */
	  cinfo.in_color_space = JCS_RGB;       /* colorspace of input image */
	  jpeg_set_defaults(&cinfo);
	  jpeg_set_quality(&cinfo, 75, TRUE /* limit to baseline-JPEG values */);

	  jpeg_start_compress(&cinfo, TRUE);

	  row_stride = WIDTH * 3;       /* JSAMPLEs per row in image_buffer */

	  int y = HEIGHT - 1;
	  while (cinfo.next_scanline < cinfo.image_height) {
		int x;
		for (x = 0; x < WIDTH; x++) {
			int r = 0, g = 0, b = 0;

#if 0
			int xx, yy;
			for (xx = x - 2; xx <= x + 2; xx++) {
				for (yy = y - 2; yy <= y + 2; yy++) {
					double d = sqrt((xx - x) * (xx - x) + (yy - y) * (yy - y)) + 1;
					double rat = 1 / (d * d);

					if (xx >= 0 && yy >= 0 && xx < WIDTH && yy < HEIGHT) {
						r += red[xx + yy * WIDTH] * rat;
						g += green[xx + yy * WIDTH] * rat;
						b += blue[xx + yy * WIDTH] * rat;
					}
				}

			}

#else
			r = red[x + y * WIDTH];
			g = green[x + y * WIDTH];
			b = blue[x + y * WIDTH];
#endif

			if (r > 255) {
				r = 255;
			}
			if (g > 255) {
				g = 255;
			}
			if (b > 255) {
				b = 255;
			}

#if 0
			int t = r + g + b;
			if (t > 0) {
				int ot = t;
				if (ot > 255) {
					ot = 255;
				}
				r = ot * r / t;
				g = ot * g / t;
				b = ot * b / t;
			}
#endif

			g = (r + b) / 2;

			buf[x * 3 + 0] = r;
			buf[x * 3 + 1] = g;
			buf[x * 3 + 2] = b;
		}


	    /* jpeg_write_scanlines expects an array of pointers to scanlines.
	     * Here the array is only one element long, but you could pass
	     * more than one scanline at a time if that's more convenient.
	     */
	    row_pointer[0] = buf;
	    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
		y--;
	  }

	  /* Step 6: Finish compression */

	  jpeg_finish_compress(&cinfo);
	  /* After finish_compress, we can close the output file. */
	  fclose(outfile);

	  jpeg_destroy_compress(&cinfo);
}

struct point project(struct point p) {
	struct point ret;

	ret.lon = (p.lon - minlon) * WIDTH / (maxlon - minlon);
	ret.lat = (p.lat - minlat) * HEIGHT / (maxlat - minlat);

	// ret.lon /= M_PI;

	return ret;
}

void chomp(char *s) {
	for (; *s; s++) {
		if (*s == '\012') {
			*s = '\0';
		}
	}
}

int main(int argc, char **argv) {
	char s[2000];
	int seq = 0;

	if (argc < 6) {
		fprintf(stderr, "Usage: a.out minlat minlon maxlat maxlon file.jpg\n");
		exit(EXIT_FAILURE);
	}

	minlat = atof(argv[1]);
	minlon = atof(argv[2]);
	maxlat = atof(argv[3]);
	maxlon = atof(argv[4]);
	fname = argv[5];

	int oseq2 = 0;
	while (fgets(s, 2000, stdin)) {
		struct point p;
		char *cp = s;
		int tweet = 0;

		seq++;
		int seq2 = 255 - (seq / (889979));
		if (seq2 < 0) {
			break;
		}

		if (seq2 != oseq2) {
			printf("%d\n", seq2);
			oseq2 = seq2;
		}

		if (*cp == '@') {
			tweet = 1;
		}

		while (*cp && *cp != ' ') {
			cp++;
		}
		while (*cp && *cp == ' ') {
			cp++;
		}

		while (*cp && *cp != ' ') {
			cp++;
		}
		while (*cp && *cp == ' ') {
			cp++;
		}

		while (*cp && *cp != ' ') {
			cp++;
		}
		while (*cp && *cp == ' ') {
			cp++;
		}

		p.lat = atof(cp); // * M_PI / 180;
		while (*cp && *cp != ',') {
			cp++;
		}
		if (*cp) {
			cp++;
		}

		p.lon = atof(cp); // * M_PI / 180;
		// printf("%f %f %s", p.lat, p.lon, s);
		// printf("%f %f\n", p.lat, p.lon);

		p = project(p);

		// p.lat = p.lat / M_PI;

		int x = p.lon;
		int y = p.lat;

		if (x == WIDTH) {
			x = WIDTH - 1;
		}
		if (y == HEIGHT) {
			y = HEIGHT - 1;
		}

		if (x < 0 || y < 0 || x > WIDTH || y > HEIGHT) {
			//fprintf(stderr, "fail %d %d %s", x, y, s);
			continue;
		} else {
			//fprintf(stderr, "succeed %d %d %s", x, y, s);
		}

		//printf("%d %d => %d\n", x, y, counts[y * WIDTH + x]);

		int xx, yy;
		int xa = x, ya = y;

		unsigned char *array = red;
		if (tweet) {
			array = blue;
		}

		for (xx = xa - 1; xx <= xa + 0; xx++) {
			for (yy = ya - 1; yy <= ya + 0; yy++) {
				x = xx;
				y = yy;

				int i;
				int tseq = seq2;
				for (i = 0; i < 8; i++) {
					if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT || tseq <= 0) {
						break;
					}

					if (array[y * WIDTH + x] < tseq) {
						array[y * WIDTH + x] = tseq;
						break;
					} else {
						y += rand() % 3 - 1;
						x += rand() % 3 - 1;
						tseq = tseq * 2 / 3;
					}
				}
			}
		}
	}


	spew();

#if 0
	for (i = 0; i < WIDTH * HEIGHT; i += WIDTH) {
		for (j = i; j < i + WIDTH; j++) {
			printf("%d ", 255 - counts[j]);
		}

		printf("\n");
	}
#endif

	return 0;
}
