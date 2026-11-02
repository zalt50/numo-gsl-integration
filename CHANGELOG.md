# Changelog

## [Unreleased]

### Added

- Added `numo-gsl-alt` as a runtime dependency with support for versions `>= 0.1.3` and `< 0.2`.
- Added isolated-process specs for loading `numo/gsl` before or through `numo-gsl-integration`, including representative QAG failure modes.
- Added RBS declarations for the public integration API and RBS validation to the default Rake task.
- Added automated rubyfmt and clang-format checks, with `format:check` and `format:apply` tasks.

### Changed

- Load `numo/gsl` before the integration native extension and make `Numo::GSL::Integration::Error` inherit from `Numo::GSL::GslError`.
- Delegate GSL-reported QAG errors to the process-wide handler installed by `numo-gsl-alt` instead of converting return statuses to `Integration::Error`.
- Ensure the QAG workspace is released when integration completes normally or raises an exception.

## [0.1.0] - 2026-11-01

- Initial release
