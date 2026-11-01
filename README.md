# Numo::GSL::Integration

`numo-gsl-integration` provides numerical integration routines from the GNU Scientific Library (GSL) under the `Numo::GSL::Integration` namespace. It is intended to complement the Numo ecosystem without depending on `numo-gsl` itself.

## Requirements

- Ruby 3.2 or later
- `numo-narray`
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

`Numo::GSL::Integration.qag` integrates a Ruby block over a finite interval and returns the integral and its estimated absolute error.

```ruby
result, error = Numo::GSL::Integration.qag(0.0, 1.0) do |x|
  x * x
end

puts result
# approximately 0.3333333333333333

puts error
# estimated absolute error
```

The method currently uses fixed GSL integration parameters internally. Exceptions raised by the integrand are propagated to the caller, while errors reported by GSL are raised as `Numo::GSL::Integration::Error`.

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

The default Rake task compiles the native extension, runs the RSpec suite, and runs RuboCop.

Ruby source code is formatted with `rubyfmt`. RuboCop is used for linting, with `rubocop-rake`, `rubocop-rspec`, and `rubocop-rubyfmt`.

## License

This project is licensed under the GNU General Public License version 3 or later (`GPL-3.0-or-later`). See `LICENSE` for the full license text.
