mruby-r3 [![Build Status](https://travis-ci.org/katzer/mruby-r3.svg?branch=master)](https://travis-ci.org/katzer/mruby-r3) [![Build status](https://ci.appveyor.com/api/projects/status/dhiknegayv8k18mw/branch/master?svg=true)](https://ci.appveyor.com/project/katzer/mruby-r3/branch/master)
--------

mruby binding for [lib3r][r3], the high-performance path dispatching library.

The code compiles nativ on Linux, OSX and Windows. Cross-compilation is also supported if you [mruby-cli][mruby-cli] and optional a more up-to-date [docker image][appplant/mruby-cli-glibc-docker].

## Installation

Add the line below to your `build_config.rb`:

```ruby
MRuby::Build.new do |conf|

    # ... (snip) ...

    conf.gem 'mruby-r3'
end
```

Or add this line to your aplication's `mrbgem.rake`:

```ruby
MRuby::Gem::Specification.new('your-mrbgem') do |spec|

    # ... (snip) ...

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

The pattern syntax for routes is the following:

    /blog/post/{id}      use [^/]+ regular expression by default.
    /blog/post/{id:\d+}  use `\d+` regular expression instead of default.

They can be added to the tree at any time, however dont forget to call __compile__ before using them.

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
# 1.
tree << '/'
tree.compile

tree.match? '/'
# => true
tree.match? '/', R3::HEAD
# => true
tree.match? '/users'
# => false

# 2.
tree.add '/users', R3::ANY
tree.compile

tree.match? '/users', R3::GET
# => true
tree.match? '/users', R3::POST
# => true

# 3.
tree.add '/posts', R3::GET
tree.compile

tree.match? '/posts', R3::GET
# => true
tree.match? '/posts', R3::POST
# => false
tree.mismatch? '/posts', R3::POST
# => true
```

And finally the most important part.

```ruby
# 1.
tree << '/'
tree.compile

tree.match '/'
# => {}
tree.match '/users'
# => nil

tree.add '/users/{id}'
tree.add '/users/{user_id}/feeds/{feed_id}', R3::GET
tree.compile

tree.match '/users/1'
# => { id: '1' }
tree.match '/users/1/feeds/2'
# => { user_id: '1', feed_id: '2' }
tree.match '/users/1/feeds/2', R3::POST
# => nil
tree.match '/users/1/feeds/2/post/3'
# => nil
```


## Development

Clone the repo:
    
    $ git clone https://github.com/katzer/mruby-r3.git && cd mruby-r3/

Compile the source:

    $ rake compile

Run the tests:

    $ rake test


## TODO

It's not yet possible to add a matching route for multiple HTTP methods. Either any or only one are possible. 


## Authors

- Sebastián Katzer, Fa. appPlant GmbH


## License

The mgem is available as open source under the terms of the [MIT License][license].

Made with :yum: from Leipzig

© 2017 [appPlant GmbH][appplant]

[r3]: https://github.com/c9s/r3
[mruby-cli]: https://github.com/hone/mruby-cli
[appplant/mruby-cli-glibc-docker]: https://github.com/appPlant/mruby-cli-glibc-docker
[license]: http://opensource.org/licenses/MIT
[appplant]: www.appplant.de
