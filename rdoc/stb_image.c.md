# stb_image.c Header Comment

The header comment of the original stb_image.c, included for reference, as it
lists the limitations of the formats supported by it as well as its credits,
notes, and other information.

    stbi-1.33 - public domain JPEG/PNG reader - http://nothings.org/stb_image.c
       when you control the images you're loading
                                         no warranty implied; use at your own risk

       QUICK NOTES:
          Primarily of interest to game developers and other people who can
              avoid problematic images and only need the trivial interface

          JPEG baseline (no JPEG progressive)
          PNG 8-bit-per-channel only

          TGA (not sure what subset, if a subset)
          BMP non-1bpp, non-RLE
          PSD (composited view only, no extra channels)

          GIF (*comp always reports as 4-channel)
          HDR (radiance rgbE format)
          PIC (Softimage PIC)

          - decode from memory or through FILE (define STBI_NO_STDIO to remove code)
          - decode from arbitrary I/O callbacks
          - overridable dequantizing-IDCT, YCbCr-to-RGB conversion (define STBI_SIMD)

       Latest revisions:
          1.33 (2011-07-14) minor fixes suggested by Dave Moore
          1.32 (2011-07-13) info support for all filetypes (SpartanJ)
          1.31 (2011-06-19) a few more leak fixes, bug in PNG handling (SpartanJ)
          1.30 (2011-06-11) added ability to load files via io callbacks (Ben Wenger)
          1.29 (2010-08-16) various warning fixes from Aurelien Pocheville 
          1.28 (2010-08-01) fix bug in GIF palette transparency (SpartanJ)
          1.27 (2010-08-01) cast-to-uint8 to fix warnings (Laurent Gomila)
                            allow trailing 0s at end of image data (Laurent Gomila)
          1.26 (2010-07-24) fix bug in file buffering for PNG reported by SpartanJ

       See end of file for full revision history.

       TODO:
          stbi_info support for BMP,PSD,HDR,PIC


     ============================    Contributors    =========================
                  
     Image formats                                Optimizations & bugfixes
        Sean Barrett (jpeg, png, bmp)                Fabian "ryg" Giesen
        Nicolas Schulz (hdr, psd)                                                 
        Jonathan Dummer (tga)                     Bug fixes & warning fixes           
        Jean-Marc Lienher (gif)                      Marc LeBlanc               
        Tom Seddon (pic)                             Christpher Lloyd           
        Thatcher Ulrich (psd)                        Dave Moore                 
                                                     Won Chun                   
                                                     the Horde3D community      
     Extensions, features                            Janez Zemva                
        Jetro Lauha (stbi_info)                      Jonathan Blow              
        James "moose2000" Brown (iPhone PNG)         Laurent Gomila                             
        Ben "Disch" Wenger (io callbacks)            Aruelien Pocheville
        Martin "SpartanJ" Golini                     Ryamond Barbiero
                                                     David Woo
                                                     

     If your name should be here but isn't, let Sean know.
