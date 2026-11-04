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
For example, use `qag` to integrate a function over a finite interval:  

```ruby  
result, error = Numo::GSL::Integration.qag(0.0, 1.0) do |x|  
  x * x  
end  
```  

## Supported routines  

| Method | Interval | Intended use |  
| --- | --- | --- |  
| `qag` | Finite | General-purpose adaptive integration |  
| `qng` | Finite | Non-adaptive integration of smooth functions |  
| `qags` | Finite | Functions with integrable singularities or discontinuities |  
| `qagp` | Finite | Known singular or discontinuity points |  
| `qagi` | Infinite | Integration over the entire real line |  
| `qagiu` | Semi-infinite | Integration from a finite lower bound to positive infinity |  
| `qagil` | Semi-infinite | Integration from negative infinity to a finite upper bound |  
| `qawc` | Finite | Cauchy principal value with a known singular point |  
| `qaws` | Finite | Algebraic and logarithmic endpoint singularities |  
| `qawo` | Finite | Sine- or cosine-weighted oscillatory integrals |  

See [Integration routines](docs/routines.md) for parameters and examples.  

## Documentation  

- [Integration routines](docs/routines.md)  
- [GSL error handling](docs/error-handling.md)  
- [Development](docs/development.md)  

## License  

This project is licensed under the [GNU General Public License version 3 or later](https://www.gnu.org/licenses/gpl-3.0.html). See the [LICENSE](LICENSE) file for the full license text.
