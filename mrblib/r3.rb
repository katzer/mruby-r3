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

module R3
  # Method code for written HTTP method name.
  #
  # :call-sequence:
  # => R3.method_code 'GET'
  # => R3::GET
  #
  # @param [ String ] Uppercase name of the HTTP method.
  #
  # @return [ Integer ]
  def self.method_code(method)
    const = const_get(method) if const_defined? method
    const.is_a?(Integer) ? const : nil
  rescue StandardError
    nil
  end

  # Written HTTP method name for the method code.
  #
  # :call-sequence:
  # => R3.method_name R3::GET
  # => 'GET'
  #
  # @param [ Int ] code The method code.
  #
  # @return [ String ]
  def self.method_name(code)
    case code
    when ANY     then 'ANY'
    when GET     then 'GET'
    when PUT     then 'PUT'
    when POST    then 'POST'
    when DELETE  then 'DELETE'
    when HEAD    then 'HEAD'
    when OPTIONS then 'OPTIONS'
    when PATCH   then 'PATCH'
    end
  end

  class Tree
    attr_reader :routes
  end
end
