// Quick'n'dirty converter from raw 8-bits-per-voxel volume data to
// VOL format.
//
// Compile with 'g++ -o raw2vol raw2vol.cpp -I$(COINDIR)/include'.
//
// 20021128 mortene.

#include <Inventor/system/inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>


struct vol_header {
  uint32_t magic_number;
  uint32_t header_length;
  uint32_t width;
  uint32_t height;
  uint32_t images;
  uint32_t bits_per_voxel;
  uint32_t index_bits;
  // FIXME: should assert-chk that sizeof(float)==4. 20021109 mortene.
  float scaleX, scaleY, scaleZ;
  float rotX, rotY, rotZ;
};

enum EndiannessValues {
  HOST_IS_UNKNOWNENDIAN = -1,
  HOST_IS_LITTLEENDIAN = 0,
  HOST_IS_BIGENDIAN = 1
};

static int endianness = HOST_IS_UNKNOWNENDIAN;
  
static int
host_get_endianness(void)
{
  union temptype {
    uint32_t value;
    uint8_t  bytes[4];
  } temp;
  temp.bytes[0] = 0x00;
  temp.bytes[1] = 0x01;
  temp.bytes[2] = 0x02;
  temp.bytes[3] = 0x03;
  switch (temp.value) {
  case 0x03020100: return HOST_IS_LITTLEENDIAN;
  case 0x00010203: return HOST_IS_BIGENDIAN;
  /* might be more variations here for some obscure CPU architectures */
  }
  assert(0 && "system has unknown endianness");
  return HOST_IS_UNKNOWNENDIAN; /* maybe just as well exit()? */
}

static void
swap_32bit_word(uint8_t * block)
{
  uint8_t tmp;

  tmp = block[3];
  block[3] = block[0];
  block[0] = tmp;

  tmp = block[2];
  block[2] = block[1];
  block[1] = tmp;
} 
  
static uint32_t  
hton_uint32(uint32_t value)
{
  if (endianness == HOST_IS_UNKNOWNENDIAN)
    endianness = host_get_endianness();
  switch (endianness) {
  case HOST_IS_BIGENDIAN:
    /* big-endian is the same order as network order */
    break;
  case HOST_IS_LITTLEENDIAN:
    swap_32bit_word((uint8_t *)&value);
    break;
  default:
    assert(0 && "system has unknown endianness");
  }
  return value;
} 
  
static uint32_t  
ntoh_uint32(uint32_t value)
{
  return hton_uint32(value);
}

static float
hton_float(float value)
{
  if ( endianness == HOST_IS_UNKNOWNENDIAN )
    endianness = host_get_endianness();
  switch ( endianness ) {
  case HOST_IS_BIGENDIAN:
    /* big-endian is the same order as network order */
    break;
  case HOST_IS_LITTLEENDIAN:
    swap_32bit_word((uint8_t *)&value);
    break;
  default:
    assert(0 && "system has unknown endianness");
  }
  return value;
}

static float
ntoh_float(float value)
{
  return hton_float(value);
}

static void
show_usage(const char * exe)
{
  (void)fprintf(stderr, 
                "\n\tUsage: %s WIDTH HEIGHT DEPTH IN-FILENAME.raw OUT-FILENAME.vol\n\n",
                exe);
}


int
main(int argc, char ** argv)
{
  struct vol_header vh = {
    hton_uint32(0x0b7e7759), // magic_number
    hton_uint32(sizeof(struct vol_header)),
    0, 0, 0, // whatever -- these are replaced
    hton_uint32(8), hton_uint32(0),
    hton_float(1.0f), hton_float(1.0f), hton_float(1.0f),
    hton_float(0.0f), hton_float(0.0f), hton_float(0.0f)
  };

  const char * exename = argc > 0 ? argv[0] : "raw2vol";
  if (argc != 6) {
    show_usage(exename);
    exit(1);
  }

  uint32_t width = atoi(argv[1]);
  uint32_t height = atoi(argv[2]);
  uint32_t images = atoi(argv[3]);

  vh.width = hton_uint32(width);
  vh.height = hton_uint32(height);
  vh.images = hton_uint32(images);

  FILE * rawf = fopen(argv[4], "rb");
  if (!rawf) {
    show_usage(exename);
    (void)fprintf(stderr, "Couldn't open file '%s' for reading: %s\n\n",
                  strerror(errno));
    exit(1);
  }

  FILE * volf = fopen(argv[5], "wb");
  if (!volf) {
    show_usage(exename);
    (void)fprintf(stderr, "Couldn't open file '%s' for writing: %s\n\n",
                  strerror(errno));
    exit(1);
  }

  const unsigned int rawsize = width * height * images;
  uint8_t * rawblock = (uint8_t *)malloc(rawsize);
  assert(rawblock);

  size_t wasread = fread(rawblock, 1, rawsize, rawf);
  assert(wasread == rawsize);

  size_t waswritten = fwrite(&vh, 1, sizeof(struct vol_header), volf);
  assert(waswritten == sizeof(struct vol_header));
  waswritten = fwrite(rawblock, 1, rawsize, volf);
  assert(waswritten == rawsize);

  free(rawblock);
  fclose(rawf);
  fclose(volf);

  return 0;
}
