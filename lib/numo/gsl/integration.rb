# frozen_string_literal: true

require_relative "integration/version"
require "numo/gsl"

module Numo
  module GSL
    module Integration
      class Error < Numo::GSL::GslError
      end
    end
  end
end

require "numo/gsl/integration/integration"
