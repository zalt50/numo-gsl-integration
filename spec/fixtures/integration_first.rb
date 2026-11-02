# frozen_string_literal: true

require "numo/gsl/integration"

abort("GslError is not defined") unless defined?(Numo::GSL::GslError)
abort("Integration::Error does not inherit from GslError") unless Numo::GSL::Integration::Error < Numo::GSL::GslError

begin
  Numo::GSL::Integration.qag(0.0, 1.0) { Float::NAN }
rescue Numo::GSL::GslError => e
  abort("GSL failure did not raise GslError directly") unless e.instance_of?(Numo::GSL::GslError)
else
  abort("GSL failure did not raise an exception")
end
