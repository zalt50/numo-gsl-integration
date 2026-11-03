# frozen_string_literal: true
RSpec.describe Numo::GSL::Integration do
  describe ".qagil" do
    it "integrates a function over a lower semi-infinite interval", :aggregate_failures do
      result, error = described_class.qagil(0.0) { |x| Math.exp(x) }
      expect(result).to(be_within(1e-10).of(1.0))
      expect(error).to(be >= 0.0)
    end

    it "propagates an exception raised by the integrand" do
      error_class = Class.new(StandardError)
      expect do
        described_class.qagil(0.0) { raise error_class, "integrand failed" }
      end
        .to(raise_error(error_class, "integrand failed"))
    end

    it "raises when the integrand returns a non-numeric value" do
      expect do
        described_class.qagil(0.0) { Object.new }
      end
        .to(raise_error(TypeError))
    end

    it "raises GslError directly when GSL integration fails" do
      error = begin
        described_class.qagil(
          0.0,
          epsabs: 0.0,
          epsrel: 0.0
        ) { |x| Math.exp(x) }
      rescue Numo::GSL::GslError => e
        e
      end

      expect(error).to(be_instance_of(Numo::GSL::GslError))
    end

    it "accepts integration options", :aggregate_failures do
      result, error = described_class.qagil(
        -1.0,
        epsabs: 1e-12,
        epsrel: 1e-12,
        limit: 100
      ) { |x| Math.exp(x) }
      expect(result).to(be_within(1e-10).of(Math.exp(-1.0)))
      expect(error).to(be >= 0.0)
    end
  end
end
