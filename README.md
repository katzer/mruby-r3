# mruby-r3 [![Build Status](https://travis-ci.org/katzer/mruby-r3.svg?branch=master)](https://travis-ci.org/katzer/mruby-r3) [![Build status](https://ci.appveyor.com/api/projects/status/dhiknegayv8k18mw/branch/master?svg=true)](https://ci.appveyor.com/project/katzer/mruby-r3/branch/master)

mruby binding for [lib3r][r3], a high-performance path dispatching library.

```ruby
tree << '/users/{user_id}/feeds/{feed_id}'

tree.match '/users/1/feeds/2'
# => { user_id: '1', feed_id: '2' }
```


## Installation

Add the line below to your `build_config.rb`:

```ruby
MRuby::Build.new do |conf|
  # ... (snip) ...
  conf.gem mgem: 'mruby-regexp-pcre' # Optional
  conf.gem mgem: 'mruby-r3'
end
```

Or add this line to your aplication's `mrbgem.rake`:

```ruby
MRuby::Gem::Specification.new('your-mrbgem') do |spec|
  # ... (snap) ...
  spec.add_dependency 'mruby-regexp-pcre' # Optional
  spec.add_dependency 'mruby-r3'
end
```


## Usage

First of all a tree object has to be created.

```ruby
tree = R3::Tree.new
```

The size grows dynamically. However the initializer accepts an optional initial size.
The default initial size is up to 5 routes.

```ruby
tree = R3::Tree.new(100)
```

The pattern syntax for routes is the following. Enhanced support for regular expression requires __mruby-regexp-pcre__ to be installed!

    /blog/post/{id}      use [^/]+ regular expression by default.
    /blog/post/{id:\d+}  use `\d+` regular expression instead of default.

Routes can be added to the tree at any time, however dont forget to call __compile__ before using them.

```ruby
tree << '/'
tree << '/blog/post'
tree << '/blog/post/{id}'
tree << '/user/{user_id}/feeds/{feed_id}'
```

Its also possible to specify the HTML method rather then to allow any.

```ruby
tree.add('/blog/post/{id:\\d+}', R3::DELETE)
```

Once the tree has been compiled he's ready for dispatching.

```ruby
tree << '/'
tree.add '/users', R3::ANY
tree.add '/posts', R3::GET
tree.add '/users/{user_id}/feeds/{feed_id}', R3::GET
tree.compile

tree.match? '/'
# => true
tree.match '/'
# => {}

tree.match? '/other'
# => false
tree.match '/other'
# => nil

tree.match? '/users', R3::POST
# => true
tree.match? '/posts', R3::POST
# => false

tree.match '/users/1/feeds/2'
# => { user_id: '1', feed_id: '2' }
tree.match '/users/1/feeds/2', R3::POST
# => nil
```

Before you're writing your own URL map, you can make use of the built-in feature to add any kind of data with the route.

```ruby
tree.add('/user/{name}', R3::ANY, -> { 'callback handler' })
tree.compile

params, handler = tree.match('/user/bernd')
handler.call
# => 'callback handler'
```


## Development

Clone the repo:
    
    $ git clone https://github.com/katzer/mruby-r3.git && cd mruby-r3/

Compile the source:

    $ rake compile

Run the tests:

    $ rake test


## TODO

1. Add a route for multiple HTTP methods.


## Authors

- Sebastián Katzer, Fa. appPlant GmbH


## License

The mgem is available as open source under the terms of the [MIT License][license].

Made with :yum: from Leipzig

© 2017 [appPlant GmbH][appplant]

[r3]: https://github.com/c9s/r3
[license]: http://opensource.org/licenses/MIT
[appplant]: www.appplant.de
