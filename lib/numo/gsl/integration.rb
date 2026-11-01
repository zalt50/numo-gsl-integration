# frozen_string_literal: true

require "numo/narray"

require_relative "integration/version"
require "numo/gsl/integration/integration"

module Numo
  module GSL
    module Integration
      class Error < StandardError
      end
    end
  end
end
