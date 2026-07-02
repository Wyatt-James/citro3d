# Differences to Normal C3D

- [My libctru fork](https://github.com/Wyatt-James/libctru/) is
**REQUIRED** to build. It is expected to be installed in-place of
the normal libctru release and must be built before C3D. Docker
or an equivalent is recommended.
- Dual command queues: C3D_InitEx can initialize a second GPU command
list to reduce blocking during command list creation. Feature by derrekr.
- API changes
  - C3D_DEPTHTYPE has been removed. It was an awkward feature that
  tended to break if not used very carefully. C3D_RenderTargetCreate
  now accepts a normal GPU_DEPTHBUF parameter.
- General optimizations
  - Float uniforms are much faster, as their dirty states are tracked
  in a bitfield instead of a bool array.
  - Geometry shader uniforms are not sent to the GPU when there is
  no active geoshader.
  - The libctru fork's GPU driver is broadly faster and is utilized
  effectively.
  - Command lists are usually slightly smaller. Shader uniforms can
  be slightly larger, but C3D_BufInfo and C3D_Effect are now smaller.
  - Other miscellaneous small improvements have been made to uniforms
  and binding.
- The following build flags were added to the Makefile:
  - `ENABLE_PROFILER (Default: 0)`: enables profiling of C3D internals.
  Overhead is very high and accuracy is limited, so this setting is
  only useful for development of C3D itself.
  - `GPUCMD_DISABLE_BOUNDS_CHECKS (Default 0)`:
  If 0, the GPU driver will avoid exceeding the bounds of its buffer.
  If 1, this functionality is disabled. Disabling this can cause memory
  corruption if the buffer is not large enough! See Footnote 1.
  - `GPUCMD_ENABLE_ZERO_PADDING (Default: 0)`:
  If 1, the GPU driver will pad odd-sized GPU commands with zeroes,
  mirroring stock libctru behavior. If 0, commands are still aligned,
  but padding data is not zeroed. This seems to be safe and improves
  performance. See Footnote 1.
  - `GPUCMD_INLINE_THRESH (Default: 6)`: Adjusts the auto-inline
  threshold for applicable GPU driver calls. Writes smaller or equal
  to this number are inlined to improve performance. Set to -1 to
  disable. See Footnote 1.
  - `ENABLE_LTO (Default: 0)`: Enables building C3D itself with LTO.

Footnote 1: This build flag is a direct mirror of one in libctru. Care
should be taken to ensure that these are synchronized between the two.

# How to Modify Your Codebase For This Fork

Your project may not build correctly at first. The issue is likely
caused by be the removal of C3D_DEPTHTYPE. Use C3D_DEPTH_NONE
in-place of -1 to skip allocating a depth buffer.

To achieve the best performance, you should:
- Find out how big your project's GPU command lists actually are,
ensure that buffers are large enough, and disable GPUCMD bounds checks.
- Use constants for vertex shader uniform locations. Picasso can
output these in custom header files. This improves inlining
tremendously.
- Ensure that vertex shader float uniforms that are frequently updated
together have contiguous uniform locations.
- Enable LTO at build time for a small speedup.

# Setup

citro3d can be built and installed using the following command:

    make install

# License

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you
     must not claim that you wrote the original software. If you use
     this software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and
     must not be misrepresented as being the original software.
  3. This notice may not be removed or altered from any source
     distribution.
