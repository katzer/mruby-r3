# MIT License
#
# Copyright (c) 2017 by Sebastian Katzer. All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

def tree(capa = 1)
  R3::Tree.new(capa)
end

def setup_tree
  tree = R3::Tree.new(1)
  yield tree if block_given?
  tree.compile
  tree
end

def compiled_with_pcre?
  Object.const_defined? :Regexp
end

assert 'R3::Tree' do
  assert_kind_of Class, R3::Tree
end

assert 'R3::Tree#initialize()' do
  assert_nothing_raised(ArgumentError) { R3::Tree.new }
end

assert 'R3::Tree#initialize(int)' do
  assert_nothing_raised(ArgumentError) { R3::Tree.new(1) }
end

assert 'R3::Tree#initialize(string)' do
  assert_raise(TypeError) { R3::Tree.new('1') }
end

assert 'R3::Tree#initialize(int, int)' do
  assert_raise(ArgumentError) { R3::Tree.new(1, 1) }
end

assert 'R3::Tree#add(str)' do
  assert_nothing_raised { tree.add('/route') }
end

assert 'R3::Tree#<<(str)' do
  assert_nothing_raised { tree << '/route' }
end

assert 'R3::Tree#add(str, int)' do
  assert_nothing_raised { tree.add('/route', R3::GET) }
end

assert 'R3::Tree#add(str, int, proc)' do
  assert_nothing_raised { tree.add('/route', R3::GET, -> {}) }
end

assert 'R3::Tree#add(str, int, hash)' do
  assert_nothing_raised { tree.add('/route', R3::GET, {}) }
end

assert 'R3::Tree#add(int)' do
  assert_raise(TypeError) { tree.add(R3::GET) }
end

assert 'R3::Tree#add(int, str)' do
  assert_raise(TypeError) { tree.add(R3::GET, '/route') }
end

assert 'R3::Tree#add(str, int, int, int)' do
  assert_raise(ArgumentError) { tree.add('/route', R3::GET, 1, 1) }
end

assert 'R3::Tree#compile()' do
  tree = R3::Tree.new(1)

  assert_nothing_raised { tree.compile }
  tree << '/route1/{id}'
  tree << '/route2/{id}'
  assert_kind_of Integer, tree.compile

  tree.free
end

assert 'R3::Tree#compile(int)' do
  assert_raise(ArgumentError) { tree.compile(1) }
end

assert 'R3::Tree#match?(str)' do
  assert_true  setup_tree { |t| t << '/route' }.match? '/route'
  assert_true  setup_tree { |t| t << '/route' }.match? '/route/'
  assert_true  setup_tree { |t| t << '/route' }.match? '/route', R3::GET
  assert_false setup_tree { |t| t << '/route' }.match? '/other'
  assert_false setup_tree { |t| t << '/route' }.match? '/other', R3::DELETE
end

assert 'R3::Tree#match?(str, int)' do
  assert_true  setup_tree { |t| t.add '/', R3::GET }.match? '/', R3::GET
  assert_false setup_tree { |t| t.add '/', R3::GET }.match? '/', R3::POST
end

assert 'R3::Tree#match?(str, R3::ANY)' do
  assert_true setup_tree { |t| t.add '/', R3::GET }.match? '/', R3::ANY
  assert_true setup_tree { |t| t.add '/', R3::ANY }.match? '/', R3::GET
end

assert 'R3::Tree#match?()' do
  assert_raise(ArgumentError) { setup_tree.match? }
end

assert 'R3::Tree#match?(int)' do
  assert_raise(TypeError) { setup_tree.match? 1 }
end

assert 'R3::Tree#match?(str, int, int)' do
  assert_raise(ArgumentError) { setup_tree.match? '/', 1, 1 }
end

assert 'R3::Tree#mismatch?(str)' do
  assert_false setup_tree { |t| t << '/route' }.mismatch? '/route'
  assert_false setup_tree { |t| t << '/route' }.mismatch? '/route', R3::OPTIONS
  assert_true  setup_tree { |t| t << '/route' }.mismatch? '/other'
  assert_true  setup_tree { |t| t << '/route' }.mismatch? '/other', R3::HEAD
end

assert 'R3::Tree#mismatch?(str, int)' do
  assert_false setup_tree { |t| t.add '/', R3::GET }.mismatch? '/', R3::GET
  assert_true  setup_tree { |t| t.add '/', R3::GET }.mismatch? '/', R3::POST
end

assert 'R3::Tree#mismatch?()' do
  assert_raise(ArgumentError) { setup_tree.mismatch? }
end

assert 'R3::Tree#mismatch?(int)' do
  assert_raise(TypeError) { setup_tree.mismatch? 1 }
end

assert 'R3::Tree#mismatch?(str, int, int)' do
  assert_raise(ArgumentError) { setup_tree.mismatch? '/', 1, 1 }
end

assert 'R3::Tree#match(str)' do
  tree = setup_tree do |t|
    t.add '/user/{name}', R3::GET
    t.add '/user/{name}/{age}', R3::GET
    t.add '/user', R3::GET
  end

  params = tree.match('/other')
  assert_nil params

  params = tree.match('/user')
  assert_kind_of Hash, params
  assert_true params.empty?

  params = tree.match('/user/bernd')
  assert_equal 1, params.size
  assert_include params, :name
  assert_equal 'bernd', params[:name]

  params = tree.match('/user/bernd/99')
  assert_equal 2, params.size
  assert_include params, :name
  assert_include params, :age
  assert_equal 'bernd', params[:name]
  assert_equal '99', params[:age]

  tree.free
end

assert 'R3::Tree#match(str, int)' do
  tree = setup_tree do |t|
    t.add '/user/{name}', R3::GET
    t.add '/user/{name}/{age}', R3::GET
    t.add '/user', R3::GET
  end

  params = tree.match('/other', R3::GET)
  assert_nil params

  params = tree.match('/user', R3::GET)
  assert_kind_of Hash, params
  assert_true params.empty?

  if compiled_with_pcre?
    params = tree.match('/user/bernd', R3::GET)
    assert_equal 1, params.size
    assert_include params, :name
    assert_equal 'bernd', params[:name]
  end

  params = tree.match('/user/bernd', R3::DELETE)
  assert_nil params

  params = tree.match('/user/bernd/99', R3::GET)
  assert_equal 2, params.size
  assert_include params, :name
  assert_include params, :age
  assert_equal 'bernd', params[:name]
  assert_equal '99', params[:age]

  tree.free
end

assert 'R3::Tree#match(str)', 'return params and data' do
  tree = setup_tree do |t|
    t.add('/user/{name}', R3::ANY, -> { 'callback handler' })
  end

  params, handler = tree.match('/user/bernd')
  assert_equal({ name: 'bernd' }, params)
  assert_kind_of Proc, handler

  tree.free
end

assert 'R3::Tree#match', 'chomp does not modify string' do
  route = '/user/'
  copy  = route.dup

  tree = setup_tree { |t| t << route }

  tree.match? route
  assert_equal copy, route

  tree.match? route
  assert_equal copy, route

  tree.match route
  assert_equal copy, route

  tree.free
end

assert 'R3::Tree#match()' do
  assert_raise(ArgumentError) { setup_tree.match }
end

assert 'R3::Tree#match(int)' do
  assert_raise(TypeError) { setup_tree.match 1 }
end

assert 'R3::Tree#match(str, int, int)' do
  assert_raise(ArgumentError) { setup_tree.match '/', 1, 1 }
end

assert 'R3::Tree#free' do
  tree = setup_tree

  assert_true  tree.free
  assert_false tree.free
end
