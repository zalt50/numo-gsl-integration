# frozen_string_literal: true

require "numo/gsl/integration"

abort("GslError is not defined") unless defined?(Numo::GSL::GslError)
abort("Integration::Error does not inherit from GslError") unless Numo::GSL::Integration::Error < Numo::GSL::GslError

assert_direct_gsl_error = lambda do |label, **options, &integrand|
  Numo::GSL::Integration.qag(0.0, 1.0, **options, &integrand)
rescue Numo::GSL::GslError => e
  abort("#{label} did not raise GslError directly") unless e.instance_of?(Numo::GSL::GslError)
else
  abort("#{label} did not raise an exception")
end

assert_direct_gsl_error.call("bad tolerance", epsabs: 0.0, epsrel: 0.0) { |x| x * x }
assert_direct_gsl_error
  .call(
    "maximum subdivisions",
    epsabs: 0.0,
    epsrel: 1e-12,
    limit: 2,
    key: Numo::GSL::Integration::GAUSS15
  ) { |x| Math.sin(1.0 / x) }
