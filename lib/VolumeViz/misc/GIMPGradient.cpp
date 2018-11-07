/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**************************************************************************/
#include <VolumeViz/misc/CvrGIMPGradient.h>
#include <cassert>
#include <cstdio>
#include <cstring>

CvrGIMPGradient *
CvrGIMPGradient::read(const char * buf)
{
  // The format of a gradient in GIMP v1.2 is:
  //
  // <header = "GIMP Gradient\n">
  // <nrsegments>\n
  // <segment 0: left middle right leftR leftG leftB leftA rightR rightG rightB rightA type color>
  // <segment 1: ...>
  // ...
  //
  // Note that the format has changed after GIMP 1.2.

  const int BUFLEN = strlen(buf);
  const char * HEADER = "GIMP Gradient\n";
  const int HEADERLEN = strlen(HEADER);

  const char * ptr = buf + HEADERLEN;
  assert(ptr < (buf + BUFLEN));

  CvrGIMPGradient * gg = new CvrGIMPGradient;

  int r = sscanf(ptr, "%d\n", &gg->nrsegments);
  assert(r == 1); // Note: this will fail if input is from Gimp v > 1.2.

  gg->segments = new CvrGIMPGradientSegment[gg->nrsegments];

  while (*ptr++ != '\n');
  for (int i=0; i < gg->nrsegments; i++) {
    CvrGIMPGradientSegment * s = &gg->segments[i];

    r = sscanf(ptr,
               "%f %f %f "
               "%f %f %f %f "
               "%f %f %f %f "
               "%d %d",
               &s->left, &s->middle, &s->right,
               &s->left_RGBA[0], &s->left_RGBA[1], &s->left_RGBA[2], &s->left_RGBA[3],
               &s->right_RGBA[0], &s->right_RGBA[1], &s->right_RGBA[2], &s->right_RGBA[3],
               &s->type, &s->color);
    assert(r == 13);

    // Consistency check to help us catch bugs early.
    assert(s->type == 0 && "unhandled gradient data");
    assert(s->color == 0 && "unhandled gradient data");
    assert(s->left < s->middle);
    assert(s->middle < s->right);
    assert(s->left >= 0.0f && s->left <= 1.0f);
    assert(s->middle >= 0.0f && s->middle <= 1.0f);
    assert(s->right >= 0.0f && s->right <= 1.0f);
    assert(s->left_RGBA[0] >= 0.0f && s->left_RGBA[0] <= 1.0f);
    assert(s->left_RGBA[1] >= 0.0f && s->left_RGBA[1] <= 1.0f);
    assert(s->left_RGBA[2] >= 0.0f && s->left_RGBA[2] <= 1.0f);
    assert(s->left_RGBA[3] >= 0.0f && s->left_RGBA[3] <= 1.0f);
    assert(s->right_RGBA[0] >= 0.0f && s->right_RGBA[0] <= 1.0f);
    assert(s->right_RGBA[1] >= 0.0f && s->right_RGBA[1] <= 1.0f);
    assert(s->right_RGBA[2] >= 0.0f && s->right_RGBA[2] <= 1.0f);
    assert(s->right_RGBA[3] >= 0.0f && s->right_RGBA[3] <= 1.0f);

    while (*ptr++ != '\n');
  }

  return gg;
}

// Fills out a 256-value array of integer values, by iterating over
// the colors of the GIMP gradient.
void
CvrGIMPGradient::convertToIntArray(uint8_t intgradient[256][4]) const
{
  int segmentidx = 0;
  CvrGIMPGradientSegment * segment = NULL;
  float middle_RGBA[4];

  for (int i=0; i < 256; i++) {
    float gradpos = (1.0f / 256.0f) * float(i);

    // Advance to correct segment, if necessary.
    while ((segment == NULL) || (gradpos > segment->right)) {
      assert(segmentidx < this->nrsegments);
      segment = &this->segments[segmentidx];
      segmentidx++;

      // While we're at it, calculate the RGBA value of the middle
      // gradient point of the new segment.
      for (int j=0; j < 4; j++) {
        middle_RGBA[j] =
          (segment->right_RGBA[j] - segment->left_RGBA[j]) / 2.0f +
          segment->left_RGBA[j];
      }
    }

    float left, right;
    float left_RGBA[4], right_RGBA[4];
    if (gradpos < segment->middle) {
      left = segment->left;
      right = segment->middle;
      memcpy(left_RGBA, segment->left_RGBA, 4*sizeof(float));
      memcpy(right_RGBA, middle_RGBA, 4*sizeof(float));
    }
    else {
      left = segment->middle;
      right = segment->right;
      memcpy(left_RGBA, middle_RGBA, 4*sizeof(float));
      memcpy(right_RGBA, segment->right_RGBA, 4*sizeof(float));
    }

    for (int k=0; k < 4; k++) {
      float changeperunit = float(right_RGBA[k] - left_RGBA[k]) / (right - left);
      float add = changeperunit * (gradpos - left);
      intgradient[i][k] = uint8_t((left_RGBA[k] + add) * 255.0f);
    }
  }
}
