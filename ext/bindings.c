/*
  This source is part of the Ruby bindings for stb_image.
  It is released into the public domain, as with stb_image itself.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#define STBI_NO_STDIO
#define STBI_HEADER_FILE_ONLY

#include "stb_image.c"
#include "ruby.h"

static VALUE s_stbi_module;
static ID s_read_funcid;
static ID s_skip_funcid;
static ID s_eof_funcid;
static ID s_eof_qm_funcid;

/******************************************************************************
  Callbacks used by STBI to load data using either Ruby IO objects or 'special'
  IO objects that implement both read and eof/eof? (and optionally skip).

  Specifically, any class that has the following

  class STBI_IO
    def read(num_bytes)
    end

    def skip(num_bytes)
      # optional
    end

    def eof?
    end
  end
 ******************************************************************************/

static int sr_read_callback(void *user, char *data, int size)
{
  VALUE callback_obj = *(VALUE *)user;
  VALUE sr_size = INT2NUM(size);
  VALUE sr_data = rb_check_funcall(callback_obj, s_read_funcid, 1, &sr_size);

  if (sr_data != Qundef && RTEST(sr_data)) {
    const char *string_data = StringValuePtr(sr_data);
    int string_length = RSTRING_LENINT(sr_data);
    memcpy(data, string_data, string_length);
    return string_length;
  }

  rb_warn("IO object did not respond to read, returning 0 bytes read");

  /*
    stb_image should fail on its own if it doesn't get the data it wants here,
    so just return 0
   */
  return 0;
}

static void sr_skip_callback(void *user, unsigned size)
{
  VALUE callback_obj = *(VALUE *)user;
  VALUE sr_size = UINT2NUM(size);

  /* If skip doesn't exist, call read and discard the result */
  if (rb_check_funcall(callback_obj, s_skip_funcid, 1, &sr_size) != Qundef ||
      rb_check_funcall(callback_obj, s_read_funcid, 1, &sr_size) != Qundef) {
    return;
  }

  rb_warn("IO object did not respond to either skip or read");
}

static int sr_eof_callback(void *user)
{
  VALUE callback_obj = *(VALUE *)user;
  VALUE result = rb_check_funcall(callback_obj, s_eof_qm_funcid, 0, NULL);
  if (result == Qundef && (result = rb_check_funcall(callback_obj, s_eof_qm_funcid, 0, NULL)) == Qundef) {
    rb_warn("IO object did not respond to eof or eof?, assuming not at EOF");
    result = Qfalse;
  }
  return RTEST(result);
}

/* Callback structure -- passed to STBI */
static stbi_io_callbacks s_st_callbacks = {
  sr_read_callback,
  sr_skip_callback,
  sr_eof_callback
};

/******************************************************************************
  Bindings implementations
******************************************************************************/

/*
  call-seq:
    load_image(io, required_components = COMPONENTS_DEFAULT) => [data, width, height, components]
    load_image(io, required_components = COMPONENTS_DEFAULT) { |info| ... } => obj

  Loads an image using stb_image and returns the resulting data and its width,
  height, and the number of components per pixel in the data. The returned data
  is a packed string of unsigned 8-bit integers (8 bits per component). The
  length of the string will always be width * height * components.

  In the second form, the info array is yielded to the block if the image is
  successfully loaded. Otherwise, the method returns nil. This is possibly more
  convenient than doing an <tt>if info ... end</tt> block to check if the
  image was successfully loaded.

  === IO Objects

  IO objects accepted for loading data with must implement at least
  IO#read(length) and either IO#eof or IO#eof?. In addition, they may also
  implement a skip(length) method that skips length bytes and does not return
  any value. If the IO object does not respond to #skip, #read will be called
  instead and its result will be discarded. If you want to avoid unnecessary
  allocations, it may be wise to implement a skip method.

  === Components

  If required_components is provided and not the default value, the image data
  returned will have as many components as are requested. In turn, the number of
  components returned via the array will be the same as required_components.

  Valid options for required_components are:

  [::COMPONENTS_DEFAULT]    The default value, loads as many components as are
                            provided by the image.
  [::COMPONENTS_GREY]       Loads only one component.
  [::COMPONENTS_GREY_ALPHA] Loads two components.
  [::COMPONENTS_RGB]        Loads three components (red, green, and blue).
  [::COMPONENTS_RGB_ALPHA]  Loads four components (red, green, blue, and alpha).

  === Example

    open('image.png') { |io|
      STBI.load_image(io) { |data, width, height, components|
        format = case components
                 when STBI::COMPONENTS_GREY       then Gl::GL_RED
                 when STBI::COMPONENTS_GREY_ALPHA then Gl::GL_RG
                 when STBI_COMPONENTS_RGB         then Gl::RGB
                 when STBI_COMPONENTS_RGB_ALPHA   then Gl::RGBA
                 end

        Gl::glTexImage2D(Gl::GL_TEXTURE_2D, 0, format, width, height, 0,
                         format, Gl::GL_UNSIGNED_BYTE, data)
      }
    }
 */
