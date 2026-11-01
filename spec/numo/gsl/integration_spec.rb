# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  it "has a version number" do
    expect(Numo::GSL::Integration::VERSION).not_to(be(nil))
  end

  describe ".qag" do
    it "integrates a function over a finite interval" do
      result, error = described_class.qag(0.0, 1.0) do |x|
        x * x
      end

      expect(result).to(be_within(1e-10).of(1.0 / 3.0))
      expect(error).to(be >= 0.0)
    end
  end
end
