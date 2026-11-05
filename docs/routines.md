# Integration routines

Load the library before calling an integration routine:

```ruby
require "numo/gsl/integration"
```

Exceptions raised by an integrand are propagated to the caller. A non-numeric return value from an integrand raises `TypeError`.

## QNG integration

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
) do |x|
  x * x
end
```

## QAG integration

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
) do |x|
  x * x
end
```

The `key` keyword selects the Gauss-Kronrod integration rule and defaults to `GAUSS61`. The available values are `GAUSS15`, `GAUSS21`, `GAUSS31`, `GAUSS41`, `GAUSS51`, and `GAUSS61`.

## QAGS integration

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
) do |x|
  x * x
end
```

## QAGP integration

GSL [QAGP](https://www.gnu.org/software/gsl/doc/html/integration.html#qagp-adaptive-integration-with-known-singular-points) applies adaptive integration when the locations of interior singularities or discontinuities are known. Pass an array containing the left endpoint, zero or more known interior singularity or discontinuity points, and the right endpoint:

```ruby
result, error = Numo::GSL::Integration.qagp(
  [0.0, 0.5, 1.0]
) do |x|
  1.0 / Math.sqrt((x - 0.5).abs)
end
```

The points must be finite numeric values in strictly increasing order. The array must contain at least two values. A non-array argument or a non-numeric element raises `TypeError`. Too few points, non-finite values, or values that are not strictly increasing raise `ArgumentError` before GSL is called.
The `epsabs`, `epsrel`, and `limit` keyword arguments control the requested error limits and maximum number of subintervals:

```ruby
result, error = Numo::GSL::Integration.qagp(
  [0.0, 0.25, 0.75, 1.0],
  epsabs: 1e-12,
  epsrel: 1e-12,
  limit: 100
) do |x|
  x * x
end
```

## QAGI integration

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
) do |x|
  1.0 / (1.0 + (x * x))
end
```

## QAGIU integration

