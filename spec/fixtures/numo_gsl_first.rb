# frozen_string_literal: true
require "numo/gsl"

2.times { require "numo/gsl/integration" }
result, error = Numo::GSL::Integration.qag(0.0, 1.0) { |x| x * x }
difference = (result - (1.0 / 3.0)).abs
abort("integration result is incorrect") unless difference < 1e-10
abort("integration error is negative") unless error >= 0.0
begin
  Numo::GSL::Integration.qag(0.0, 1.0, epsabs: 0.0, epsrel: 0.0) { |x| x * x }
rescue Numo::GSL::GslError => e
  abort("integration did not install its GSL error handler") unless e.message.match?(/\(GSL error \d+\)\z/)
else
  abort("GSL failure did not raise an exception")
end
