# frozen_string_literal: true

require_relative "integration/version"

module Numo
  module GSL
    module Integration
      class Error < StandardError
      end
    end
  end
end

require "numo/gsl/integration/integration"