GSL [QAGIU](https://www.gnu.org/software/gsl/doc/html/integration.html#qagi-adaptive-integration-on-infinite-intervals) provides adaptive integration from a finite lower bound to positive infinity. `Numo::GSL::Integration.qagiu` takes the lower bound and returns the integral and its estimated absolute error.

```ruby
result, error = Numo::GSL::Integration.qagiu(0.0) do |x|
  Math.exp(-x)
end
```

The example integral has an exact value of `1.0`. The `epsabs`, `epsrel`, and `limit` keyword arguments control the requested error limits and maximum number of subintervals:

```ruby
result, error = Numo::GSL::Integration.qagiu(
  1.0,
  epsabs: 1e-12,
  epsrel: 1e-12,
  limit: 100
) do |x|
  Math.exp(-x)
end
```

## QAGIL integration

GSL [QAGIL](https://www.gnu.org/software/gsl/doc/html/integration.html#qagi-adaptive-integration-on-infinite-intervals) provides adaptive integration from negative infinity to a finite upper bound. `Numo::GSL::Integration.qagil` takes the upper bound and returns the integral and its estimated absolute error.

```ruby
result, error = Numo::GSL::Integration.qagil(0.0) do |x|
  Math.exp(x)
end
```

The example integral has an exact value of `1.0`. The `epsabs`, `epsrel`, and `limit` keyword arguments control the requested error limits and maximum number of subintervals:

```ruby
result, error = Numo::GSL::Integration.qagil(
  -1.0,
  epsabs: 1e-12,
  epsrel: 1e-12,
  limit: 100
) do |x|
  Math.exp(x)
end
```

## QAWC integration

GSL [QAWC](https://www.gnu.org/software/gsl/doc/html/integration.html#qawc-adaptive-integration-for-cauchy-principal-values) computes the Cauchy principal value (PV) of an integral over a finite interval with a singularity at $c$:

```math
PV \int_a^b \frac{f(x)}{x-c}\,dx = \lim_{\varepsilon\to 0^{+}} \left( \int_a^{c-\varepsilon}\frac{f(x)}{x-c}\,dx + \int_{c+\varepsilon}^b\frac{f(x)}{x-c}\,dx \right)
```

The block passed to `Numo::GSL::Integration.qawc` returns $f(x)$. Do not divide the block result by $x - c$; QAWC applies that weight internally.
The following example uses the default integration options and computes the principal value of `x / (x - 1.0)` over the interval from `0.0` to `2.0`, whose exact value is `2.0`:

```ruby
result, error = Numo::GSL::Integration.qawc(0.0, 2.0, 1.0) do |x|
  x
end
```

The `epsabs`, `epsrel`, and `limit` keyword arguments control the requested error limits and maximum number of subintervals. The following example computes the principal value of `1.0 / x` over the symmetric interval from `-1.0` to `1.0`, whose exact value is `0.0`. It specifies a positive `epsabs` because a relative error bound alone is not suitable when the expected result is zero:

```ruby
result, error = Numo::GSL::Integration.qawc(
  -1.0,
  1.0,
  0.0,
  epsabs: 1e-12,
  epsrel: 1e-12,
  limit: 100
) do |_x|
  1.0
end
```

## QAWS integration

GSL [QAWS](https://www.gnu.org/software/gsl/doc/html/integration.html#qaws-adaptive-integration-for-singular-functions) provides adaptive integration for functions with algebraic or logarithmic singularities at the endpoints of a finite interval. It computes an integral with the weight

```math
(x-a)^\alpha (b-x)^\beta
\log^\mu(x-a) \log^\nu(b-x).
```

Call `Numo::GSL::Integration.qaws` with the endpoints `a` and `b`, followed by `alpha`, `beta`, `mu`, and `nu`. The exponents `alpha` and `beta` must be greater than `-1.0`. The logarithmic powers `mu` and `nu` must each be `0` or `1`.
The block returns the unweighted function $f(x)$. QAWS applies the endpoint weight internally. The following example integrates

```math
\frac{1}{\sqrt{x(1-x)}}
```

over the interval from `0.0` to `1.0`. Its exact value is `Math::PI`:

```ruby
result, error = Numo::GSL::Integration.qaws(
  0.0,
  1.0,
  -0.5,
  -0.5,
  0,
  0
) do |_x|
  1.0
end
```

Set `mu` to `1` to include `log(x - a)`, and set `nu` to `1` to include `log(b - x)`. The `epsabs`, `epsrel`, and `limit` keyword arguments control the requested error limits and maximum number of subintervals:

```ruby
result, error = Numo::GSL::Integration.qaws(
  0.0,
  1.0,
  0.0,
  0.0,
  1,
  0,
  epsabs: 1e-12,
  epsrel: 1e-12,
  limit: 100
) do |_x|
  1.0
end
```

This example computes the integral of `log(x)` from `0.0` to `1.0`, whose exact value is `-1.0`.

## QAWO integration

GSL [QAWO](https://www.gnu.org/software/gsl/doc/html/integration.html#qawo-adaptive-integration-for-oscillatory-functions) provides adaptive integration for finite-interval integrals with a sine or cosine weight. The block returns the unweighted function; QAWO applies the oscillatory weight internally.

```ruby
result, error = Numo::GSL::Integration.qawo(
  0.0,
  Math::PI,
  1.0,
  Numo::GSL::Integration::SINE
) do |_x|
  1.0
end
```

The `epsabs`, `epsrel`, and `limit` keyword arguments control the requested error limits and workspace capacity.

## QAWF integration

GSL [QAWF](https://www.gnu.org/software/gsl/doc/html/integration.html#qawf-adaptive-integration-for-fourier-integrals) provides adaptive integration for sine- or cosine-weighted Fourier integrals over a semi-infinite interval.

```ruby
result, error = Numo::GSL::Integration.qawf(
  0.0,
  1.0,
  Numo::GSL::Integration::COSINE
) do |x|
  Math.exp(-x)
end
```

The `epsabs` keyword controls the requested absolute error. The `limit` keyword controls the maximum number of Fourier cycles and the allocated workspace capacity.

## CQUAD integration

GSL [CQUAD](https://www.gnu.org/software/gsl/doc/html/integration.html#cquad-doubly-adaptive-integration) provides doubly-adaptive integration over a finite interval. It is intended for difficult integrands, including functions with integrable singularities. The return value contains the integral, its estimated absolute error, and the number of integrand evaluations.

```ruby
result, error, evaluations = Numo::GSL::Integration.cquad(0.0, 1.0) do |x|
  Math.log(x) / Math.sqrt(x)
end
```

The example integral has an endpoint singularity and an exact value of `-4.0`. The `epsabs` and `epsrel` keyword arguments control the requested error limits. The `limit` keyword controls the number of intervals retained in the CQUAD workspace and defaults to `100`; GSL requires at least `3`.

## Romberg integration

GSL [Romberg integration](https://www.gnu.org/software/gsl/doc/html/integration.html#romberg-integration) applies Richardson extrapolation to successive trapezoidal approximations. It is intended for smooth functions over a finite interval. The return value contains the integral and the number of integrand evaluations.

```ruby
result, evaluations = Numo::GSL::Integration.romberg(0.0, Math::PI) do |x|
  Math.sin(x)
end
```

The example integral has an exact value of `2.0`. The `epsabs` and `epsrel` keyword arguments control the requested error limits. The `limit` keyword controls the maximum number of Romberg levels and defaults to `20`. GSL requires at least one level and limits the effective workspace to 30 levels.
Romberg integration does not return an estimated absolute error. Use CQUAD, QAG, QAGS, or another routine that returns an error estimate when the caller needs one.

## GLFIXED integration

GSL [fixed-order Gauss-Legendre integration](https://www.gnu.org/software/gsl/doc/html/integration.html#gauss-legendre-integration) provides fast integration of smooth functions when an appropriate polynomial order is known. An `n`-point rule integrates polynomials of degree `2 * n - 1` or less exactly, subject to floating-point precision.

```ruby
result = Numo::GSL::Integration.glfixed(
  0.0,
  1.0,
  order: 3
) do |x|
  x**4
end
```

The example integral has an exact value of `1.0 / 5.0`. The `order` keyword controls the number of Gauss-Legendre points and defaults to `10`. It must be a positive integer.
GLFIXED returns only the integral. It does not accept absolute or relative error bounds and does not return an error estimate. Use an adaptive routine when the required fixed order is not known in advance.

## Fixed point quadratures

GSL [fixed point quadratures](https://www.gnu.org/software/gsl/doc/html/integration.html#fixed-point-quadratures) provide fixed-order Gaussian quadrature rules for several weight functions. `Numo::GSL::Integration.fixed` takes a quadrature type, the number of points, and the GSL rule parameters:

```ruby
result = Numo::GSL::Integration.fixed(
  :legendre,
  order: 3,
  a: 0.0,
  b: 1.0
) do |x|
  x ** 4
end
```

The example has an exact value of `1.0 / 5.0`. The block returns the unweighted function. For weighted rules, GSL applies the selected weight internally.

The supported types and required keyword arguments are:

| Type           | Required rule parameters | Description                                                  |
| -------------- | ------------------------ | ------------------------------------------------------------ |
| `:legendre`    | —                        | Gauss-Legendre rule over the finite interval from `a` to `b` |
| `:chebyshev`   | —                        | First-kind Chebyshev rule over the finite interval           |
| `:chebyshev2`  | —                        | Second-kind Chebyshev rule over the finite interval          |
| `:gegenbauer`  | `alpha`                  | Gegenbauer rule                                              |
| `:jacobi`      | `alpha`, `beta`          | Jacobi rule                                                  |
| `:laguerre`    | `alpha`                  | Generalized Laguerre rule                                    |
| `:hermite`     | `alpha`                  | Generalized Hermite rule                                     |
| `:exponential` | `alpha`                  | Exponential rule over a finite interval                      |
| `:rational`    | `alpha`, `beta`          | Rational rule                                                |

`order` must be a positive integer. The keywords `a` and `b` are required for every type because they are passed to `gsl_integration_fixed_alloc`; their exact role and valid range depend on the selected GSL rule. The `alpha` and `beta` values must also satisfy the constraints imposed by that rule. Invalid numerical parameters are reported as `Numo::GSL::GslError` by GSL.

The following example uses the rational rule. Its parameters satisfy the rule's order-dependent constraints for `order: 3`:

```ruby
result = Numo::GSL::Integration.fixed(
  :rational,
  order: 3,
  a: 0.0,
  b: 1.0,
  alpha: 0.0,
  beta: -7.0
) do |_x|
  1.0
end
```

The example result is `1.0 / 6.0`. Fixed point quadratures return only the integral. They do not accept absolute or relative error bounds and do not return an error estimate. Use an adaptive routine when the required order is not known in advance.
