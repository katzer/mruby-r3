# MIT License
#
# Copyright (c) 2017 Sebastian Katzer
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

def target_win32?
  return true if ENV['OS'] == 'Windows_NT'
  build.is_a?(MRuby::CrossBuild) && build.host_target.to_s =~ /mingw/
end

MRuby::Gem::Specification.new('mruby-r3') do |spec|
  spec.license = 'MIT'
  spec.authors = 'Sebastian Katzer'
  spec.summary = 'Router dispatcher'

  r3_dir = "#{spec.dir}/r3"
  r3_src = "#{r3_dir}/src"

  pcre_h = Pathname.new("#{build.build_dir}/../mrbgems/mruby-regexp-pcre/pcre")
                   .cleanpath.to_s

  spec.cc.defines       += %w[HAVE_STRDUP _GNU_SOURCE]
  spec.cc.defines       += %w[HAVE_STRNDUP] unless target_win32?
  spec.cc.include_paths += %W[#{r3_dir}/include #{r3_src}]
  spec.linker.libraries << 'pthread'

  if Dir.exist? pcre_h
    spec.cc.defines       << 'HAVE_PCRE_H'
    spec.cc.include_paths << pcre_h
    [spec.cc, spec.linker].each { |cc| cc.defines << 'PCRE_STATIC' }
  end

  files = %W[
    #{r3_src}/edge.c
    #{r3_src}/match_entry.c
    #{r3_src}/memory.c
    #{r3_src}/node.c
    #{r3_src}/slug.c
    #{r3_src}/str.c
    #{r3_src}/token.c
  ]

  files += %W[#{r3_src}/mman.c #{r3_src}/getpagesize.c] if target_win32?

  files.map! do |f|
    f.relative_path_from(dir).pathmap("#{build_dir}/%X#{spec.exts.object}")
  end

  spec.objs += files
end