static VALUE sr_load_image(int argc, VALUE *argv, VALUE sr_self)
{
  VALUE sr_callbacks;
  VALUE sr_req_comp;
  VALUE sr_image_data = Qnil;
  int x = 0;
  int y = 0;
  int components[2] = {
    STBI_default,
    0
  };

  rb_scan_args(argc, argv, "11", &sr_callbacks, &sr_req_comp);

  if (NIL_P(sr_callbacks)) {
    rb_raise(rb_eArgError, "IO object cannot be nil");
    return Qnil;
  } if (RTEST(sr_req_comp)) {
    components[0] = FIX2INT(sr_req_comp);
  }

  if (!rb_obj_respond_to(sr_callbacks, s_read_funcid, 0) ||
      !(rb_obj_respond_to(sr_callbacks, s_eof_funcid, 0) ||
        rb_obj_respond_to(sr_callbacks, s_eof_qm_funcid, 0))) {
    rb_raise(rb_eTypeError, "IO object does not respond to either read or eof/eof?");
  }

  stbi_uc *data = stbi_load_from_callbacks(&s_st_callbacks, &sr_callbacks,
    &x, &y, &components[1], components[0]);

  if (data) {
    const long length = x * y * components[!components[0]];
    sr_image_data = rb_ary_new3(4,
      rb_external_str_new((const char *)data, length),
      INT2FIX(x), INT2FIX(y),
      INT2FIX(components[!components[0]]));
    stbi_image_free(data);
  }

  if (!NIL_P(sr_image_data) && rb_block_given_p()) {
    return rb_yield_splat(sr_image_data);
  } else {
    return sr_image_data;
  }
}

/*
  call-seq:
    load_float_image(io, required_components = COMPONENTS_DEFAULT) => [data, width, height, components]
    load_float_image(io, required_components = COMPONENTS_DEFAULT) { |info| ... } => obj

  Similar to ::load_image, except the returned image data is a packaed string
  for an array of 32-bit floats (e.g., String#unpack('f*') will extract an array
  of floating point values representing the components of the image's pixels).

  In the second form, the info array is yielded to the block if the image is
  successfully loaded. Otherwise, the method returns nil. This is possibly more
  convenient than doing an <tt>if info ... end</tt> block to check if the
  image was successfully loaded.

  For further information on the IO object, the required_components argument,
  and so on, see the documentation for load_image.

  === Example

    open('image.png') { |io|
      STBI.load_float_image(io) { |data, width, height, components|
        format = case components
                 when STBI::COMPONENTS_GREY       then Gl::GL_RED
                 when STBI::COMPONENTS_GREY_ALPHA then Gl::GL_RG
                 when STBI_COMPONENTS_RGB         then Gl::RGB
                 when STBI_COMPONENTS_RGB_ALPHA   then Gl::RGBA
                 end

        Gl::glTexImage2D(Gl::GL_TEXTURE_2D, 0, format, width, height, 0,
                         format, Gl::GL_FLOAT, data)
      }
    }
 */
static VALUE sr_load_float_image(int argc, VALUE *argv, VALUE sr_self)
{
  VALUE sr_callbacks;
  VALUE sr_req_comp;
  VALUE sr_image_data = Qnil;
  int x = 0;
  int y = 0;
  int components[2] = {
    STBI_default,
    0
  };

  rb_scan_args(argc, argv, "11", &sr_callbacks, &sr_req_comp);

  if (NIL_P(sr_callbacks)) {
    rb_raise(rb_eArgError, "IO object cannot be nil");
    return Qnil;
  } if (RTEST(sr_req_comp)) {
    components[0] = FIX2INT(sr_req_comp);
  }

  float *data = stbi_loadf_from_callbacks(&s_st_callbacks, &sr_callbacks,
    &x, &y, &components[1], components[0]);

  if (data) {
    const long length = x * y * components[!components[0]] * sizeof(float);
    sr_image_data = rb_ary_new3(4,
      rb_external_str_new((const char *)data, length),
      INT2FIX(x), INT2FIX(y),
      INT2FIX(components[!components[0]]));
    stbi_image_free(data);
  }

  if (!NIL_P(sr_image_data) && rb_block_given_p()) {
    return rb_yield_splat(sr_image_data);
  } else {
    return sr_image_data;
  }
}

/*
  call-seq:
    set_hdr_to_ldr_gamma(value) => value
    hdr_to_ldr_gamma = value => value

  Sets the HDR to LDR gamma used when loading HDR images with load_image.
 */
