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

### QNG integration

GSL [QNG](https://www.gnu.org/software/gsl/doc/html/integration.html#qng-non-adaptive-gauss-kronrod-integration) provides non-adaptive integration for smooth functions over a finite interval. It applies increasingly accurate Gauss-Kronrod-Patterson rules and evaluates the integrand at up to 87 points.

```ruby
result, error, evaluations = Numo::GSL::Integration.qng(0.0, 1.0) do |x|
  x * x
end
```

The return value contains the integral, its estimated absolute error, and the number of integrand evaluations. The `epsabs` and `epsrel` keyword arguments control the requested absolute and relative error limits:

```ruby
result, error, evaluations = Numo::GSL::Integration.qng(
  0.0,
  1.0,
  epsabs: 1e-12,
  epsrel: 1e-12
) { |x| x * x }
```

### QAGS integration

GSL [QAGS](https://www.gnu.org/software/gsl/doc/html/integration.html#qags-adaptive-integration-with-singularities) combines adaptive subdivision with extrapolation to integrate functions with integrable singularities or discontinuities over a finite interval.

```ruby
result, error = Numo::GSL::Integration.qags(0.0, 1.0) do |x|
  Math.log(x) / Math.sqrt(x)
end
```

The example integral has an endpoint singularity and an exact value of `-4.0`. The `epsabs`, `epsrel`, and `limit` keyword arguments control the requested error limits and maximum number of subintervals:

```ruby
result, error = Numo::GSL::Integration.qags(
  0.0,
  1.0,
  epsabs: 1e-12,
  epsrel: 1e-12,
  limit: 100
) { |x| x * x }
```

### QAGP integration

GSL [QAGP](https://www.gnu.org/software/gsl/doc/html/integration.html#qagp-adaptive-integration-with-known-singular-points) applies adaptive integration when the locations of interior singularities or discontinuities are known. Pass an array containing the left endpoint, zero or more known interior singularity or discontinuity points, and the right endpoint:

```ruby
result, error = Numo::GSL::Integration.qagp(
  [0.0, 0.5, 1.0]
) { |x| 1.0 / Math.sqrt((x - 0.5).abs) }
```

The points must be finite numeric values in strictly increasing order. The array must contain at least two values. Invalid point arrays raise `Numo::GSL::Integration::Error` before GSL is called.

The `epsabs`, `epsrel`, and `limit` keyword arguments control the requested error limits and maximum number of subintervals:

```ruby
result, error = Numo::GSL::Integration.qagp(
  [0.0, 0.25, 0.75, 1.0],
  epsabs: 1e-12,
  epsrel: 1e-12,
  limit: 100
) { |x| x * x }
```

### QAGI integration

GSL [QAGI](https://www.gnu.org/software/gsl/doc/html/integration.html#qagi-adaptive-integration-on-infinite-intervals) provides adaptive integration over the infinite interval from negative infinity to positive infinity. `Numo::GSL::Integration.qagi` takes no interval endpoints and returns the integral and its estimated absolute error.

```ruby
result, error = Numo::GSL::Integration.qagi do |x|
  Math.exp(-(x * x))
end
```

The example integrates the Gaussian function, whose exact value is `Math.sqrt(Math::PI)`. The `epsabs`, `epsrel`, and `limit` keyword arguments control the requested error limits and maximum number of subintervals:

```ruby
result, error = Numo::GSL::Integration.qagi(
  epsabs: 1e-12,
  epsrel: 1e-12,
  limit: 100
) { |x| 1.0 / (1.0 + (x * x)) }
```

## GSL error handling

`numo-gsl-alt` installs the process-wide GSL error handler that converts GSL errors to `Numo::GSL::GslError`. `numo-gsl-integration` neither replaces nor temporarily disables that handler.

Errors reported by GSL are raised directly as `Numo::GSL::GslError` by the handler installed by `numo-gsl-alt`. `numo-gsl-integration` does not convert integration return statuses into separate exceptions. The workspaces used by QAG, QAGS, QAGP, and QAGI are released when a call completes normally or raises an exception. QAGP also releases its temporary copy of the point array.

`Numo::GSL::Integration::Error` is reserved for validation or state errors detected by `numo-gsl-integration` itself. It inherits from `Numo::GSL::GslError`:

```text
StandardError
└── Numo::GSL::GslError
    └── Numo::GSL::Integration::Error
```

Code can rescue all GSL-related errors with `Numo::GSL::GslError`, or rescue `Numo::GSL::Integration::Error` when it needs to handle only errors detected by the integration wrapper.

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
- validates RBS signatures;
- checks Ruby files with [rubyfmt](https://github.com/fables-tales/rubyfmt);
- checks C source files with `clang-format` using the LLVM style; and
- checks `README.md` and `CHANGELOG.md` with `markdownlint-cli2`.

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

The compilation database contains the compiler options used to build the native extension, including the Ruby and GSL header paths. It is specific to the local build environment and should not be committed to the repository. Regenerate it after changing the Ruby or GSL installation or other build settings.

## License

This project is licensed under the [GNU General Public License version 3 or later](https://www.gnu.org/licenses/gpl-3.0.html). See the [LICENSE](LICENSE) file for the full license text.
