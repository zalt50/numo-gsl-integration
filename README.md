# Numo::GSL::Integration

`numo-gsl-integration` provides numerical integration routines from the [GNU Scientific Library (GSL)](https://www.gnu.org/software/gsl/) under the `Numo::GSL::Integration` namespace. It can be installed and loaded without [`numo-gsl`](https://github.com/ruby-numo/numo-gsl) or [`numo-gsl-alt`](https://github.com/mvz/numo-gsl-alt).

## Requirements

- [Ruby](https://www.ruby-lang.org/) 3.2 or later
- GNU Scientific Library (GSL) 2.8 or later
- `gsl-config` available in `PATH`

  GSL itself must be installed separately.

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

For example, use `qag` to integrate a function over a finite interval:

```ruby
result, error = Numo::GSL::Integration.qag(0.0, 1.0) do |x|
  x * x
end
```

To reuse the allocation across repeated adaptive integrations, create an integration workspace and pass it with the `workspace:` keyword:

```ruby
Numo::GSL::Integration::Workspace.open(limit: 100) do |workspace|
  2.times do
    result, error = Numo::GSL::Integration.qag(
      0.0,
      1.0,
      workspace: workspace
    ) do |x|
      x * x
    end
  end
end
```

Reusable workspaces are supported by `qag`, `qags`, `qagp`, `qagi`, `qagiu`, `qagil`, and `qawc`. See [Integration routines](docs/routines.md) for lifecycle and capacity details.

## Supported routines

| Method    | Interval       | Intended use                                                     |
| --------- | -------------- | ---------------------------------------------------------------- |
| `qng`     | Finite         | Non-adaptive integration of smooth functions                     |
| `qag`     | Finite         | General-purpose adaptive integration                             |
| `qags`    | Finite         | Functions with integrable singularities or discontinuities       |
| `qagp`    | Finite         | Known singular or discontinuity points                           |
| `qagi`    | Infinite       | Integration over the entire real line                            |
| `qagiu`   | Semi-infinite  | Integration from a finite lower bound to positive infinity       |
| `qagil`   | Semi-infinite  | Integration from negative infinity to a finite upper bound       |
| `qawc`    | Finite         | Cauchy principal value with a known singular point               |
| `qaws`    | Finite         | Algebraic and logarithmic endpoint singularities                 |
| `qawo`    | Finite         | Sine- or cosine-weighted oscillatory integrals                   |
| `qawf`    | Semi-infinite  | Sine- or cosine-weighted Fourier integrals                       |
| `cquad`   | Finite         | Difficult integrands and endpoint singularities                  |
| `romberg` | Finite         | Smooth functions with extrapolated trapezoidal integration       |
| `glfixed` | Finite         | Fixed-order Gauss-Legendre integration of smooth functions       |
| `fixed`   | Rule-dependent | Fixed-order Gaussian quadrature with selectable weight functions |
| `lebedev` | Unit sphere    | Lebedev quadrature over the surface of the unit sphere           |

See [Integration routines](docs/routines.md) for parameters and examples.

## Documentation

- [Integration routines](docs/routines.md)
- [GSL error handling](docs/error-handling.md)
- [Development](docs/development.md)

## License

This project is licensed under the [GNU General Public License version 3 or later](https://www.gnu.org/licenses/gpl-3.0.html). See the [LICENSE](LICENSE) file for the full license text.