static VALUE sr_set_hdr_to_ldr_gamma(VALUE self, VALUE gamma)
{
  stbi_hdr_to_ldr_gamma((float)NUM2DBL(gamma));
  return gamma;
}

/*
  call-seq:
    set_hdr_to_ldr_scale(value) => value
    hdr_to_ldr_scale = value => value

  Sets the HDR to LDR scale used when loading HDR images with load_image.
 */
static VALUE sr_set_hdr_to_ldr_scale(VALUE self, VALUE scale)
{
  stbi_hdr_to_ldr_scale((float)NUM2DBL(scale));
  return scale;
}

/*
  call-seq:
    set_ldr_to_hdr_gamma(value) => value
    ldr_to_hdr_gamma = value => value

  Sets the LDR to HDR gamma used when loading LDR images with load_float_image.
 */
static VALUE sr_set_ldr_to_hdr_gamma(VALUE self, VALUE gamma)
{
  stbi_ldr_to_hdr_gamma((float)NUM2DBL(gamma));
  return gamma;
}

/*
  call-seq:
    set_ldr_to_hdr_scale(value) => value
    ldr_to_hdr_scale = value => value

  Sets the LDR to HDR scale used when loading LDR images with load_float_image.
 */
static VALUE sr_set_ldr_to_hdr_scale(VALUE self, VALUE scale)
{
  stbi_ldr_to_hdr_scale((float)NUM2DBL(scale));
  return scale;
}

void Init_stb_image_bindings(void)
{
  s_read_funcid = rb_intern("read");
  s_skip_funcid = rb_intern("skip");
  s_eof_funcid = rb_intern("eof");
  s_eof_qm_funcid = rb_intern("eof?");

  /*
    Container module for stb-image bindings. Of main interest are load_image
    and load_float_image, which allow you to load data from a variety of image
    formats, including

    - PNG
    - JPEG
    - TGA
    - BMP
    - PSD
    - GIF
    - HDR
    - PIC

    These formats come with some limitations, which you can read in the original
    stb_image.c header.
   */
  s_stbi_module = rb_define_module("STBI");

  /* The version constant provided by stb_image.c */
  rb_define_const(s_stbi_module, "STBI_VERSION", INT2FIX(STBI_VERSION));
  /*
    Only valid for required_components arguments to load_image
    and load_float_image. See load_image for usage.
   */
  rb_define_const(s_stbi_module, "COMPONENTS_DEFAULT", INT2FIX(STBI_default));
  /*
    Specifies that pixels in image data must have or has 1 component.
    See load_image for usage.
   */
  rb_define_const(s_stbi_module, "COMPONENTS_GREY", INT2FIX(STBI_grey));
  /*
    Specifies that pixels in image data must have or has 2 components.
    See load_image for usage.
   */
  rb_define_const(s_stbi_module, "COMPONENTS_GREY_ALPHA", INT2FIX(STBI_grey_alpha));
  /*
    Specifies that pixels in image data must have or has 3 components.
    See load_image for usage.
   */
  rb_define_const(s_stbi_module, "COMPONENTS_RGB", INT2FIX(STBI_rgb));
  /*
    Specifies that pixels in image data must have or has 4 components.
    See load_image for usage.
   */
  rb_define_const(s_stbi_module, "COMPONENTS_RGB_ALPHA", INT2FIX(STBI_rgb_alpha));

  rb_define_singleton_method(s_stbi_module, "load_image", sr_load_image, -1);
  rb_define_singleton_method(s_stbi_module, "load_float_image", sr_load_float_image, -1);

  rb_define_singleton_method(s_stbi_module, "set_hdr_to_ldr_gamma", sr_set_hdr_to_ldr_gamma, 1);
  rb_define_singleton_method(s_stbi_module, "set_hdr_to_ldr_scale", sr_set_hdr_to_ldr_scale, 1);
  rb_define_singleton_method(s_stbi_module, "set_ldr_to_hdr_gamma", sr_set_ldr_to_hdr_gamma, 1);
  rb_define_singleton_method(s_stbi_module, "set_ldr_to_hdr_scale", sr_set_ldr_to_hdr_scale, 1);

  rb_alias(rb_singleton_class(s_stbi_module), rb_intern("hdr_to_ldr_gamma="), rb_intern("set_hdr_to_ldr_gamma"));
  rb_alias(rb_singleton_class(s_stbi_module), rb_intern("hdr_to_ldr_scale="), rb_intern("set_hdr_to_ldr_scale"));
  rb_alias(rb_singleton_class(s_stbi_module), rb_intern("ldr_to_hdr_gamma="), rb_intern("set_ldr_to_hdr_gamma"));
  rb_alias(rb_singleton_class(s_stbi_module), rb_intern("ldr_to_hdr_scale="), rb_intern("set_ldr_to_hdr_scale"));
}

