# Changelog

## [0.8.0] - 2026-11-09

### Added

- Added `Numo::GSL::Integration::FixedWorkspace` for reusing fixed-point quadrature nodes and weights across calls.
- Added block-scoped workspace management with `FixedWorkspace.open`, explicit lifecycle methods with `close` and `closed?`, and accessors for the fixed quadrature parameters.
- Added `Numo::GSL::Integration::LebedevWorkspace` for reusing Lebedev spherical nodes and weights across calls, with block-scoped and explicit lifecycle management.

### Changed

- Added a `workspace:` keyword to `fixed`. The method accepts either a quadrature type with its complete rule parameters or a reusable `FixedWorkspace` by itself. Calls that omit `workspace:` retain the previous allocate-per-call behavior.
- Rejected calls that combine `workspace:` with explicit fixed quadrature parameters or supply rule parameters without a quadrature type.
- Added a `workspace:` keyword to `lebedev`. The method accepts either a node count or a reusable `LebedevWorkspace` by itself; calls that omit `workspace:` retain the previous allocate-per-call behavior.
- Rejected `lebedev` calls that combine a node count with `workspace:`.

## [0.7.0] - 2026-11-08

### Added in 0.7.0

- Added `Numo::GSL::Integration::CQUADWorkspace` and an optional `workspace:` keyword for reusing CQUAD workspace allocations across calls.
- Added `Numo::GSL::Integration::RombergWorkspace` and an optional `workspace:` keyword for reusing Romberg workspace allocations across calls.
- Added `Numo::GSL::Integration::GLFixedTable` and an optional `table:` keyword for reusing fixed-order Gauss-Legendre tables across calls.

### Changed in 0.7.0

- Validated that the CQUAD `limit` is at least `3`. When a reusable CQUAD workspace and an explicit `limit:` are supplied together, the limit must equal the workspace capacity.
- Validated that the Romberg `limit` is between `1` and `30`. When a reusable Romberg workspace and an explicit `limit:` are supplied together, the limit must equal the workspace capacity.
- When a reusable GLFIXED table is supplied, an omitted `order:` uses the table order and an explicit order must match it.

## [0.6.0] - 2026-11-08

### Added in 0.6.0

- Added `Numo::GSL::Integration::QAWFTable` for reusing Fourier-cycle QAWO moment tables with a fixed angular frequency, sine or cosine weight, and level count.

### Changed in 0.6.0

- Added optional `workspace:` and `cycle_workspace:` keywords to `qawf` so its outer Fourier-cycle workspace and per-cycle QAWO workspace can be reused independently. Calls that omit either keyword continue to allocate and release the missing workspace for that call.
- Added an optional `table:` keyword to `qawf`. Calls that omit it continue to allocate and release a temporary QAWO table.

### Fixed in 0.6.0

- Validated that the `qawf` sine or cosine selector is an integer equal to `SINE` or `COSINE`.
- Prevented open `QAWOTable` and `QAWFTable` objects from being reinitialized and leaking their native tables.
- Avoided intermediate overflow when calculating QAWF cycle lengths and rejected angular frequencies that produce a non-finite cycle length.

## [0.5.0] - 2026-11-07

### Added in 0.5.0

- Added `Numo::GSL::Integration::Workspace` for reusing adaptive-integration workspace allocations across `qag`, `qags`, `qagp`, `qagi`, `qagiu`, `qagil`, `qawc`, `qaws`, and `qawo` calls.
- Added block-scoped workspace management with `Workspace.open`, explicit lifecycle methods with `close` and `closed?`, and workspace capacity reporting with `limit`.
- Added `Numo::GSL::Integration::QAWOTable` for reusing QAWO Chebyshev-moment tables with fixed angular frequency, interval length, weight, and level count.
- Added `Numo::GSL::Integration::QAWSTable` for reusing QAWS Chebyshev-moment tables with fixed algebraic and logarithmic weight parameters.

### Changed in 0.5.0

- Added an optional `table:` keyword to `qaws` and `qawo`. Calls that omit it continue to allocate and release a temporary routine-specific table.
- Added an optional `workspace:` keyword to `qag`, `qags`, `qagp`, `qagi`, `qagiu`, `qagil`, `qawc`, `qaws`, and `qawo`. Calls that omit it retain the previous allocate-per-call behavior.

## [0.4.0] - 2026-11-06

### Added in 0.4.0

- Added Lebedev quadrature for surface integration over the unit sphere.

### Changed in 0.4.0

- Required GNU Scientific Library 2.8 or later.

## [0.3.1] - 2026-11-06

### Changed in 0.3.1

- Simplified the internal handling of fixed quadrature rules without changing the public API.

## [0.3.0] - 2026-11-05

### Added in 0.3.0

- Added fixed point quadrature with Legendre, first- and second-kind Chebyshev, Gegenbauer, Jacobi, Laguerre, Hermite, exponential, and rational rules.

## [0.2.0] - 2026-11-05

### Changed in 0.2.0

- Removed the runtime dependency on `numo-gsl-alt`.
- Defined `Numo::GSL::GslError` and installed the process-wide GSL error handler in the native extension.
- Added the numeric GSL error code to messages raised by the integration GSL error handler.
- Replaced wrapper-side validation errors with `TypeError` and `ArgumentError`.
- Changed Markdown formatting from `markdownlint-cli2 --fix` to Prettier while retaining `markdownlint-cli2` for linting.
- Extended Markdown formatting and linting to all Markdown files under the project root and `docs`.
- Added Prettier installation to the CI workflow.

### Removed in 0.2.0

- Removed `Numo::GSL::Integration::Error`.

## [0.1.0] - 2026-11-04

### Added in 0.1.0

- Added fixed-order Gauss-Legendre integration for smooth functions.
- Added Romberg integration for smooth functions with function-evaluation counts.
- Added CQUAD doubly-adaptive integration with function-evaluation counts.
- Added QAWF adaptive integration for sine- and cosine-weighted Fourier integrals over a semi-infinite interval.
- Added QAWO adaptive integration for sine- and cosine-weighted oscillatory functions.
- Added QAWS adaptive integration for algebraic and logarithmic endpoint singularities.
- Added QAWC adaptive integration for Cauchy principal values.
- Added QAGIU adaptive integration from a finite lower bound to positive infinity.
- Added QAGIL adaptive integration from negative infinity to a finite upper bound.
- Added QAGI adaptive integration over the infinite interval from negative infinity to positive infinity.
- Added QAGP adaptive integration for user-supplied singularity and discontinuity points, including validation that the points are finite and strictly increasing.
- Added QNG non-adaptive integration with result, error estimate, and function-evaluation count.
- Added QAGS adaptive integration for functions with integrable singularities and discontinuities.
- Added `numo-gsl-alt` as a runtime dependency with support for versions `>= 0.1.3` and `< 0.2`.
- Added isolated-process specs for loading `numo/gsl` before or through `numo-gsl-integration`, including representative QAG failure modes.
- Added RBS declarations for the public integration API and RBS validation to the default Rake task.
- Added automated rubyfmt, clang-format, and Markdown checks, with `format:check` and `format:apply` tasks.

### Changed in 0.1.0

- Loaded `numo/gsl` before the integration native extension and made `Numo::GSL::Integration::Error` inherit from `Numo::GSL::GslError`.
- Delegated GSL-reported integration errors to the process-wide handler installed by `numo-gsl-alt` instead of converting return statuses to `Integration::Error`.
- Ensured integration workspaces, weight tables, and temporary arrays are released when integration completes normally or raises an exception.
