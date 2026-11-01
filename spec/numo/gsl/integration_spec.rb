# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  it "has a version number" do
    expect(Numo::GSL::Integration::VERSION).not_to(be_nil)
  end

  describe ".qag" do
    it "integrates a function over a finite interval", :aggregate_failures do
      result, error = described_class.qag(0.0, 1.0) do |x|
        x * x
      end

      expect(result).to(be_within(1e-10).of(1.0 / 3.0))
      expect(error).to(be >= 0.0)
    end

    it "propagates an exception raised by the integrand" do
      error_class = Class.new(StandardError)

      expect do
        described_class.qag(0.0, 1.0) { raise error_class, "integrand failed" }
      end
        .to(raise_error(error_class, "integrand failed"))
    end

    it "raises when the integrand returns a non-numeric value" do
      expect do
        described_class.qag(0.0, 1.0) { Object.new }
      end
        .to(raise_error(TypeError))
    end

    it "raises Integration::Error when GSL integration fails" do
      expect do
        described_class.qag(0.0, 1.0) { Float::NAN }
      end
        .to(raise_error(Numo::GSL::Integration::Error))
    end
  end
end
