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
