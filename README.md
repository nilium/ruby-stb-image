stb-image gem
=============

    $ gem install stb-image

-------------------------------------------------------------------------------

This stb-image gem is a simple wrapper around [Sean Barrett's][sean-barrett]
stb_image.c file (including contributions by others), which provides a simple,
easy to use image loading API fo folks who really don't want anything other
than to load an image, cram it into a GL texture object, and discard it.

[sean-barrett]: http://nothings.org

Using it is fairly simple, and best illustrated by the following example:

    require 'stb-image'

    info = open('image.png') { |io|
        STBI.load_image(io)
    }
    raise "Unable to load image" unless info
    image_data, width, height, num_components = info

So, let's walk through this line by line.

1. Require the stb-image gem. You probably know why this is necessary.
2. Open 'image.png' with a block with the intent to store the result of the
   block in the info variable.
3. In the block, call STBI.load_image and pass the open IO object to it so it
   can read data. The result, an array containing the image data string, the
   width and height of the image, and the number of components per texel in the
   image.
4. If info is nil, raise an exception.
5. Grab the data, width, height, and components from the returned info.
   Commence doing whatever you want with this data.

It's pretty simple. Arguably, at least two or three steps here are optional.

In addition, if you'd like to load an HDR image or any image as floats instead
of bytes, you can use STBI.load_float_image, whose returned data is an array
of floats rather than bytes. You can inspect the results of either by using
`info[0].unpack('f*')` for HDR images and `'C*'` for LDR images.


License (or Lack Thereof)
-------

    These bindings for stb_image ("the software") are public domain, as with
    stb_image itself. 

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
