# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe ".qaws" do
    it "integrates a function with algebraic endpoint singularities", :aggregate_failures do
      result, error = described_class.qaws(
        0.0,
        1.0,
        -0.5,
        -0.5,
        0,
        0
      ) { 1.0 }

      expect(result).to(be_within(1e-10).of(Math::PI))
      expect(error).to(be >= 0.0)
    end

    it "applies a logarithmic weight at the lower endpoint", :aggregate_failures do
      result, error = described_class.qaws(
        0.0,
        1.0,
        0.0,
        0.0,
        1,
        0
      ) { 1.0 }

      expect(result).to(be_within(1e-10).of(-1.0))
      expect(error).to(be >= 0.0)
    end

    it "applies a logarithmic weight at the upper endpoint", :aggregate_failures do
      result, error = described_class.qaws(
        0.0,
        1.0,
        0.0,
        0.0,
        0,
        1
      ) { 1.0 }

      expect(result).to(be_within(1e-10).of(-1.0))
      expect(error).to(be >= 0.0)
    end

    it "applies logarithmic weights at both endpoints", :aggregate_failures do
      result, error = described_class.qaws(
        0.0,
        1.0,
        0.0,
        0.0,
        1,
        1
      ) { 1.0 }

      expected = 2.0 - ((Math::PI ** 2) / 6.0)
      expect(result).to(be_within(1e-10).of(expected))
      expect(error).to(be >= 0.0)
    end

    it "propagates an exception raised by the integrand" do
      error_class = Class.new(StandardError)

      expect do
        described_class.qaws(0.0, 1.0, 0.0, 0.0, 0, 0) do
          raise error_class, "integrand failed"
        end
      end
        .to(raise_error(error_class, "integrand failed"))
    end

    it "raises when the integrand returns a non-numeric value" do
      expect do
        described_class.qaws(0.0, 1.0, 0.0, 0.0, 0, 0) { Object.new }
      end
        .to(raise_error(TypeError))
    end

    it "raises GslError directly when GSL integration fails" do
      error = begin
        described_class.qaws(
          0.0,
          1.0,
          0.0,
          0.0,
          0,
          0,
          epsabs: 0.0,
          epsrel: 0.0
        ) { 1.0 }
      rescue Numo::GSL::GslError => e
        e
      end

      expect(error).to(be_instance_of(Numo::GSL::GslError))
    end

    it "accepts integration options", :aggregate_failures do
      result, error = described_class.qaws(
        0.0,
        1.0,
        -0.5,
        0.0,
        0,
        0,
        epsabs: 1e-12,
        epsrel: 1e-12,
        limit: 100
      ) { 1.0 }

      expect(result).to(be_within(1e-10).of(2.0))
      expect(error).to(be >= 0.0)
    end
  end
end
