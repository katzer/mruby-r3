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

assert 'R3' do
  assert_kind_of Module, R3
end

assert 'R3::ANY' do
  assert_include R3.constants, :ANY
  assert_equal 0, R3::ANY
end

assert 'R3::GET' do
  assert_include R3.constants, :GET
end

assert 'R3::POST' do
  assert_include R3.constants, :POST
end

assert 'R3::PUT' do
  assert_include R3.constants, :PUT
end

assert 'R3::DELETE' do
  assert_include R3.constants, :DELETE
end

assert 'R3::PATCH' do
  assert_include R3.constants, :PATCH
end

assert 'R3::HEAD' do
  assert_include R3.constants, :HEAD
end

assert 'R3::OPTIONS' do
  assert_include R3.constants, :OPTIONS
end

assert 'R3::method_code' do
  assert_equal R3::OPTIONS, R3.method_code(:OPTIONS)
  assert_equal R3::OPTIONS, R3.method_code('OPTIONS')
  assert_nil R3.method_code(R3::OPTIONS)
  assert_nil R3.method_code('Tree')
end

assert 'R3::method_name' do
  assert_equal 'OPTIONS', R3.method_name(R3::OPTIONS)
  assert_equal 'DELETE',  R3.method_name(R3::DELETE)
  assert_nil R3.method_name(1.5)
end
