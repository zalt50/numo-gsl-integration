# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe ".glfixed" do
    it "integrates a smooth function over a finite interval" do
      result = described_class.glfixed(0.0, Math::PI) do |x|
        Math.sin(x)
      end

      expect(result).to(be_within(1e-10).of(2.0))
    end

    it "integrates a polynomial exactly for a sufficient order" do
      result = described_class.glfixed(0.0, 1.0, order: 3) do |x|
        x ** 4
      end

      expect(result).to(be_within(1e-14).of(1.0 / 5.0))
    end

    it "integrates over a non-unit interval" do
      result = described_class.glfixed(-1.0, 2.0, order: 2) do |x|
        x
      end

      expect(result).to(be_within(1e-14).of(1.5))
    end

    it "propagates an exception raised by the integrand" do
      error_class = Class.new(StandardError)

      expect do
        described_class.glfixed(0.0, 1.0) do
          raise error_class, "integrand failed"
        end
      end
        .to(raise_error(error_class, "integrand failed"))
    end

    it "raises when the integrand returns a non-numeric value" do
      expect do
        described_class.glfixed(0.0, 1.0) { Object.new }
      end
        .to(raise_error(TypeError))
    end

    it "requires a positive integration order" do
      expect do
        described_class.glfixed(0.0, 1.0, order: 0) { |x| x }
      end
        .to(raise_error(Numo::GSL::Integration::Error))
    end
  end
end
