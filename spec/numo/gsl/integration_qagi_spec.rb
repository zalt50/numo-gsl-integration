# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe ".qagi" do
    it "integrates a function over an infinite interval", :aggregate_failures do
      result, error = described_class.qagi do |x|
        Math.exp(-(x * x))
      end

      expect(result).to(be_within(1e-10).of(Math.sqrt(Math::PI)))
      expect(error).to(be >= 0.0)
    end

    it "propagates an exception raised by the integrand" do
      error_class = Class.new(StandardError)

      expect do
        described_class.qagi { raise error_class, "integrand failed" }
      end
        .to(raise_error(error_class, "integrand failed"))
    end

    it "raises when the integrand returns a non-numeric value" do
      expect do
        described_class.qagi { Object.new }
      end
        .to(raise_error(TypeError))
    end

    it "raises GslError directly when GSL integration fails" do
      error = begin
        described_class.qagi(
          epsabs: 0.0,
          epsrel: 0.0
        ) { |x| x * x }
      rescue Numo::GSL::GslError => e
        e
      end

      expect(error).to(be_instance_of(Numo::GSL::GslError))
    end

    it "accepts integration options", :aggregate_failures do
      result, error = described_class.qagi(
        epsabs: 1e-12,
        epsrel: 1e-12,
        limit: 100
      ) { |x| 1.0 / (1.0 + (x * x)) }

      expect(result).to(be_within(1e-10).of(Math::PI))
      expect(error).to(be >= 0.0)
    end
  end
end
