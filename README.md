# Numo::GSL::Integration

`numo-gsl-integration` provides numerical integration routines from the [GNU Scientific Library (GSL)](https://www.gnu.org/software/gsl/) under the `Numo::GSL::Integration` namespace. It is intended to complement the [Numo](https://github.com/ruby-numo) ecosystem without depending on [numo-gsl](https://github.com/ruby-numo/numo-gsl) itself.

## Requirements

- [Ruby](https://www.ruby-lang.org/) 3.2 or later
- GNU Scientific Library (GSL)
- `gsl-config` available in `PATH`

## Installation

Add the gem to your Gemfile:

```ruby
gem "numo-gsl-integration"
```

Then run:

```bash
bundle install
```

GSL must be installed separately. The native extension uses `gsl-config` to locate the GSL headers and libraries.

## Usage

Load the library:

```ruby
require "numo/gsl/integration"
```

### QAG integration

GSL [QAG](https://www.gnu.org/software/gsl/doc/html/integration.html#qag-adaptive-integration) provides adaptive integration over a finite interval. `Numo::GSL::Integration.qag` exposes this routine to Ruby and returns the integral and its estimated absolute error.

```ruby
result, error = Numo::GSL::Integration.qag(0.0, 1.0) do |x|
  x * x
end
```

Integration parameters can be specified with the `epsabs`, `epsrel`, `limit`, and `key` keyword arguments:

```ruby
result, error = Numo::GSL::Integration.qag(
  0.0,
  1.0,
  epsabs: 1e-12,
  epsrel: 1e-12,
  limit: 100,
  key: Numo::GSL::Integration::GAUSS21,
) { |x| x * x }
```

The available Gauss-Kronrod integration rules are `GAUSS15`, `GAUSS21`, `GAUSS31`, `GAUSS41`, `GAUSS51`, and `GAUSS61`. Exceptions raised by the integrand are propagated to the caller, while errors reported by GSL are raised as `Numo::GSL::Integration::Error`.

## GSL error handling

Loading `numo-gsl-integration` calls `gsl_set_error_handler_off()` and therefore disables GSL's default error handler process-wide. This prevents GSL from aborting the process when an integration routine reports an error and allows the gem to raise `Numo::GSL::Integration::Error` instead.

The GSL error handler is global, so loading this gem also changes error handling for other code that uses GSL in the same process.

## Development

After checking out the repository, install the dependencies:

```bash
bundle install
```

Build the native extension and run the test and lint suites:

```bash
bundle exec rake
```

The default Rake task compiles the native extension, runs the [RSpec](https://rspec.info/) suite, and runs [RuboCop](https://rubocop.org/).

Ruby source code is formatted with [rubyfmt](https://github.com/fables-tales/rubyfmt). RuboCop is used for linting, with `rubocop-rake`, `rubocop-rspec`, and `rubocop-rubyfmt`.

### clangd

The native extension requires compiler options for the Ruby and GSL headers. To make these options available to `clangd`, generate a compilation database with [Bear](https://github.com/rizsotto/Bear).

First, build the native extension:

```bash
bundle exec rake compile
```

Then rebuild the extension with Bear and create a symbolic link to the compilation database in the repository root:

```bash
build_dir="tmp/$(ruby -rrbconfig -e 'print RbConfig::CONFIG["arch"]')/integration/$(ruby -e 'print RUBY_VERSION')"
make -C "$build_dir" clean
bear --output "$build_dir/compile_commands.json" -- make -C "$build_dir"
ln -sf "$build_dir/compile_commands.json" compile_commands.json
```

The compilation database contains the compiler options used to build `integration.c`, including the Ruby and GSL header paths. It is specific to the local build environment and should not be committed to the repository. Regenerate it after changing the Ruby or GSL installation or other build settings.

## License

This project is licensed under the [GNU General Public License version 3 or later](https://www.gnu.org/licenses/gpl-3.0.html). See the [LICENSE](LICENSE) file for the full license text.
