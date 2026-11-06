# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe ".lebedev" do
    let(:integrate) do
      lambda do |&block|
        described_class.lebedev(26, &block)
      end
    end

    it_behaves_like("an integration callback")

    it "integrates a constant over the unit sphere" do
      result = described_class.lebedev(6) do |_x, _y, _z|
        1.0
      end

      expect(result).to(be_within(1e-14).of(4.0 * Math::PI))
    end

    it "integrates an odd function to zero" do
      result = described_class.lebedev(14) do |x, _y, _z|
        x
      end

      expect(result).to(be_within(1e-14).of(0.0))
    end

    it "integrates a quadratic coordinate" do
      result = described_class.lebedev(14) do |x, _y, _z|
        x * x
      end

      expect(result).to(be_within(1e-14).of(4.0 * Math::PI / 3.0))
    end

    it "passes points on the unit sphere to the block" do
      result = described_class.lebedev(26) do |x, y, z|
        (x * x) + (y * y) + (z * z)
      end

      expect(result).to(be_within(1e-14).of(4.0 * Math::PI))
    end

    it "requires an integer node count" do
      expect do
        described_class.lebedev("6") { |_x, _y, _z| 1.0 }
      end
        .to(raise_error(TypeError, "n must be an Integer"))
    end

    it "requires a positive node count" do
      expect do
        described_class.lebedev(0) { |_x, _y, _z| 1.0 }
      end
        .to(raise_error(ArgumentError, "n must be positive"))
    end

    it "rejects an unsupported node count" do
      expect do
        described_class.lebedev(7) { |_x, _y, _z| 1.0 }
      end
        .to(raise_error(Numo::GSL::GslError))
    end
  end
end
