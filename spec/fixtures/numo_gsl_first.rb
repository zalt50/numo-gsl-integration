# frozen_string_literal: true

require "numo/gsl"

2.times { require "numo/gsl/integration" }

result, error = Numo::GSL::Integration.qag(0.0, 1.0) { |x| x * x }
difference = (result - (1.0 / 3.0)).abs
abort("integration result is incorrect") unless difference < 1e-10
abort("integration error is negative") unless error >= 0.0

begin
  Numo::GSL::Integration.qag(0.0, 1.0) { Float::NAN }
rescue Numo::GSL::GslError => e
  abort("integration replaced the numo-gsl-alt error handler") unless e.instance_of?(Numo::GSL::GslError)
else
  abort("GSL failure did not raise an exception")
end
