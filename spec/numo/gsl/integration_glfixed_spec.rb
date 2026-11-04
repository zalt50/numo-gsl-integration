# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe ".glfixed" do

    let(:integrate) do
      -> (&block) { described_class.glfixed(0.0, 1.0, &block) }
    end

    it_behaves_like("an integration callback")

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

    it "requires a positive integration order" do
      expect do
        described_class.glfixed(0.0, 1.0, order: 0) { |x| x }
      end
        .to(raise_error(Numo::GSL::Integration::Error))
    end
  end
end
