/*
  Least Recently Used (LRU) system

    To support large sets of volume data, a simple memory management
    system is implemented. The scene graph's VolumeData-node contains
    a logical clock that's incremented for each run through it's
    GLRender. All in-memory subpages are tagged with a timestamp at
    the time they're loaded. The VolumeData-node has a max limit for
    the amount of HW/SW memory it should occupy, and whenever it
    exceeds this limit it throws out the subpage with the oldest
    timestamp. A simple LRU-cache, in other words. Of course, all
    subpages' timestamps are "touched" (updated) whenever they're
    rendered.
*/

// FIXME: suggestion found on comp.graphics.api.opengl on how to check
// how much texture VRAM is available:
//
// [...]
// Once you create texture(s), you can give it(them) a priority using
// glPrioritizeTexture
// (http://www.3dlabs.com/support/developer/GLmanpages/glprioritizetextures.htm)
// 
// this should tell the ICD to put textures with the highest priority
// (and according to available texture memory on your hardware) into
// the actual video ram on you sweet geforce (or whatever card you
// happen to use:-).
// 
// and then, you can see if that texture(s) is really resident (stored
// in vram) by calling glAreTexturesResident
// (http://www.3dlabs.com/support/developer/GLmanpages/glaretexturesresident.htm)
// 
// So if you want to test how many textures you can cram into your
// vram, you can just keep on creating new textures and setting their
// priority to high (1) and for every new one you create see if it is
// really stored in vram.  Keep in mind that you need to use a given
// texture at least once before it can be "prioritized", put into
// vram.
// 
// This is really really cool, because say you want to use 10 textures
// interchangeably. If you put them in vram then calls to
// glBindTexture will be ultra fricking fast, because the texture will
// no longer have to go through the AGP bus.
// [...]
//
// 20021130 mortene.
