# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe ".qawf" do

    let(:integrate) do
      lambda do |&block|
        described_class.qawf(
          0.0,
          1.0,
          described_class::COSINE,
          &block
        )
      end
    end

    it_behaves_like("an integration callback")

    it "integrates over a semi-infinite interval with a cosine weight", :aggregate_failures do
      result, error = described_class.qawf(
        0.0,
        1.0,
        described_class::COSINE
      ) { |x| Math.exp(-x) }

      expect(result).to(be_within(1e-8).of(0.5))
      expect(error).to(be >= 0.0)
    end

    it "integrates over a semi-infinite interval with a sine weight", :aggregate_failures do
      result, error = described_class.qawf(
        0.0,
        1.0,
        described_class::SINE
      ) { |x| Math.exp(-x) }

      expect(result).to(be_within(1e-8).of(0.5))
      expect(error).to(be >= 0.0)
    end

    it "raises GslError directly when GSL integration fails" do
      error = begin
        described_class.qawf(
          0.0,
          1.0,
          described_class::COSINE,
          epsabs: 0.0
        ) { |x| Math.exp(-x) }
      rescue Numo::GSL::GslError => e
        e
      end

      expect(error).to(be_instance_of(Numo::GSL::GslError))
    end

    it "accepts integration options", :aggregate_failures do
      result, error = described_class
        .qawf(
          0.0,
          1.0,
          described_class::SINE,
          epsabs: 1e-12,
          limit: 100
        ) { |x| Math.exp(-x) }

      expect(result).to(be_within(1e-8).of(0.5))
      expect(error).to(be >= 0.0)
    end
  end
end
