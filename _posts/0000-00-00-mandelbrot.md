---
title: Mandelbrot
author: Dr. Matthias Kretz
date: 0000-03-09
layout: post
cover: assets/mandelbrot-banner.jpg
---

## How to determine "in" or "out"

The [Mandelbrot set][2] is a visualization of a mathematical property of values 
on the complex plane. Every pixel location `[x, y]` (with e.g. `x` in `[0, 
1920)` and `y` in `[0, 1080)`) is mapped to a complex value
$c=\frac{x-1280}{540} + \frac{(y-540)}{540}i$.

Then let us iterate the sequence $z_{n+1} = z_n^2 + c$ with $z_0 = c$ until 
either a predefined number of iterations is reached, or the sequence tends to 
infinity (which is certain once $norm(z_n) = Re(z_n)^2+Im(z_n)^2 >= 2^2$). If 
the sequence tends to infinity, color the corresponding pixel with the number 
of iterations that were required to determine the condition. Otherwise color 
the pixel black.

In simpler terms, given `cr` and `ci` ($c = c_r + c_ii$) the iteration goes 
like this:
```c++
float zr = cr;
float zi = ci;
float norm = zr * zr + zi * zi;
unsigned iterations = 0;
while (norm < 4 and iterations < max_iterations) {
  // z = z² + c
  zr = zr * zr - zi * zi + cr;
  zi = 2 * zr * zi + ci;
  norm = zr * zr + zi * zi;
  ++iterations;
}
color = iterations == max_iterations ? BLACK : colorize(iterations);
```

## Exercises

> Where's the data-parallelism we can use for SIMD?
{:.block-warning}

> TIP
>
> You don't have to start from scratch. There's an existing (non-SIMD) 
implementation in a [zip][3] or [tar.gz][4].
>
> See [below](#starting-point) for an overview of the code.
{:.block-tip}

> Can the compiler vectorize the loop?
>
> Why doesn't `g++ -O3 -ffast-math -march=native` vectorize the loop? Can you 
fix it? (I gave up... maybe `std::execution::unseq` can do it?)
{:.block-task}

> TIP
>
> [:green_book: GCC optimization dump options][12] (try e.g. 
`-fopt-info-vec-all`)
{:.block-tip}

> Explicitly vectorize with SIMD
>
> Use `simd::vec<T>` to explicitly vectorize the Mandelbrot computation.
{:.block-task}

> TIP
>
> - Create an iota object.
> - `basic_mask` can be reduced to `bool` via `all_of`, `any_of`, and `none_of`
> - `simd::select(basic_mask, basic_vec, basic_vec)`
{:.block-tip}

> Are your images not equal?
>
> Are your images not equal? Why? How can you make them equal? A discussion 
about floating-point and equality should follow on Friday.
{:.block-warning}

> Compare speed-ups
>
> Compare speed-ups. Does this match your expectations? Try wider `simd` (using `simd::vec<T, N>`).
{:.block-task}

> Render a zoomed region
>
> Render this: `./mandel part3.ppm 300000 -0.743643887037158704752191506114774 
0.131825904205311970493132056385139 0.00000000002`.
{:.block-task}

> Why does it look bad?
>
> Why does it look bad? How can you fix it? Performance impact?
{:.block-warning}

## Hints

### Starting point

1. Quickly scan over `image.h` and `mycomplex.h`.

   * Image pixels can be accessed via `img[x, y]` and are stored as 
   `std::uint32_t`. The low 8 bits represent red, the bits 8:15 represent 
   green, the bits 16:23 represent blue, and the high 8 bits are discarded.

   * `mycomplex<T>` is a very trimmed-down implementation of complex numbers, 
   only providing the operations we need for this task. You can also use 
   `std::complex<float>`, but for me at least that makes the program slower. 
   Also `std::complex<simd::vec<float>>` is not really a thing, whereas 
   `mycomplex<simd::vec<float>>` works just fine. (`simd::vec<std::complex<float>>` is part of C++26, but not in GCC 
   16.)

2. Ignore `tsc.h`. It provides the `time_stamp_counter` used in `main()`.

3. Open up `main.cpp` and find the commented code at the bottom of `main()`. 
   Uncomment the code and then the compiler will complain about `mandelbrot.h`.

4. `mandelbrot.h` is where the fun happens.

   * The file already includes `<simd>`.
   It then defines two `simd` specializations for `float` and `unsigned int`.

   * The `mandelbrot` function constructs an `Image` and iterates over all 
   pixels. For each pixel it determines the corresponding value for `c` and 
   then iterates the sequence. The number of iterations is then turned into a 
   color using three differently oscillating functions for the three colors.


### How to write an image (`write_image` implements this already)

A very simple, straightforward solution is to output [PBM (b/w), PGM 
(grayscale), or PPM (RGB)][1]

E.g. for PGM, open a file (`std::fstream`) and write

```c++
constexpr int width = 1920;
constexpr int height = 1280;

file << "P5\n"                         // filetype
     << width << ' ' << height << '\n' // image size
     << "255\n";                       // max grayscale color value;

for (y = 0; y < height; ++y) {
  for (x = 0; x < width; ++x) {
    unsigned char color = 256 - mandelbrot_iterations(x, y); // 0-255
    file.put(color);
  }
}
```

> Bonus: Buddhabrot
>
> The [Buddhabrot][10] is a derivative of the Mandelbrot iteration above. [The 
Wikipedia section on "Rendering method"][11] explains how it's done. Your 
challenge is to find a way to apply what you learned about SIMD, ILP, and memory 
accesses to compute an image more efficiently.
{:.block-task}


[1]: https://en.wikipedia.org/wiki/Netpbm#File_formats
[2]: https://en.wikipedia.org/wiki/Mandelbrot_set
[3]: assets/mandelbrot.zip
[4]: assets/mandelbrot.tar.gz
[5]: https://en.cppreference.com/w/cpp/experimental/simd
[6]: https://github.com/mattkretz/vir-simd#simple-iota-simd-constants
[7]: https://github.com/mattkretz/vir-simd#making-simd-conversions-more-convenient
[8]: https://en.cppreference.com/w/cpp/experimental/simd/all_of
[9]: https://en.cppreference.com/w/cpp/experimental/simd/where
[10]: https://en.wikipedia.org/wiki/Buddhabrot
[11]: https://en.wikipedia.org/wiki/Buddhabrot#Rendering_method
[12]: https://gcc.gnu.org/onlinedocs/gcc/Developer-Options.html#index-fopt-info
