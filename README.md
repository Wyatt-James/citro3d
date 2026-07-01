# How to Modify Your Codebase For This Fork

- Use constants for GPU float shader uniform locations. Picasso can
output these in custom header files. This improves inlining
tremendously.
- Find out how large your command lists actually are, then disable
GPUCMD bounds checks. See below.
- Enable LTO at C3D build time for a small speedup.

# Differences to Normal C3D

- [My libctru fork](https://github.com/Wyatt-James/libctru/) is
**REQUIRED** to build. It is expected to be installed in-place of
the normal libctru release. Docker or an equivalent is recommended.
- Dual command queues: C3D_Init can now set up dual command queues
to reduce blocking caused by CPU render time. Feature by derrekr.
- API changes
  - C3D_DEPTHTYPE has been removed from render target creation. It
  was an awkward feature that tended to break if not used very
  carefully. C3D_RenderTargetCreate now accepts a normal GPU_DEPTHBUF
  parameter.
- General optimizations
  - Float Uniforms are much faster, as their dirty states are tracked
  in a bitfield instead of a bool array.
  - Geometry shader uniforms are not sent to the GPU when there is
  no active geoshader.
  - The libctru fork's GPU driver is broadly faster and is utilized
  effectively.
  - Command lists are usually slightly smaller. Shader uniforms can
  be slightly larger, but C3D_BufInfo and C3D_Effect are now smaller.
  - Other miscellaneous small improvements have been made to uniforms
  and binding.
- Build flags: set in the makefile but can be overridden by passing
arguments to make.
  - `ENABLE_PROFILER (Default: 0)`: enables profiling of C3D internals.
  Very high overhead and limited accuracy, so only useful for
  development of C3D itself.
  - `ENABLE_LTO (Default: 0)`: enables building C3D itself with LTO.
  - `GPUCMD_DISABLE_BOUNDS_CHECKS (Default: 0, checks enabled)`:
  disables bounds checking in the libctru GPU driver. This can cause
  memory corruption if the queue is not large enough!
  - `GPUCMD_ENABLE_ZERO_PADDING (Default: 0, padding not zeroed)`:
  pads odd-sized GPU commands with zeroes, a stock libctru feature.
  Disabling this seems to be safe and improves performance.
  - `GPUCMD_INLINE_THRESH (Default: 6)`: adjusts the auto-inline
  threshold for applicable GPU driver calls. Writes smaller or
  equal to this number are inlined to improve performance.

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
