# This source is part of the Ruby bindings for stb_image.
# It is released into the public domain, as with stb_image itself.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

Gem::Specification.new { |s|
  s.name        = 'stb-image'
  s.version     = '1.0.1'
  s.date        = '2013-07-24'
  s.summary     = 'stb_image bindings'
  s.description = <<-EOS
stb_image.c bindings for loading basic image data from a variety of formats
(JPG, PNG, TGA, BMP, PSD, GIF, HDR, and PIC). Data can be loaded either as
LDR (8-bit channels) or HDR (32-bit float channels) data.
  EOS
  s.authors     = [ 'Noel Raymond Cower' ]
  s.email       = 'ncower@gmail.com'
  s.files       = Dir.glob('lib/**/*.{rb,stub}') +
                  Dir.glob('ext/**/*.{c,rb}') +
                  Dir.glob('rdoc/**/*.{rdoc,md}') +
                  [ 'COPYING', 'README.md' ]
  s.extensions << 'ext/extconf.rb'
  s.homepage    = 'https://github.com/nilium/ruby-stb-image'
  s.license     = 'Public Domain'
  s.has_rdoc    = true
  s.extra_rdoc_files = [
      'ext/bindings.c',
      'rdoc/stb_image.c.md',
      'README.md',
      'COPYING'
  ]
  s.rdoc_options << '--title' << 'stb-image' <<
                    '--main' << 'README.md' <<
                    '--line-numbers'
}
