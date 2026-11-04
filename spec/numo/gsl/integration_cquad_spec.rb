# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe ".cquad" do
    it "integrates a function with an endpoint singularity", :aggregate_failures do
      result, error, evaluations = described_class.cquad(0.0, 1.0) do |x|
        Math.log(x) / Math.sqrt(x)
      end

      expect(result).to(be_within(1e-8).of(-4.0))
      expect(error).to(be >= 0.0)
      expect(evaluations).to(be_positive)
    end

    it "integrates a smooth function over a finite interval", :aggregate_failures do
      result, error, evaluations = described_class.cquad(0.0, 1.0) do |x|
        x * x
      end

      expect(result).to(be_within(1e-10).of(1.0 / 3.0))
      expect(error).to(be >= 0.0)
      expect(evaluations).to(be_positive)
    end

    it "propagates an exception raised by the integrand" do
      error_class = Class.new(StandardError)

      expect do
        described_class.cquad(0.0, 1.0) do
          raise error_class, "integrand failed"
        end
      end
        .to(raise_error(error_class, "integrand failed"))
    end

    it "raises when the integrand returns a non-numeric value" do
      expect do
        described_class.cquad(0.0, 1.0) { Object.new }
      end
        .to(raise_error(TypeError))
    end

    it "raises GslError directly when GSL integration fails" do
      error = begin
        described_class.cquad(
          0.0,
          1.0,
          epsabs: 0.0,
          epsrel: 0.0
        ) { |x| x * x }
      rescue Numo::GSL::GslError => e
        e
      end

      expect(error).to(be_instance_of(Numo::GSL::GslError))
    end

    it "accepts integration options", :aggregate_failures do
      result, error, evaluations = described_class
        .cquad(
          0.0,
          1.0,
          epsabs: 1e-12,
          epsrel: 1e-12,
          limit: 100
        ) { |x| x * x }

      expect(result).to(be_within(1e-10).of(1.0 / 3.0))
      expect(error).to(be >= 0.0)
      expect(evaluations).to(be_positive)
    end
  end
end
