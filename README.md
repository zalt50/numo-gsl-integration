# Numo::GSL::Integration

`numo-gsl-integration` provides numerical integration routines from the [GNU Scientific Library (GSL)](https://www.gnu.org/software/gsl/) under the `Numo::GSL::Integration` namespace. It depends on [`numo-gsl-alt`](https://github.com/mvz/numo-gsl-alt) for the `Numo::GSL` namespace and GSL error handling.

## Requirements

- [Ruby](https://www.ruby-lang.org/) 3.2 or later
- GNU Scientific Library (GSL)
- `gsl-config` available in `PATH`

The `numo-gsl-alt` Ruby gem is installed as a runtime dependency. GSL itself must be installed separately.

## Installation

Add the gem to your Gemfile:

```ruby
gem "numo-gsl-integration"
```

Then run:

```bash
bundle install
```

The native extension uses `gsl-config` to locate the GSL headers and libraries.

## Usage

Load the library:

```ruby
require "numo/gsl/integration"
```

The entry point loads `numo/gsl` before the integration native extension. This ensures that `numo-gsl-alt` initializes the `Numo::GSL` namespace and installs its GSL error handler before integration routines are loaded.

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

The available Gauss-Kronrod integration rules are `GAUSS15`, `GAUSS21`, `GAUSS31`, `GAUSS41`, `GAUSS51`, and `GAUSS61`.

Exceptions raised by the integrand are propagated to the caller. A non-numeric return value from the integrand raises `TypeError`.

## GSL error handling

`numo-gsl-alt` installs the process-wide GSL error handler that converts GSL errors to `Numo::GSL::GslError`. `numo-gsl-integration` neither replaces nor temporarily disables that handler.

Errors raised directly by the GSL handler are reported as `Numo::GSL::GslError`. Integration-specific failures that return to the wrapper are reported as `Numo::GSL::Integration::Error`, which inherits from `Numo::GSL::GslError`:

```text
StandardError
└── Numo::GSL::GslError
    └── Numo::GSL::Integration::Error
```

Code can therefore rescue all GSL-related errors with `Numo::GSL::GslError`, or rescue `Numo::GSL::Integration::Error` when it needs to handle only integration-specific failures.

The GSL error handler is shared by the entire process. This gem leaves its ownership with `numo-gsl-alt` and does not switch the handler around individual integration calls.

## Development

After checking out the repository, install the dependencies:

```bash
bundle install
```

Build the native extension and run the complete verification suite:

```bash
bundle exec rake
```

The default Rake task performs the following checks:

- compiles the native extension;
- runs the [RSpec](https://rspec.info/) suite;
- runs [RuboCop](https://rubocop.org/);
- checks Ruby files with [rubyfmt](https://github.com/fables-tales/rubyfmt); and
- checks C source files with `clang-format` using the LLVM style.

Check source formatting without applying changes:

```bash
bundle exec rake format:check
```

Format the Ruby and C source files:

```bash
bundle exec rake format:apply
```

RuboCop is configured with `rubocop-rake`, `rubocop-rspec`, and `rubocop-rubyfmt`.

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
